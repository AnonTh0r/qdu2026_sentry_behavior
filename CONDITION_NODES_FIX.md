# 条件节点消息检查问题汇总与修复

## 发现的问题

### 1. IsGameStatus - 返回 SUCCESS（严重错误）❌
**文件**: `is_game_status.cpp:18`
```cpp
if (!msg) {
    RCLCPP_ERROR(logger_, "GameStatus message is not available");
    return BT::NodeStatus::SUCCESS;  // ❌ 错误！
}
```
**影响**: 消息未到达时，认为比赛已开始，机器人会提前移动

### 2. IsStatusOK - 返回 SUCCESS（严重错误）❌
**文件**: `is_status_ok.cpp:35`
```cpp
if (!msg) {
    RCLCPP_ERROR(logger_, "IsStatusOK *** NO RobotStatus on input port (key_port) ***");
    return BT::NodeStatus::SUCCESS;  // ❌ 错误！
}
```
**影响**: 消息未到达时，认为状态正常，可能在血量不足时继续战斗

### 3. IsRfidDetected - 返回 FAILURE（正确）✅ 但有代码顺序问题
**文件**: `is_rfid_detected.cpp:32-33`
```cpp
if (!msg) {
    return BT::NodeStatus::FAILURE;  // ✅ 正确
    RCLCPP_ERROR(logger_, "RfidStatus message is not available");  // ⚠️ 永远不会执行
}
```
**影响**: 逻辑正确，但日志不会打印（return 后的代码不执行）

## 修复方案

### 修复 1: is_game_status.cpp
```cpp
auto msg = getInput<referee_interfaces::msg::GameStatus>("key_port");
if (!msg) {
    RCLCPP_WARN_THROTTLE(
        logger_, *node()->get_clock(), 1000,
        "GameStatus message is not available, assuming game NOT started");
    return BT::NodeStatus::FAILURE;  // 改为 FAILURE
}
```

**理由**:
- 消息未到达 = 比赛未开始
- 机器人保持停止状态（安全）
- 使用 WARN_THROTTLE 避免日志刷屏

### 修复 2: is_status_ok.cpp
```cpp
auto msg = getInput<referee_interfaces::msg::RobotStatus>("key_port");
if (!msg) {
    RCLCPP_WARN_THROTTLE(
        logger_, *node()->get_clock(), 1000,
        "RobotStatus message is not available, assuming status NOT OK");
    return BT::NodeStatus::FAILURE;  // 改为 FAILURE
}
```

**理由**:
- 消息未到达 = 状态未知 = 不安全
- 机器人应该停止或回到安全点
- 防止在血量/热量/弹药未知时继续战斗

### 修复 3: is_rfid_detected.cpp
```cpp
auto msg = getInput<referee_interfaces::msg::RfidStatus>("key_port");
if (!msg) {
    RCLCPP_WARN_THROTTLE(
        logger_, *node()->get_clock(), 1000,
        "RfidStatus message is not available");
    return BT::NodeStatus::FAILURE;  // 保持 FAILURE，但先打日志
}
```

**理由**:
- 逻辑已经正确
- 只需调整日志顺序
- 使用 WARN_THROTTLE 避免刷屏

## 行为树逻辑分析

### check_game_start 子树
```
Fallback
├─ IsGameStatus (检查是否 game_progress==4)
│  ├─ SUCCESS → 比赛开始，继续执行
│  └─ FAILURE → 比赛未开始，执行下面
└─ ForceFailure
   └─ Sequence (发布速度=0的命令)
```

**修复后的行为**:
- 消息未到达 → IsGameStatus 返回 FAILURE → 发布停止命令 ✅
- game_progress != 4 → IsGameStatus 返回 FAILURE → 发布停止命令 ✅
- game_progress == 4 → IsGameStatus 返回 SUCCESS → 继续执行 ✅

