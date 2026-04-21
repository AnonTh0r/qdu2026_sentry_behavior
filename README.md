现在我理解用户的问题了！

用户说：

启动时HP=400
第一次改成HP=90 → IsHpLow触发，第一个Sequence中的PubNav2Goal变成SUCCESS（绿色）
HP恢复到400 → 但是IsHpLow的PubNav2Goal不变成其他状态（还是绿色）
第二次再改成HP=90 → IsHpLow正常响应，但PubNav2Goal依旧是SUCCESS（绿色）
问题是：PubNav2Goal一旦变成SUCCESS后，就一直保持SUCCESS状态，不会变回IDLE

这是Groot显示的问题，还是实际的树执行问题？

让我分析一下树的结构：


<KeepRunningUntilFailure>
  <Sequence>
    <RetryUntilSuccessful>
      <IsGameStatus expected_game_progress="4"/>
    </RetryUntilSuccessful>
    <ReactiveFallback>
      <Sequence>
        <IsHpLow hp_threshold="200"/>
        <ReactiveSequence>
          <PubNav2Goal goal="1.1;0.44;0"/>
        </ReactiveSequence>
      </Sequence>
      <PubNav2Goal goal="1.62;-7.12;0"/>
      <AlwaysSuccess/>
    </ReactiveFallback>
  </Sequence>
</KeepRunningUntilFailure>
执行流程：

HP=400时：

IsHpLow返回FAILURE
第一个Sequence失败
ReactiveFallback执行第二个分支：PubNav2Goal(1.62;-7.12;0)返回SUCCESS
第三个分支：AlwaysSuccess（不会执行，因为已经有SUCCESS了）
ReactiveFallback返回SUCCESS
HP=90时（第一次）：

IsHpLow返回SUCCESS
ReactiveSequence执行：PubNav2Goal(1.1;0.44;0)返回SUCCESS
ReactiveSequence返回SUCCESS
第一个Sequence返回SUCCESS
ReactiveFallback得到SUCCESS，返回SUCCESS（不会执行后面的分支）
HP恢复到400：

IsHpLow返回FAILURE
第一个Sequence失败
ReactiveFallback执行第二个分支：PubNav2Goal(1.62;-7.12;0)返回SUCCESS
问题在于：ReactiveSequence中的PubNav2Goal在第一个Sequence失败后，不会被重置为IDLE

这是因为：

当第一个Sequence失败时，ReactiveFallback会跳过它，执行第二个分支
第一个Sequence中的ReactiveSequence和PubNav2Goal不会被tick，所以保持之前的状态（SUCCESS）
Groot显示的是最后一次tick的状态，如果节点没有被tick，就保持之前的状态
这不是"卡死"，而是Groot的显示问题。实际上树在正常工作，只是Groot显示的颜色让人误以为节点还在执行。

但用户说"第二次再改成HP=90 → hplow的pubnav2goal依旧是success"，这可能意味着：

第二次HP降低时，PubNav2Goal确实被执行了（返回SUCCESS）
但用户期望看到状态变化（从IDLE到RUNNING到SUCCESS），而不是一直是SUCCESS
或者，用户遇到的真正问题是：第二次HP降低时，机器人的行为不对（比如不导航到回家点）？

让我问清楚用户的具体问题。


核心区别

  1. 航点方式（NavigateThroughPoses/FollowWaypoints）

  工作原理：
  - 从CSV文件一次性加载所有航点（如 rmul26.csv）
  - 调用Nav2的 /navigate_through_poses 或 /follow_waypoints Action服务
  - Nav2规划一条连续路径穿越所有点
  - 机器人平滑移动，不会在中间点停止

  CSV格式：
  id,pose_x,pose_y,pose_z,rot_x,rot_y,rot_z,rot_w,command,
  0,0.317783,-3.43582,0,0,0,0,1,
  1,2.5,1.3,0,0,0,0.707,0.707,

  优势：
  - 路径优化：Nav2会规划最优轨迹
  - 动态避障：整个路径过程中持续避障
  - 不会停顿：机器人连续移动
  - 可中断：收到反馈，知道当前在第几个航点

  2. 多个PubNav2Goal方式

  工作原理：
  <PubNav2Goal goal="2;3;0"/>
  <Sleep msec="15000"/>
  <PubNav2Goal goal="1;1;0"/>
  <Sleep msec="15000"/>

  - 每次只发布一个目标点到 /goal_pose 话题
  - 等待固定时间（如15秒）
  - 再发布下一个点
  - 每个点之间完全独立

  劣势：
  - 机器人会在每个点停下来
  - 无法保证到达：只是等时间，不检测是否真的到了
  - 路径不连续：每次都是新的导航任务
  - 时间浪费：固定等待可能过长或过短

  实际效果对比

