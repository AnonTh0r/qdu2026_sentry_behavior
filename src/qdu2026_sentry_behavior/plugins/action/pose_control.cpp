// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "qdu2026_sentry_behavior/plugins/action/pose_control.hpp"

namespace qdu2026_sentry_behavior
{

PoseControl::PoseControl(
  const std::string & name,
  const BT::NodeConfig & config,
  const BT::RosNodeParams & params)
: BT::RosTopicPubNode<referee_interfaces::msg::SetPose>(name, config, params)
{
}

bool PoseControl::setMessage(referee_interfaces::msg::SetPose & msg)
{
  // 读取当前姿态状态
  auto state_msg = getInput<referee_interfaces::msg::SentryState>("key_port");
  if (state_msg) {
    int current = static_cast<int>(state_msg.value().current_state);
    setOutput("current_pose", current);

    std::string pose_name;
    switch (current) {
      case 0: pose_name = "进攻"; break;
      case 1: pose_name = "防御"; break;
      case 2: pose_name = "移动"; break;
      default: pose_name = "未知"; break;
    }
    RCLCPP_DEBUG(node_->get_logger(), "当前姿态: %s (%d)", pose_name.c_str(), current);
  } else {
    setOutput("current_pose", 0);
  }

  // 如果设置了目标姿态，则发送切换指令
  int target_pose;
  if (!getInput("target_pose", target_pose)) {
    // 没有设置目标姿态，只读取不发送
    return false;
  }

  if (target_pose < 0 || target_pose > 2) {
    RCLCPP_ERROR(
      node_->get_logger(),
      "Invalid target_pose: %d (must be 0-2)", target_pose);
    return false;
  }

  msg.pose = static_cast<uint8_t>(target_pose);
  msg.patrol_pose = 2;
  msg.attack_pose = 0;
  msg.defence_pose = 1;

  std::string pose_name;
  if (target_pose == 0) {
    pose_name = "进攻姿态";
  } else if (target_pose == 1) {
    pose_name = "防御姿态";
  } else {
    pose_name = "移动姿态";
  }

  RCLCPP_INFO(
    node_->get_logger(),
    "切换到 %s (模式: %d)", pose_name.c_str(), target_pose);

  return true;
}

BT::PortsList PoseControl::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::SentryState>(
      "key_port", "{@referee_sentryState}", "SentryState from blackboard"),
    BT::InputPort<int>(
      "target_pose", "目标姿态 (可选): 1=进攻, 2=防御, 3=移动"),
    BT::InputPort<std::string>(
      "topic_name", "/referee/set_pose", "Topic name"),
    BT::OutputPort<int>(
      "current_pose", "当前姿态: 1=进攻, 2=防御, 3=移动, 0=未知"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::PoseControl, "PoseControl");
