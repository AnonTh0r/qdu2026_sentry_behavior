# 最终测试指南

## 已完成的修复

### 1. 降低 tick 频率（避免卡顿）
- 从 100 Hz 降低到 10 Hz
- 文件：`params/sentry_behavior.yaml`

### 2. 使用 Reliable QoS（确保消息不丢失）
- 所有裁判系统消息使用 Reliable QoS
- 文件：`qdu2026_sentry_behavior_server.cpp`

### 3. 修复 Ctrl+C 无法退出
- 改用 SingleThreadedExecutor
- 文件：`qdu2026_sentry_behavior_server.cpp`

### 4. 添加详细调试日志
- 订阅回调中打印接收到的消息
- 条件节点中打印读取状态
- 使用 ✓ 和 ✗ 符号便于识别

### 5. 修复 blackboard key 语法
- `patrol.xml` 中的 `{referee_robotStatus}` → `{@referee_robotStatus}`

## 测试步骤

### 1. Source 环境
```bash
cd ~/qdu2026_behavior
source install/setup.bash
```

### 2. 启动 simulation（终端 1）
```bash
ros2 launch referee_simulation referee_simulation.launch.py
```

### 3. 启动 behavior（终端 2）
```bash
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
```

### 4. 观察日志

**正常情况下应该看到**：
```
[INFO] ✓ Received GameStatus: progress=4, time=xxx, writing to key='referee_gameStatus'
[INFO] ✓ Received RobotStatus: hp=400, writing to key='referee_robotStatus'
[INFO] ✓ GameStatus available: progress=4, time=xxx
[INFO] ✓ RobotStatus available: hp=400
```

**如果还是有问题，会看到**：
```
[ERROR] ✗ GameStatus NOT available from key_port, returning FAILURE
[ERROR] ✗ RobotStatus NOT available from key_port, returning FAILURE
```

或者：
```
[WARN] ✗ Received message on topic '/referee/game_status' but tree_bb is null (tree not created yet)
```

## 如果还是显示 "NOT available"

### 检查 1: 确认 tree_bb 已创建
如果看到 "tree_bb is null" 警告，说明行为树没有启动。

**解决方案**：确认 client 节点正在运行
```bash
ros2 node list | grep client
```

应该看到：`/qdu2026_sentry_behavior_client`

### 检查 2: 确认使用的树名称
```bash
ros2 param get /qdu2026_sentry_behavior_client target_tree
```

应该显示：`patrol`

### 检查 3: 查看 action 状态
```bash
ros2 action list
ros2 action info /qdu2026_sentry_behavior
```

### 检查 4: 手动发送 action goal
```bash
ros2 action send_goal /qdu2026_sentry_behavior btcpp_ros2_interfaces/action/ExecuteTree \
  "{target_tree: 'patrol'}" --feedback
```

## Ctrl+C 退出

现在应该可以正常用 Ctrl+C 退出了。如果还是不行：

### 方法 1: 按两次 Ctrl+C
```bash
# 第一次 Ctrl+C
# 等待 1 秒
# 第二次 Ctrl+C
```

### 方法 2: 使用 Ctrl+Z 然后 kill
```bash
# 按 Ctrl+Z 暂停进程
# 然后执行
kill %1
```

### 方法 3: 强制结束
```bash
# 在另一个终端
killall qdu2026_sentry_behavior_server
killall qdu2026_sentry_behavior_client
```

## 性能优化

### 当前设置
- tick_frequency: 10 Hz（每秒 10 次）
- 订阅回调日志：每 2 秒打印一次
- 条件节点日志：每 2 秒打印一次

### 如果还是卡顿
可以进一步降低频率，修改 `params/sentry_behavior.yaml`：
```yaml
tick_frequency: 5  # 降低到 5 Hz
```

### 如果想提高响应速度
```yaml
tick_frequency: 20  # 提高到 20 Hz（但可能会卡顿）
```

## 调试命令

### 查看所有节点
```bash
ros2 node list
```

### 查看节点信息
```bash
ros2 node info /qdu2026_sentry_behavior_server
```

### 查看话题连接
```bash
ros2 topic info /referee/game_status -v
```

### 查看参数
```bash
ros2 param list /qdu2026_sentry_behavior_server
ros2 param get /qdu2026_sentry_behavior_server tick_frequency
```

### 实时监控日志
```bash
ros2 topic echo /rosout | grep -E "GameStatus|RobotStatus"
```

## 预期行为

### patrol 树的行为
1. 等待 game_progress == 4（比赛开始）
2. 发布 spin_speed = 0.3
3. 检查机器人状态（HP >= 390）
4. 如果状态正常：导航到 (2,3,0) → 等待 15 秒 → 导航到 (1,1,0) → 等待 15 秒
5. 如果状态异常：导航到补给区 (3,0,0)
6. 循环执行

### 如果想测试 rmul_2025 树
修改 `params/sentry_behavior.yaml` 第 29 行：
```yaml
target_tree: rmul_2025
```

然后重启 behavior（不需要重新编译，因为使用了 --symlink-install）。

## 总结

关键修复：
1. ✅ 降低 tick 频率到 10 Hz
2. ✅ 使用 Reliable QoS
3. ✅ 修复 Ctrl+C 退出
4. ✅ 添加详细日志
5. ✅ 修复 blackboard key 语法

现在启动测试，观察日志中是否有 ✓ 符号，如果有就说明正常工作了！