航点方式：
  起点 ──平滑曲线──> 点1 ──平滑曲线──> 点2 ──平滑曲线──> 点3(不停顿)         (不停顿)         (不停顿)

  多PubNav2Goal方式：
  起点 ──直线──> 点1 [停15秒] ──直线──> 点2 [停15秒] ──直线──> 点3
                    ↑                    ↑
                可能没到就发下一个      可能早到了还在等

  总结

  航点导航适合巡逻、快速穿越场景，路径连续高效；多个PubNav2Goal适合需要在每个点执行任务的场景（如拍照、扫描），但效率较低且不够智能。



   COD的血量检测实现

  1. 数据来源

  COD通过订阅串口数据获取裁判系统信息：

  // WriteToBlackboard节点订阅话题
  sub_ = global_node_->create_subscription<rm_interfaces::msg::SerialReceiveData>(
      "/SerialReceiveData", 10,
      std::bind(&WriteToBlackboard::callback, this, std::placeholders::_1));

  2. 回调函数更新数据

  void callback(const rm_interfaces::msg::SerialReceiveData::SharedPtr msg) {
      hp = msg->judge_system_data.hp;
      zone_status = msg->judge_system_data.zone_status;
      is_defence = msg->judge_system_data.is_defence;
      is_attack = msg->judge_system_data.is_attack;
      // ...
  }

  3. 写入黑板

  WriteToBlackboard 节点每次tick时将最新数据写入黑板：

  setOutput("Hp", hp);
  setOutput("Zone_status", zone_status);
  // ...

  4. 条件检测

  HpCondition 从黑板读取血量并判断：

  auto hp_ = getInput<float>("Hp");  // 从黑板读取
  if (hp < 210) {
      return SUCCESS;  // 血量低，触发回家
  }
  return FAILURE;

  COD vs QDU 对比

  COD方式：
  - 订阅自定义串口消息 /SerialReceiveData
  - 通过黑板共享数据
  - 灵活扩展（6个状态变量）

  QDU方式：
  - 直接订阅裁判系统标准接口 @referee_robotStatus
  - 在条件节点内部检测
  - 使用官方 referee_interfaces 消息

  核心区别：COD需要自己处理串口通信并封装消息，QDU直接用标准裁判系统接口。

  译成功，只有一条无关的 warning（ament_auto_package 头文件安装路径的建议，不影响功能）。

  新增的文件汇总：

  ┌──────────────────────────────────────────────────┬─────────────────────────────────────────────────────────┐
  │                       文件                       │                          说明                           │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ include/.../waypoint_utils.hpp                   │ CSV 加载工具函数（inline，namespace 内）                │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ include/.../follow_waypoints_action.hpp          │ FollowWaypointsAction 类声明                            │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ plugins/action/follow_waypoints_action.cpp       │ 实现 + CreateRosNodePlugin(..., "FollowWaypoints")      │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ include/.../navigate_through_poses_action.hpp    │ NavigateThroughPosesAction 类声明                       │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ plugins/action/navigate_through_poses_action.cpp │ 实现 + CreateRosNodePlugin(..., "NavigateThroughPoses") │
  ├──────────────────────────────────────────────────┼─────────────────────────────────────────────────────────┤
  │ CMakeLists.txt                                   │ 新增两个 ament_auto_add_library 条目                    │
  └──────────────────────────────────────────────────┴─────────────────────────────────────────────────────────┘

  在 XML 行为树中使用时：

  <!-- 逐点停靠（每个航点都会停）-->
  <FollowWaypoints waypoint_file="/path/to/waypoints.csv" frame_id="map" current_waypoint="{wp_idx}"/>

  <!-- 平滑穿越（不在中间点停留，适合巡逻）-->
  <NavigateThroughPoses waypoint_file="/path/to/waypoints.csv" frame_id="map" current_waypoint="{wp_idx}"/>

  CSV 格式与 COD 项目相同：id,pose_x,pose_y,pose_z,rot_x,rot_y,rot_z,rot_w[,command,wait_sec]

