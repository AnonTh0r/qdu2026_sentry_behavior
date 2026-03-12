# ChangePose 卡住问题分析

## 问题现象
行为树执行到 ChangePose 节点后卡住，后续节点不执行。

## 可能的原因

### 1. RosTopicPubStatefulActionNode 的行为
`RosTopicPubStatefulActionNode` 是一个 **Stateful Action**，它的行为是：
1. 第一次 tick：发布消息，返回 RUNNING
2. 后续 tick：检查某个条件，如果满足返回 SUCCESS，否则继续 RUNNING
3. 如果一直不满足条件，会一直卡在 RUNNING 状态

### 2. 需要实现的方法
可能需要实现 `onStateUpdate()` 方法来告诉节点何时完成：

```cpp
BT::NodeStatus onStateUpdate() override {
  // 检查是否完成
  // 如果完成返回 SUCCESS
  // 否则返回 RUNNING
  return BT::NodeStatus::SUCCESS;  // 立即完成
}
```

## 解决方案

### 方案 1：改用 RosTopicPubNode（推荐）
如果 ChangePose 只需要发布一次消息就完成，应该使用 `RosTopicPubNode` 而不是 `RosTopicPubStatefulActionNode`。

修改头文件：
```cpp
class ChangePose
: public BT::RosTopicPubNode<referee_interfaces::msg::SetPose>  // 改这里
{
  // ...
};
```

`RosTopicPubNode` 会：
1. 发布消息
2. 立即返回 SUCCESS
3. 不会卡住

### 方案 2：实现 onStateUpdate 方法
如果必须使用 Stateful 版本，需要实现状态检查：

```cpp
// 在头文件中添加
BT::NodeStatus onStateUpdate() override;

// 在 cpp 文件中实现
BT::NodeStatus ChangePose::onStateUpdate()
{
  // 立即返回成功，不等待任何状态
  return BT::NodeStatus::SUCCESS;
}
```

### 方案 3：添加超时
在 XML 中给 ChangePose 添加超时：

```xml
<ChangePose current_pose="1"
            topic_name="/referee/set_pose"
            timeout="100"/>  <!-- 100ms 超时 -->
```

## 推荐方案

**使用方案 1**：改用 `RosTopicPubNode`，因为 ChangePose 只需要发布一次消息，不需要等待任何状态反馈。

## 快速修复

修改 `change_pose.hpp` 第 27 行：
```cpp
// 修改前
class ChangePose
: public BT::RosTopicPubStatefulActionNode<referee_interfaces::msg::SetPose>

// 修改后
class ChangePose
: public BT::RosTopicPubNode<referee_interfaces::msg::SetPose>
```

修改 `change_pose.cpp` 第 11 行：
```cpp
// 修改前
: BT::RosTopicPubStatefulActionNode<referee_interfaces::msg::SetPose>(
    name, config, params)

// 修改后
: BT::RosTopicPubNode<referee_interfaces::msg::SetPose>(
    name, config, params)
```

然后重新编译即可。
