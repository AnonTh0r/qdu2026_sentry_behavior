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

#include "qdu2026_sentry_behavior/plugins/condition/is_rfid_detected.hpp"

namespace qdu2026_sentry_behavior
{

IsRfidDetectedCondition::IsRfidDetectedCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsRfidDetectedCondition::tick()
{
  bool friendly_fortress_gain_point, friendly_supply_zone_non_exchange,
    friendly_supply_zone_exchange, center_gain_point;
  auto msg = getInput<referee_interfaces::msg::RfidStatus>("key_port");
  if (!msg) {
    RCLCPP_WARN(logger_, "RfidStatus message is not available");
    return BT::NodeStatus::FAILURE;
  }

  getInput("friendly_fortress_gain_point", friendly_fortress_gain_point);
  getInput("friendly_supply_zone_non_exchange", friendly_supply_zone_non_exchange);
  getInput("friendly_supply_zone_exchange", friendly_supply_zone_exchange);
  getInput("center_gain_point", center_gain_point);

  bool detected = false;

  if (friendly_fortress_gain_point && msg->friendly_fortress_gain_point == msg->DETECTED) {
    RCLCPP_INFO(logger_, "RFID Detected: 己方堡垒增益点 (Friendly Fortress Gain Point)");
    detected = true;
  }

  if (friendly_supply_zone_non_exchange && msg->friendly_supply_zone_non_exchange == msg->DETECTED) {
    RCLCPP_INFO(logger_, "RFID Detected: 己方补给区 (Friendly Supply Zone Non-Exchange)");
    detected = true;
  }

  if (friendly_supply_zone_exchange && msg->friendly_supply_zone_exchange == msg->DETECTED) {
    RCLCPP_INFO(logger_, "RFID Detected: 己方兑换补给区 (Friendly Supply Zone Exchange)");
    detected = true;
  }

  if (center_gain_point && msg->center_gain_point == msg->DETECTED) {
    RCLCPP_INFO(logger_, "RFID Detected: 中心增益点 (Center Gain Point)");
    detected = true;
  }

  if (detected) {
    return BT::NodeStatus::SUCCESS;
  } else {
    return BT::NodeStatus::FAILURE;
  }
}

BT::PortsList IsRfidDetectedCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RfidStatus>(
      "key_port", "{@referee_rfidStatus}", "RfidStatus port on blackboard"),
    BT::InputPort<bool>("friendly_fortress_gain_point", false, "己方堡垒增益点"),
    BT::InputPort<bool>(
      "friendly_supply_zone_non_exchange", false, "己方与兑换区不重叠的补给区 / RMUL 补给区"),
    BT::InputPort<bool>("friendly_supply_zone_exchange", false, "己方与兑换区重叠的补给区"),
    BT::InputPort<bool>("center_gain_point", false, "中心增益点（仅 RMUL 适用）"),
  };
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsRfidDetectedCondition>("IsRfidDetected");
}