● 全部完成，总结一下改动：

  ---
  改动内容

  6 个 condition 文件，每个做了 3 处修改：

  1. .hpp 基类：BT::SimpleConditionNode → BT::ConditionNode
  2. .hpp 方法：checkXxx() → tick() override
  3. .cpp 构造函数：删掉 std::bind(...) 参数，改为 BT::ConditionNode(name, config)
  4. .cpp 方法定义：::checkXxx() → ::tick()

  额外清理了 is_status_ok.hpp 里的残留历史代码（s_global_bb_、hp_base_、setGlobalBlackboard）。

  ---
  为什么这样就解决了

  ConditionNode::executeTick() 源码里有这段保护：

  // BT.CPP 源码
  NodeStatus ConditionNode::executeTick() {
      auto status = tick();
      if (status == NodeStatus::RUNNING) {
          throw LogicError("ConditionNode must never return RUNNING");
      }
      return status;
  }

  SimpleConditionNode 绕过了这个 executeTick()，直接调用你传入的 std::function，所以返回 RUNNING 时不报错、悄悄卡住。换回 ConditionNode 后，一旦误返回 RUNNING
  就会立即抛出异常，问题暴露得早、好定位。



ros2 launch rmu_gazebo_simulator bringup_sim.launch.py
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_multi.launch.py   namespace:=red_standard_robot1

ros2 run referee_simulation referee_simulation_node
colcon build --symlink-install --packages-select qdu2026_sentry_behavior
 ros2 topic pub /red_standard_robot1/goal_pose geometry_msgs/msg/PoseStamped '
   {
     header: {frame_id: "map"},
     pose: {
       position: {x: -0.9, y: -4.71, z: 0.0},
       orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
     }
   }'

ros2 topic echo /referee/robot_status
ros2 topic info /red_standard_robot1/goal_pose
ros2 topic  echo /referee_simulation/set_hp

ros2 topic pub /referee_simulation/set_hp std_msgs/msg/Int32 '{data: 90}' --once
ros2 launch pb2025_nav_bringup rm_navigation_simulation_launch.py world:=rmuc_2025 slam:=False
ros2 launch rmu_gazebo_simulator bringup_sim.launch.py
source install/setup.bash



<?xml version="1.0" encoding="UTF-8"?>
<root BTCPP_format="4">
  <BehaviorTree ID="rmul_2026">
    <KeepRunningUntilFailure>
      <Sequence>
        <!-- 1. 等待比赛开始 -->
        <RetryUntilSuccessful num_attempts="-1">
          <IsGameStatus max_remain_time="420"
                        min_remain_time="0"
                        expected_game_progress="4"
                        key_port="{@referee_gameStatus}"/>
        </RetryUntilSuccessful>

        <!-- 2. 初始化（只执行一次） -->
        <Sequence>
          <ChangePose current_pose="1" topic_name="/referee/set_pose"/>
          <PublishSpinSpeed spin_speed="1.0" duration="0" topic_name="cmd_spin"/>
        </Sequence>

        <!-- 3. 主循环 -->
        <ForceSuccess>
          <ReactiveSequence>
            <!-- 状态检查：只有状态正常才继续执行后续逻辑 -->
            <Fallback>
              <!-- 选项1: 状态正常 → 返回 SUCCESS，跳过后续补给/备用点 -->
              <IsStatusOK ammo_min="0" heat_max="350" hp_min="200" 
                          key_port="{@referee_robotStatus}"/>
              
              <!-- 选项2: 状态异常 → 尝试补给 -->
              <Sequence>
                <ChangePose current_pose="3" topic_name="/referee/set_pose"/>
                <PubNav2Goal goal="0;0;0" topic_name="goal_pose"/>
                <RetryUntilSuccessful num_attempts="10">
                  <IsRfidDetected friendly_supply_zone_non_exchange="true"
                                  key_port="{@referee_rfidStatus}"/>
                </RetryUntilSuccessful>
              </Sequence>
              
              <!-- 选项3: 补给也失败 → 导航到备用点 4.65,-3.5 -->
              <PubNav2Goal goal="4.65;-3.5;0" topic_name="goal_pose"/>
            </Fallback>
          </ReactiveSequence>
        </ForceSuccess>
      </Sequence>
    </KeepRunningUntilFailure>
  </BehaviorTree>

  <!-- TreeNodesModel 保持不变 -->
  <TreeNodesModel>
    <Action ID="ChangePose">
      <input_port name="current_pose"/>
      <input_port name="topic_name" default="/referee/set_pose"/>
    </Action>
    <Condition ID="IsGameStatus">
      <input_port name="max_remain_time" default="420"/>
      <input_port name="min_remain_time" default="0"/>
      <input_port name="expected_game_progress" default="4"/>
      <input_port name="key_port" default="{@referee_gameStatus}"/>
    </Condition>
    <Condition ID="IsRfidDetected">
      <input_port name="friendly_supply_zone_exchange" default="false"/>
      <input_port name="friendly_supply_zone_non_exchange" default="false"/>
      <input_port name="center_gain_point" default="false"/>
      <input_port name="friendly_fortress_gain_point" default="false"/>
      <input_port name="key_port" default="{@referee_rfidStatus}"/>
    </Condition>
    <Condition ID="IsStatusOK">
      <input_port name="ammo_min" default="0"/>
      <input_port name="heat_max" default="350"/>
      <input_port name="hp_min" default="300"/>
      <input_port name="key_port" default="{@referee_robotStatus}"/>
    </Condition>
    <Action ID="PubNav2Goal">
      <input_port name="goal" default="0;0;0"/>
      <input_port name="topic_name" default="goal_pose"/>
    </Action>
    <Action ID="PublishSpinSpeed">
      <input_port name="spin_speed" default="0.0"/>
      <input_port name="duration"/>
      <input_port name="topic_name" default="cmd_spin"/>
    </Action>
  </TreeNodesModel>
