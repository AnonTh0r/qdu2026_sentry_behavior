我在运行一个ros2导航与决策树的项目中发现了以下的问题：我写了一个基于hp的导航用来初步测试我的决策树是否能跑的通，主要在gazebo和rviz上面进行测试，鉴于目前没有裁判系统，我就又写了一个模拟裁判系统通信的包来和我的决策树进行通信，目前编译正常，且在没有运行导航和仿真的时候通信正常，但是只要打开仿真加导航，决策树的终端就会卡住，具体情况是：CTRL+C无法退出，会i一直输出ctrl+c的键值，在新的终端用topic list和echo也会出现相同的问题，而且导航那边没有按照我的决策树所写的逻辑进行运动，初步怀疑是命名空间的问题，但是表现出来的不是他的问题，然后就是考虑了是不是导航和仿真让cpu负载过大导致决策树卡住，因为我的4核心都跑到了90以上，再然后就是考虑决策树的逻辑写错了。我该怎么排除这些问题，现在我将把可能相关的文件传给你，如果还需要更多再跟我讲




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
