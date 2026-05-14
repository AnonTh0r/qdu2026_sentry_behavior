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

#include "qdu2026_sentry_behavior/plugins/condition/home_check.hpp"

namespace qdu2026_sentry_behavior
{

HomeCheckCondition::HomeCheckCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus HomeCheckCondition::tick()
{
  int base_hp_limit = 0;
  int outpost_hp_limit = 0;
  std::string team = "blue";

  getInput("base_hp_limit", base_hp_limit);
  getInput("outpost_hp_limit", outpost_hp_limit);
  getInput("team", team);

  referee_interfaces::msg::GameRobotHP msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port' (referee_allRobotHP)");
    return BT::NodeStatus::FAILURE;
  }

  uint16_t base_hp, outpost_hp;
  if (team == "red") {
    base_hp = msg.red_base_hp;
    outpost_hp = msg.red_outpost_hp;
  } else {
    base_hp = msg.blue_base_hp;
    outpost_hp = msg.blue_outpost_hp;
  }

  const bool base_safe = base_hp > static_cast<uint16_t>(base_hp_limit);
  const bool outpost_safe = outpost_hp > static_cast<uint16_t>(outpost_hp_limit);

  if (base_safe && outpost_safe) {
    RCLCPP_INFO(
      logger_, "HomeCheck SUCCESS: base_hp=%d > %d, outpost_hp=%d > %d (team=%s)",
      base_hp, base_hp_limit, outpost_hp, outpost_hp_limit, team.c_str());
    return BT::NodeStatus::SUCCESS;
  }

  RCLCPP_WARN(
    logger_, "HomeCheck FAILURE: base_hp=%d <= %d %s, outpost_hp=%d <= %d %s (team=%s)",
    base_hp, base_hp_limit, base_safe ? "(ok)" : "(danger!)",
    outpost_hp, outpost_hp_limit, outpost_safe ? "(ok)" : "(danger!)",
    team.c_str());
  return BT::NodeStatus::FAILURE;
}

BT::PortsList HomeCheckCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::GameRobotHP>(
      "key_port", "{@referee_allRobotHP}", "GameRobotHP port on blackboard"),
    BT::InputPort<int>("base_hp_limit", 0, "Base HP threshold below which home is unsafe"),
    BT::InputPort<int>("outpost_hp_limit", 0, "Outpost HP threshold below which home is unsafe"),
    BT::InputPort<std::string>("team", "blue", "Team color: red or blue"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::HomeCheckCondition>("HomeCheck");
}