</root>


修改完成。主要变更：                                 
                                                                                          
  结构改动：                                                                                                                         
  - hp_min 从 150 改为 200                                                                                                           
  - 前进路径：1;1;0 → Sleep 8s → 2;2;0 → Sleep 8s → 8.37;-0.35;0 → 等待                                                              
  - 返回路径：2;2;0 → Sleep 8s → 1;1;0 → Sleep 8s → 3.09;2.36;0（顺序反向）                                                          
  - 顺手修复了 goal=";0" 的损坏值                                                                                                    
                                                                                                                                     
  绿色虚线卡住的原因：                                                                                                               
                                                                                                                                     
  原来 Sleep 总共只有约 96 秒。Sleep 全部跑完后，内层 Sequence 返回 SUCCESS → ReactiveSequence 也返回 SUCCESS →                      
  KeepRunningUntilFailure 重新循环从头开始 → IsStatusOK 在重置间隙没有被检查，机器人又重新导航到终点，如此反复。                     
                                                                                                                                     
  不是 "节点完成就认为任务完成不检测了"，ReactiveSequence 每个 tick 都会先重新检查 IsStatusOK。问题在于 Sleep                        
  总时长不够覆盖整场比赛。
                                                                                                                                     
  修复方法： 改为 8 × 60s = 480 秒，超过比赛总时长（420s），内层 Sequence 在正常比赛中永远不会走完，ReactiveSequence 始终处于 RUNNING
   状态持续检测 hp。
                                                                                                                                     
✻ Sautéed for 46s                                                                                                                  
                                                                                          
❯ tick的话是每个sleep结束后tick吗                                                                                                    
                                                                                                                                     
● 不是，tick 是由行为树引擎持续循环调用的，频率通常是每隔几毫秒一次（取决于你的 server 配置，一般 10~100ms 一个 tick）。             
                                                                                                                                     
  Sleep 节点的工作方式是：                                                                                                           
   
  tick N:   Sleep.onStart() → 记录开始时间，返回 RUNNING                                                                             
  tick N+1: Sleep.onRunning() → 检查elapsed < duration → 返回 RUNNING                                                                
  tick N+2: Sleep.onRunning() → 检查elapsed < duration → 返回 RUNNING                                                                
  ...                                                                                                                                
  tick N+k: Sleep.onRunning() → elapsed >= duration → 返回 SUCCESS                                                                   
                                                                                                                                     
  Sleep 在计时期间每个 tick 都返回 RUNNING，并不是"睡眠期间停止 tick"。                                                              
   
  所以 ReactiveSequence 的行为是这样的：                                                                                             
                                                            
  每个 tick:                                                                                                                         
    1. 先 tick IsStatusOK（重新检查 hp）                    
       └─ SUCCESS → 继续                                                                                                             
       └─ FAILURE → 整个 ReactiveSequence 立即返回 FAILURE（hp 触发）                                                                
    2. 再 tick 内层 Sequence（从当前位置继续，比如当前在某个 Sleep）                                                                 
       └─ Sleep 返回 RUNNING → ReactiveSequence 返回 RUNNING                                                                         
                                                                                                                                     
  关键：IsStatusOK 每个 tick 都被检查，不需要等 Sleep 结束。hp 一旦低于 200，在下一个 tick 就会立即触发返回分支，响应延迟只有 1 个   
  tick（几毫秒到几十毫秒）。                                
                                                                                                                                     
