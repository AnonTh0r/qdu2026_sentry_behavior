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

#include "qdu2026_sentry_behavior/plugins/condition/is_zone_active.hpp"

namespace qdu2026_sentry_behavior
{

IsZoneActiveCondition::IsZoneActiveCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsZoneActiveCondition::tick()
{
  std::string zone_name;
  if (!getInput("zone_name", zone_name)) {
    RCLCPP_ERROR(logger_, "Missing required input 'zone_name'");
    return BT::NodeStatus::FAILURE;
  }

  referee_interfaces::msg::RfidStatus msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port' (referee_rfidStatus)");
    return BT::NodeStatus::FAILURE;
  }

  bool is_active = false;

  // Check which zone to monitor
  if (zone_name == "friendly_fortress_gain_point") {
    is_active = (msg.friendly_fortress_gain_point == msg.DETECTED);
  } else if (zone_name == "friendly_supply_zone_non_exchange") {
    is_active = (msg.friendly_supply_zone_non_exchange == msg.DETECTED);
  } else if (zone_name == "friendly_supply_zone_exchange") {
    is_active = (msg.friendly_supply_zone_exchange == msg.DETECTED);
  } else if (zone_name == "center_gain_point") {
    is_active = (msg.center_gain_point == msg.DETECTED);
  } else {
    RCLCPP_ERROR(logger_, "Unknown zone_name: %s", zone_name.c_str());
    return BT::NodeStatus::FAILURE;
  }

  if (is_active) {
    RCLCPP_DEBUG(logger_, "Zone '%s' is active", zone_name.c_str());
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

BT::PortsList IsZoneActiveCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RfidStatus>(
      "key_port", "{@referee_rfidStatus}", "RfidStatus port on blackboard"),
    BT::InputPort<std::string>(
      "zone_name", "Zone to check (friendly_fortress_gain_point, "
      "friendly_supply_zone_non_exchange, friendly_supply_zone_exchange, center_gain_point)")};
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsZoneActiveCondition>("IsZoneActive");
}
