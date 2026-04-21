# 2026/04/10 更改日志

## 新增文件

### referee_simulation 包
- `src/referee_simulation/src/mock_navigate_through_poses.cpp` — Mock NavigateThroughPoses action server，模拟导航过程，支持通过 `mock_nav/trigger_fail` topic 动态触发失败

### qdu2026_sentry_behavior 包
- `plugins/condition/stay_home.hpp / .cpp` — StayHome 条件节点，HP < 阈值时返回 RUNNING，HP >= 阈值时返回 SUCCESS（参照 COD 的 StayHome 实现）
- `plugins/action/write_status.hpp / .cpp` — WriteStatus 动作节点，从 referee_robotStatus 提取 hp、max_hp 写入 blackboard 简单变量，供 BT.CPP v4 前置条件使用

## 修改文件

### referee_simulation
- `CMakeLists.txt` — 新增 mock_navigate_through_poses executable，添加 rclcpp_action、nav2_msgs 依赖
- `package.xml` — 添加 rclcpp_action、nav2_msgs 构建和运行依赖
- `launch/referee_simulation.launch.py` — 用 C++ mock node 替换旧的 fake_nav2_server.py，集成到 launch

### qdu2026_sentry_behavior
- `CMakeLists.txt` — 注册 stay_home、write_status 插件库
- `behavior_trees/test_navigation.xml` — 重构行为树逻辑：
  - 采用 COD 的 ReactiveFallback 模式替代原来的 Repeat + Fallback
  - 用 Inverter(IsStatusOK) 检测低 HP，进入回家分支
  - 用 StayHome 节点等待 HP 恢复（返回 RUNNING 而非 RetryUntilSuccessful 嵌套）
  - patrol 用 Repeat num_cycles="-1" 包裹，防止 SUCCESS 冒泡导致 goal handle 竞态崩溃
  - 用 Parallel 并行运行受击旋转被动逻辑（IsHpDropping + PublishSpinSpeed），不影响导航
  - 去掉了 IsRfidDetected 依赖

## 使用方式

```bash
# 编译
colcon build --packages-select referee_simulation qdu2026_sentry_behavior

# 启动仿真（referee + mock导航）
ros2 launch referee_simulation referee_simulation.launch.py

# 启动行为树
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py

# 模拟低血量
ros2 topic pub /referee_simulation/set_hp std_msgs/msg/Int32 '{data: 90}' --once

# 模拟导航失败
ros2 topic pub --once /mock_nav/trigger_fail std_msgs/msg/Empty
```
