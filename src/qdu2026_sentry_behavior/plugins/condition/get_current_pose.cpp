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

#include "qdu2026_sentry_behavior/plugins/condition/get_current_pose.hpp"

namespace qdu2026_sentry_behavior
{

GetCurrentPoseCondition::GetCurrentPoseCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus GetCurrentPoseCondition::tick()
{
  auto msg = getInput<referee_interfaces::msg::SentryState>("key_port");
  if (!msg) {
    RCLCPP_WARN(logger_, "SentryState message is not available");
    setOutput("current_pose", 0);
    return BT::NodeStatus::FAILURE;
  }

  uint8_t current_state = msg->current_state;

  // 输出当前姿态
  setOutput("current_pose", static_cast<int>(current_state));

  std::string pose_name;
  switch (current_state) {
    case 0:
      pose_name = "Attack";
      break;
    case 1:
      pose_name = "Defence";
      break;
    case 2:
      pose_name = "Patrol";
      break;
    default:
      pose_name = "Unknown";
      break;
  }

  RCLCPP_DEBUG(logger_, "Current pose: %s (%d)", pose_name.c_str(), current_state);

  return BT::NodeStatus::SUCCESS;
}

BT::PortsList GetCurrentPoseCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::SentryState>(
      "key_port", "{@referee_sentryState}", "SentryState port on blackboard"),
    BT::OutputPort<int>(
      "current_pose", "0=Attack, 1=Defence, 2=Patrol"),
  };
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::GetCurrentPoseCondition>("GetCurrentPose");
}
