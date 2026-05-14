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

#include "qdu2026_sentry_behavior/plugins/condition/is_outpost_destroyed.hpp"

namespace qdu2026_sentry_behavior
{

IsOutpostDestroyedCondition::IsOutpostDestroyedCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsOutpostDestroyedCondition::tick()
{
  int hp_threshold = 0;
  std::string team = "blue";

  getInput("hp_threshold", hp_threshold);
  getInput("team", team);

  referee_interfaces::msg::GameRobotHP msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port' (referee_allRobotHP)");
    return BT::NodeStatus::FAILURE;
  }

  uint16_t outpost_hp;
  if (team == "red") {
    outpost_hp = msg.red_outpost_hp;
  } else {
    outpost_hp = msg.blue_outpost_hp;
  }

  RCLCPP_INFO(logger_, "IsOutpostDestroyed check: outpost_hp=%d, threshold=%d (team=%s)",
    outpost_hp, hp_threshold, team.c_str());

  if (outpost_hp <= static_cast<uint16_t>(hp_threshold)) {
    RCLCPP_WARN(logger_, "Outpost destroyed: %d <= %d, returning SUCCESS",
      outpost_hp, hp_threshold);
    return BT::NodeStatus::SUCCESS;
  }

  RCLCPP_INFO(logger_, "Outpost alive: %d > %d, returning FAILURE",
    outpost_hp, hp_threshold);
  return BT::NodeStatus::FAILURE;
}

BT::PortsList IsOutpostDestroyedCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::GameRobotHP>(
      "key_port", "{@referee_allRobotHP}", "GameRobotHP port on blackboard"),
    BT::InputPort<int>("hp_threshold", 0, "Outpost HP threshold (destroyed when <= this)"),
    BT::InputPort<std::string>("team", "blue", "Team color: red or blue"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsOutpostDestroyedCondition>(
    "IsOutpostDestroyed");
}
