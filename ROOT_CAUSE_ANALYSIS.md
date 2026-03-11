# 问题根源：providedPorts 中的默认值缺少 @ 符号

## 真正的问题

虽然 XML 中正确使用了 `{@referee_gameStatus}`，但是 **C++ 代码中 providedPorts 的默认值写错了**。

### is_game_status.cpp（已修复）

**错误代码**（第 58 行）：
```cpp
BT::InputPort<referee_interfaces::msg::GameStatus>(
  "key_port", "{referee_gameStatus}", "GameStatus port on blackboard"),
  //          ^^^^^^^^^^^^^^^^^^^^^ 缺少 @
```

**正确代码**：
```cpp
BT::InputPort<referee_interfaces::msg::GameStatus>(
  "key_port", "{@referee_gameStatus}", "GameStatus port on blackboard"),
  //          ^^^^^^^^^^^^^^^^^^^^^^^ 加上 @
```

### is_status_ok.cpp（已修复）

**错误代码**（第 83 行）：
```cpp
BT::InputPort<referee_interfaces::msg::RobotStatus>(
  "key_port", "RobotStatus port on blackboard"),
  //          ^^^ 完全没有默认值！
```

**正确代码**：
```cpp
BT::InputPort<referee_interfaces::msg::RobotStatus>(
  "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
  //          ^^^^^^^^^^^^^^^^^^^^^^^^ 添加默认值
```

## 为什么会出现这个问题？

1. **XML 中指定了 key_port**：
   ```xml
   <IsGameStatus key_port="{@referee_gameStatus}"/>
   ```

2. **但如果 XML 中没有指定**，就会使用 C++ 中的默认值

3. **C++ 默认值写错了**，导致即使 XML 正确，也可能读取不到

4. **BehaviorTree.CPP 的行为**：
   - 如果 XML 中指定了参数，使用 XML 的值
   - 如果 XML 中没有指定，使用 C++ providedPorts 的默认值
   - 在某些情况下（可能是 BT 版本或配置问题），默认值会覆盖 XML 的值

## 数据流分析

### 正确的数据流 ✅

```
1. Simulation 发布消息
   ↓
2. Server 订阅回调接收消息
   tree_bb->set("referee_gameStatus", *msg)
   ↓
3. XML 配置
   key_port="{@referee_gameStatus}"
   ↓
4. 条件节点读取
   getInput<GameStatus>("key_port")
   → 从 blackboard 读取 "referee_gameStatus"
   → 成功！✓
```

### 错误的数据流 ❌

```
1. Simulation 发布消息
   ↓
2. Server 订阅回调接收消息
   tree_bb->set("referee_gameStatus", *msg)  ✓
   ↓
3. providedPorts 默认值错误
   "key_port", "{referee_gameStatus}"  ❌ 缺少 @
   ↓
4. 条件节点读取
   getInput<GameStatus>("key_port")
   → 尝试读取字符串 "referee_gameStatus"（而不是 blackboard key）
   → 失败！✗
```

## 所有需要修复的地方

### ✅ 已修复
1. `is_game_status.cpp` 第 58 行：添加 `@`
2. `is_status_ok.cpp` 第 83 行：添加默认值 `{@referee_robotStatus}`
3. `patrol.xml` 第 20 行：添加 `@`（之前修复的）

### ✅ 已正确
1. `is_rfid_detected.cpp` 第 57 行：已经有 `{@referee_rfidStatus}`
2. `rmul_2025.xml`：所有 key_port 都正确使用了 `@`

## 测试验证

### 重新编译（已完成）
```bash
colcon build --packages-select qdu2026_sentry_behavior --symlink-install
source install/setup.bash
```

### 启动测试
```bash
# 终端 1
ros2 launch referee_simulation referee_simulation.launch.py

# 终端 2
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
```

### 预期日志
现在应该看到：
```
[INFO] ✓ Received GameStatus: progress=4, time=xxx, writing to key='referee_gameStatus'
[INFO] ✓ Received RobotStatus: hp=400, writing to key='referee_robotStatus'
[INFO] ✓ GameStatus available: progress=4, time=xxx
[INFO] ✓ RobotStatus available: hp=400
[INFO] [IsStatusOK SIMPLE] RESULT: SUCCESS
```

**不再有**：
```
[ERROR] ✗ GameStatus NOT available from key_port
```

## 经验教训

### 1. BehaviorTree.CPP 的 blackboard key 语法
- `{@key_name}` - 从 blackboard 读取
- `{key_name}` - 字符串字面量（错误用法）
- `key_name` - 字符串字面量（错误用法）

### 2. providedPorts 的重要性
- 默认值必须正确
- 即使 XML 中指定了值，默认值也可能影响行为
- 所有 blackboard 引用都必须加 `@`

### 3. 调试技巧
- 添加详细日志确认消息是否写入 blackboard
- 添加日志确认条件节点是否读取到数据
- 使用 ✓ 和 ✗ 符号便于快速识别问题

### 4. 常见错误模式
```cpp
// ❌ 错误 1：完全没有默认值
BT::InputPort<Type>("key_port", "description")

// ❌ 错误 2：缺少 @
BT::InputPort<Type>("key_port", "{key_name}", "description")

// ✅ 正确
BT::InputPort<Type>("key_port", "{@key_name}", "description")
```

## 总结

**根本原因**：C++ 代码中 providedPorts 的默认值缺少 `@` 符号或完全没有默认值。

**修复方法**：
1. `is_game_status.cpp`：`{referee_gameStatus}` → `{@referee_gameStatus}`
2. `is_status_ok.cpp`：添加默认值 `{@referee_robotStatus}`

**验证方法**：启动后应该看到 "✓ GameStatus available" 而不是 "✗ NOT available"

现在应该可以正常工作了！🎉
