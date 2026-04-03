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

#include "qdu2026_sentry_behavior/plugins/condition/is_hp_dropping.hpp"

#include <chrono>

namespace qdu2026_sentry_behavior
{

IsHpDroppingCondition::IsHpDroppingCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsHpDroppingCondition::tick()
{
  int hp_drop_threshold = 10;
  getInput("hp_drop_threshold", hp_drop_threshold);

  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    static auto last_error = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_error).count() >= 1) {
      RCLCPP_ERROR(logger_, "✗ Failed to read 'key_port' (referee_robotStatus)");
      last_error = now;
    }
    return BT::NodeStatus::FAILURE;
  }

  const int hp = static_cast<int>(msg.current_hp);

  // 首次读取，初始化 last_hp_
  if (last_hp_ < 0) {
    last_hp_ = hp;
    return BT::NodeStatus::FAILURE;
  }

  const int drop = last_hp_ - hp;
  last_hp_ = hp;

  if (drop >= hp_drop_threshold) {
    RCLCPP_INFO(logger_, "HP dropped by %d (threshold=%d), triggering", drop, hp_drop_threshold);
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

BT::PortsList IsHpDroppingCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("hp_drop_threshold", 10, "HP drop amount to trigger (default: 10)"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsHpDroppingCondition>("IsHpDropping");
}
