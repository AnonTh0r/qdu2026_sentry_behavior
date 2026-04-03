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

#include "qdu2026_sentry_behavior/plugins/condition/is_hp_low.hpp"

namespace qdu2026_sentry_behavior
{

IsHpLowCondition::IsHpLowCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsHpLowCondition::tick()
{
  int hp_threshold = 210;
  getInput("hp_threshold", hp_threshold);

  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port' (referee_robotStatus)");
    return BT::NodeStatus::FAILURE;
  }

  const int hp = msg.current_hp;

  if (hp < hp_threshold) {
    RCLCPP_DEBUG(logger_, "HP is critically low: %d < %d", hp, hp_threshold);
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

BT::PortsList IsHpLowCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("hp_threshold", 210, "HP threshold for critical low state")};
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsHpLowCondition>("IsHpLow");
}