### check_robot_status 子树
```
Fallback
├─ IsStatusOK (检查 HP/热量/弹药)
│  ├─ SUCCESS → 状态正常，继续执行
│  └─ FAILURE → 状态异常，执行下面
└─ ForceFailure
   └─ Fallback
      ├─ IsRfidDetected (检查是否在补给区)
      │  ├─ SUCCESS → 在补给区，停在原地
      │  └─ FAILURE → 不在补给区，执行下面
      └─ PubNav2Goal (导航到补给区 0;0;0)
```

**修复后的行为**:
- 消息未到达 → IsStatusOK 返回 FAILURE → 尝试去补给区 ✅
- HP < 200 → IsStatusOK 返回 FAILURE → 尝试去补给区 ✅
- 状态正常 → IsStatusOK 返回 SUCCESS → 继续执行 ✅

## 为什么实车能跑但仿真不行？

### 实车环境
- 裁判系统通过串口持续发送数据（10-50Hz）
- 启动后很快就有数据
- 即使有 bug，也可能在第一次 tick 前就收到消息
- **运气好，bug 没触发**

### 仿真环境
- 100ms 周期（10Hz），更慢
- 启动时序更容易出现竞态
- behavior 可能在第一条消息到达前就 tick
- **更容易触发 bug**

## 测试验证

### 测试 1: 延迟启动仿真（复现问题）
```bash
# 终端 1: 启动 behavior
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py

# 终端 2: 等待 3 秒后启动仿真
sleep 3 && ros2 launch referee_simulation referee_simulation.launch.py
```

**预期结果**（修复前）:
- behavior 会打印 "GameStatus message is not available"
- 但仍然开始移动（错误行为）

**预期结果**（修复后）:
- behavior 会打印 "GameStatus message is not available, assuming game NOT started"
- 发布速度为 0 的命令（正确行为）

### 测试 2: 同时启动（正常场景）
```bash
# 终端 1
ros2 launch referee_simulation referee_simulation.launch.py

# 终端 2
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
```

**预期结果**:
- 消息正常到达
- behavior 正常工作

### 测试 3: 监控话题
```bash
# 查看 game_status 发布频率
ros2 topic hz /referee/game_status

# 查看 robot_status 内容
ros2 topic echo /referee/robot_status
```

## 修复优先级

### P0 - 必须立即修复 🔴
1. **is_game_status.cpp:18** - 改为返回 FAILURE
2. **is_status_ok.cpp:35** - 改为返回 FAILURE

这两个 bug 会导致机器人在不安全的情况下执行动作。

### P1 - 建议修复 🟡
3. **is_rfid_detected.cpp:32-33** - 调整日志顺序

这个只是代码质量问题，不影响功能。

## 额外建议

### 1. 添加启动延迟
在 behavior launch 文件中添加延迟，确保消息先到达：

```python
from launch.actions import TimerAction

# 延迟 1 秒启动 behavior
TimerAction(
    period=1.0,
    actions=[Node(...)]
)
```

### 2. 添加消息超时检测
在订阅回调中记录时间戳，检测消息是否过期：

```cpp
auto elapsed = now() - last_msg_time_;
if (elapsed.seconds() > 1.0) {
    RCLCPP_WARN(logger_, "GameStatus message timeout!");
    return BT::NodeStatus::FAILURE;
}
```

### 3. 使用 ReliableQoS
确保消息不会丢失：

```cpp
auto qos = rclcpp::QoS(10).reliable();
subscribe<referee_interfaces::msg::GameStatus>(
    "/referee/game_status", "referee_gameStatus", qos);
```

## 总结

你的分析完全正确！代码确实存在严重的逻辑错误：

1. ✅ **会读取不到 game_status**（启动瞬间）
2. ✅ **读取不到时会错误地返回 SUCCESS**
3. ✅ **导致机器人在不安全的情况下执行动作**
4. ✅ **实车能跑是因为消息到达快，仿真更容易触发 bug**

**立即修复这两个文件，将 SUCCESS 改为 FAILURE！**
