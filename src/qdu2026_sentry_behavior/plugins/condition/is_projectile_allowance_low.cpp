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

#include "qdu2026_sentry_behavior/plugins/condition/is_projectile_allowance_low.hpp"

namespace qdu2026_sentry_behavior
{

IsProjectileAllowanceLowCondition::IsProjectileAllowanceLowCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsProjectileAllowanceLowCondition::tick()
{
  int allowance_threshold = 50;
  getInput("allowance_threshold", allowance_threshold);

  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port' (referee_robotStatus)");
    return BT::NodeStatus::FAILURE;
  }

  const int allowance = msg.projectile_allowance_17mm;

  if (allowance < allowance_threshold) {
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

BT::PortsList IsProjectileAllowanceLowCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("allowance_threshold", 50, "Projectile allowance threshold")};
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsProjectileAllowanceLowCondition>(
    "IsProjectileAllowanceLow");
}
