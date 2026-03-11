# Referee Simulation 修复说明

## 问题描述
referee_simulation 无法与 qdu2026_sentry_behavior 正确通信，因为话题名称不匹配。

## 修复内容

### 1. 话题名称对齐
修改了 `referee_simulation_node.cpp` 中的所有发布器话题名称，从相对路径改为绝对路径：

**修改前：**
```cpp
game_status_pub_ = create_publisher<...>("referee/game_status", 10);
```

**修改后：**
```cpp
game_status_pub_ = create_publisher<...>("/referee/game_status", 10);
```

### 2. 修改的话题列表
所有以下话题都已添加前导 `/` 符号：
- `/referee/game_status`
- `/referee/all_robot_hp`
- `/referee/event_data`
- `/referee/robot_status`
- `/referee/buff`
- `/referee/set_pose`
- `/referee/ground_robot_position`
- `/referee/rfid_status`
- `/referee/operating_mode`

### 3. 新增 Launch 文件
创建了 `launch/referee_simulation.launch.py` 方便启动仿真节点。

## 使用方法

### 启动仿真节点
```bash
# 方法 1: 使用 launch 文件
ros2 launch referee_simulation referee_simulation.launch.py

# 方法 2: 直接运行节点
ros2 run referee_simulation referee_simulation_node
```

### 启动 behavior 节点
```bash
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
```

### 测试话题通信
```bash
# 查看所有 referee 话题
ros2 topic list | grep referee

# 查看机器人状态消息
ros2 topic echo /referee/robot_status

# 手动设置 HP 进行测试
ros2 topic pub /referee_simulation/set_hp std_msgs/msg/Int32 '{data: 100}' --once

# 检查话题连接情况
ros2 topic info /referee/robot_status
```

## 验证对齐
behavior server 订阅的话题（来自 qdu2026_sentry_behavior_server.cpp）：
- `/referee/event_data` → `referee_eventData`
- `/referee/all_robot_hp` → `referee_allRobotHP`
- `/referee/game_status` → `referee_gameStatus`
- `/referee/ground_robot_position` → `referee_groundRobotPosition`
- `/referee/rfid_status` → `referee_rfidStatus`
- `/referee/robot_status` → `referee_robotStatus`
- `/referee/buff` → `referee_buff`
- `/referee/operating_mode` → `referee_operatingMode`
- `/referee/set_pose` → `referee_pose`

simulation 发布的话题现在完全匹配上述列表。

## 注意事项
1. 确保在运行前已经 source 了工作空间：
   ```bash
   source install/setup.bash
   ```

2. 如果修改了代码，需要重新编译：
   ```bash
   colcon build --packages-select referee_simulation
   ```

3. HP 模拟功能：
   - 初始 HP: 400
   - 可通过 `/referee_simulation/set_hp` 话题手动设置
   - 自动掉血功能已注释（107-119行），如需启用请取消注释