❯ 为什么在现实中如果他现在是绿色虚线,下次tick就不会再运行这个节点,但是在模拟中就还能运行                                             
  ⎿  ⧉ Selected 1 lines from README.md in Visual Studio Code             





  对，如果 hp>150，IsStatusOK 返回 SUCCESS，ReactiveSequence 继续执行：                                                                                                                              
                                                                                                                                                                                                     
  <ReactiveSequence>                                                                                                                                                                                 
    <IsStatusOK hp_min="150" .../>  <!-- SUCCESS -->                                                                                                                                                 
    <Sequence>                                                                                                                                                                                       
      <PubNav2Goal .../>              <!-- SUCCESS，发布点 -->                                                                                                                                       
      <Sleep msec="21000"/>           <!-- SUCCESS，等21秒 -->                                                                                                                                       
      <Sleep msec="40000"/>           <!-- SUCCESS，等40秒 -->                                                                                                                                       
      ...             <!-- 后续 Sleep 全 SUCCESS -->                                                                                                                                                 
    </Sequence>                                                                                                                                                                                      
  </ReactiveSequence>                                                                                                                                                                                
                                                                                                                                                                                                     
  所有 Sleep 跑完后，内层 Sequence 返回 SUCCESS，ReactiveSequence 也 SUCCESS，Fallback 的第一个子节点成功，返回 SUCCESS，ForceSuccess 包一层还是 SUCCESS，外层 Sequence 也 SUCCESS。                 
                                                                                                                                                                                                     
  然后 KeepRunningUntilFailure 重新 tick，又从头开始：                                                                                                                                               
                  
  1. RetryUntilSuccessful → IsGameStatus → SUCCESS（比赛还在进行）                                                                                                                                   
  2. ForceSuccess → Fallback → ReactiveSequence → IsStatusOK → 这里是关键
                                                                                                                                                                                                     
  ReactiveSequence 的特性：每次 tick 都会重新检查所有前置条件。                                                                                                                                      
                                                                                                                                                                                                     
  所以第二轮进入时，IsStatusOK 会重新检查 hp。如果 hp 还>150，应该继续执行 PubNav2Goal。                                                                                                             
                  
  但你说"卡死了"，可能的原因：                                                                                                                                                                       
                  
  ReactiveSequence 在重新 tick 时，内层 Sequence 的状态没有被重置，它记住了上一轮的 SUCCESS 状态，不会重新执行子节点。                                                                               
                  
  这是 BT.CPP 的已知行为——ReactiveSequence 只会重新检查条件节点，但 Action 节点（PubNav2Goal、Sleep）如果已经 SUCCESS，不会自动重置。                                                                
                  
  解决方法：在 Sequence 外面包一层 ForceFailure，强制让它每轮都重新执行：                                                                                                                            
                  
  <ReactiveSequence>                                                                                                                                                                                 
    <IsStatusOK hp_min="150" .../>                                                                                                                                                                   
    <ForceFailure>                                                                                                                                                                                   
      <Sequence>                                                                                                                                                                                     
        <PubNav2Goal .../>                                                                                                                                                                           
        <Sleep msec="21000"/>                                                                                                                                                                        
        ...                                                                                                                                                                                          
      </Sequence>                                                                                                                                                                                    
    </ForceFailure>                                                                                                                                                                                  
  </ReactiveSequence>                                                                                                                                                                                
                  
  这样每轮 Sequence 都会返回 FAILURE，KeepRunningUntilFailure 会继续循环。 