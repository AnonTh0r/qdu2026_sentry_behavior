# 问题根源：Blackboard Key 语法错误

## 发现的问题

在 `patrol.xml` 第 20 行，blackboard key 的语法错误：

**错误写法**：
```xml
<IsStatusOK key_port="{referee_robotStatus}"/>
```

**正确写法**：
```xml
<IsStatusOK key_port="{@referee_robotStatus}"/>
```

## 语法说明

在 BehaviorTree.CPP 中，blackboard key 的引用语法：

- `{@key_name}` - **正确**：从 blackboard 读取 key_name 的值
- `{key_name}` - **错误**：会被当作字符串字面量，而不是 blackboard 引用

## 为什么会读取不到消息

1. Server 订阅回调正确地将消息写入 blackboard：
   ```cpp
   tree_bb->set("referee_robotStatus", *msg);  // ✅ 正确
   ```

2. 但 XML 中使用了错误的语法：
   ```xml
   key_port="{referee_robotStatus}"  // ❌ 缺少 @
   ```

3. 条件节点尝试读取时：
   ```cpp
   auto msg = getInput<RobotStatus>("key_port");
   // 读取的是字符串 "referee_robotStatus"，而不是 blackboard 中的值
   // 所以返回空，导致 "not available"
   ```

## 已修复的文件

### patrol.xml
- 第 20 行：`{referee_robotStatus}` → `{@referee_robotStatus}`

### 其他文件检查

让我检查其他 XML 文件是否有类似问题：

**rmul_2025.xml** - ✅ 正确
- 第 9 行：`key_port="{@referee_gameStatus}"` ✅
- 第 30 行：`key_port="{@referee_robotStatus}"` ✅
- 第 37 行：`key_port="{@referee_rfidStatus}"` ✅

## 测试步骤

### 1. 重新编译（已完成）
```bash
colcon build --packages-select qdu2026_sentry_behavior --symlink-install
source install/setup.bash
```

### 2. 启动测试
```bash
# 终端 1
ros2 launch referee_simulation referee_simulation.launch.py

# 终端 2
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
```

### 3. 预期结果
现在应该能看到：
```
[INFO] Received GameStatus: progress=4, time=xxx, key='referee_gameStatus'
[INFO] [IsStatusOK SIMPLE] RESULT: SUCCESS
```

而不是：
```
[WARN] GameStatus message is not available
[WARN] RobotStatus message is not available
```

## 关于 --symlink-install

使用 `--symlink-install` 的好处：
- XML、YAML 等资源文件修改后不需要重新编译
- 直接修改源文件，install 目录会自动更新
- 加快开发迭代速度

**注意**：C++ 代码修改后仍然需要重新编译。

## 其他可能的问题

如果修复后还是不行，检查：

### 1. 确认使用的是 patrol 树
```bash
# 查看 params 文件
cat src/qdu2026_sentry_behavior/params/sentry_behavior.yaml | grep target_tree
```

应该显示：
```yaml
target_tree: patrol
```

### 2. 如果想测试 rmul_2025 树
修改 `sentry_behavior.yaml` 第 29 行：
```yaml
target_tree: rmul_2025
```

### 3. 查看完整日志
```bash
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py \
  log_level:=debug
```

## 总结

**根本原因**：XML 中 blackboard key 缺少 `@` 符号，导致无法正确引用 blackboard 中的数据。

**修复方法**：将 `{key_name}` 改为 `{@key_name}`

**验证方法**：启动后应该看到 "Received GameStatus" 和 "RESULT: SUCCESS" 日志

现在应该可以正常工作了！
