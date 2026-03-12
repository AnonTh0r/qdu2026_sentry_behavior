# 调试指南：解决 Behavior 读取不到消息的问题

## 问题现象
- `ros2 topic echo /referee/game_status` 能看到消息正常发布
- 但 behavior 日志显示 "GameStatus message is not available"
- Ctrl+C 无法退出（已修复）

## 已完成的修复

### 1. 添加调试日志
在 `qdu2026_sentry_behavior_server.cpp` 中添加了：
- 订阅回调中打印接收到的 GameStatus
- 如果 tree_bb 为空，打印警告

### 2. 修复 Ctrl+C 无法退出
使用独立线程运行 executor，确保信号处理正常。

### 3. 条件节点添加详细日志
在 `is_game_status.cpp` 中添加 DEBUG 级别日志。

## 调试步骤

### 步骤 1: 重新编译并 source
```bash
colcon build --packages-select qdu2026_sentry_behavior
source install/setup.bash
```

### 步骤 2: 启动 simulation
```bash
# 终端 1
ros2 launch referee_simulation referee_simulation.launch.py
```

### 步骤 3: 启动 behavior（带调试日志）
```bash
# 终端 2
ros2 run qdu2026_sentry_behavior qdu2026_sentry_behavior_server --ros-args --log-level debug
```

**注意**：使用 `--log-level debug` 可以看到详细的调试信息。

### 步骤 4: 运行诊断脚本
```bash
# 终端 3
./diagnose_topics.sh
```

## 预期看到的日志

### 如果正常工作 ✅
```
[INFO] Received GameStatus: progress=4, time=227, key='referee_gameStatus'
[DEBUG] Attempting to read GameStatus from blackboard key 'key_port'
[DEBUG] Successfully read GameStatus: progress=4, time=227
```

### 如果 tree_bb 为空 ⚠️
```
[WARN] Received message on topic '/referee/game_status' but tree_bb is null
```
这说明行为树还没创建，或者创建失败。

### 如果订阅没收到消息 ❌
```
[WARN] GameStatus message is not available on blackboard, assuming game NOT started
```
但没有 "Received GameStatus" 日志，说明订阅回调没被触发。

## 可能的问题和解决方案

### 问题 1: 行为树没有启动
**症状**: 看到 "tree_bb is null" 警告

**原因**: behavior server 需要通过 action 客户端发送目标才会创建树

**解决方案**: 使用 behavior client 启动树
```bash
# 方式 1: 使用 launch 文件（推荐）
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py

# 方式 2: 手动发送 action goal
ros2 action send_goal /behavior_server btcpp_ros2_interfaces/action/ExecuteTree \
  "{target_tree: 'rmul_2025'}" --feedback
```

### 问题 2: QoS 不匹配
**症状**: topic info 显示发布者和订阅者都存在，但没有连接

**解决方案**: 检查 QoS 设置
```bash
ros2 topic info /referee/game_status -v
```

如果 QoS 不匹配，修改 simulation 或 behavior 的 QoS 设置。

### 问题 3: 话题名称仍然不匹配
**症状**: topic list 中看不到 `/referee/game_status`

**解决方案**: 确认 simulation 已经重新编译
```bash
colcon build --packages-select referee_simulation
source install/setup.bash
```

### 问题 4: 多个 ROS_DOMAIN_ID
**症状**: 两个节点在不同的 domain

**解决方案**: 确保使用相同的 ROS_DOMAIN_ID
```bash
# 检查当前 domain
echo $ROS_DOMAIN_ID

# 如果需要，设置相同的 domain
export ROS_DOMAIN_ID=0
```

## 快速测试命令

### 测试 1: 检查话题连接
```bash
ros2 topic info /referee/game_status
```
应该看到：
```
Publishers: 1
Subscribers: 1
```

### 测试 2: 检查消息内容
```bash
ros2 topic echo /referee/game_status --once
```

### 测试 3: 检查节点列表
```bash
ros2 node list
```
应该看到：
- `/referee_simulation_node`
- `/qdu2026_sentry_behavior_server` 或类似名称

### 测试 4: 检查 action server
```bash
ros2 action list
```
应该看到：
- `/behavior_server`

### 测试 5: 发送测试 action goal
```bash
ros2 action send_goal /behavior_server btcpp_ros2_interfaces/action/ExecuteTree \
  "{target_tree: 'rmul_2025'}" --feedback
```

## 如果还是不行

### 最后的诊断方法

1. **确认消息类型匹配**
```bash
ros2 interface show referee_interfaces/msg/GameStatus
```

2. **使用 ros2 bag 录制**
```bash
ros2 bag record /referee/game_status /referee/robot_status
```

3. **检查 behavior server 的订阅列表**
```bash
ros2 node info /qdu2026_sentry_behavior_server
```

4. **查看完整日志**
```bash
ros2 run qdu2026_sentry_behavior qdu2026_sentry_behavior_server \
  --ros-args --log-level debug 2>&1 | tee behavior_debug.log
```

## 常见错误和解决

### 错误 1: "tree_bb is null"
→ 行为树没有启动，使用 action client 发送目标

### 错误 2: "No publishers on topic"
→ simulation 没有启动或话题名称不匹配

### 错误 3: "QoS mismatch"
→ 修改 QoS 设置为兼容模式

### 错误 4: Ctrl+C 无法退出
→ 已修复，重新编译即可

## 下一步

1. 先运行 `./diagnose_topics.sh` 查看话题状态
2. 使用 `--log-level debug` 启动 behavior
3. 观察日志，确定是哪个环节出问题
4. 根据上面的解决方案逐一排查

如果看到 "Received GameStatus" 日志但仍然显示 "not available"，说明 blackboard key 映射有问题，需要进一步检查 XML 配置。
