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

#include "qdu2026_sentry_behavior/plugins/action/get_detected_rfid.hpp"

namespace qdu2026_sentry_behavior
{

GetDetectedRfidAction::GetDetectedRfidAction(
  const std::string & name, const BT::NodeConfig & config)
: BT::SyncActionNode(name, config)
{
}

BT::NodeStatus GetDetectedRfidAction::tick()
{
  auto msg = getInput<referee_interfaces::msg::RfidStatus>("key_port");
  if (!msg) {
    RCLCPP_WARN(logger_, "RfidStatus message is not available");
    setOutput("detected_rfid", "none");
    return BT::NodeStatus::FAILURE;
  }

  std::string detected_type = "none";

  // 优先检查目标RFID
  if (msg->friendly_fortress_gain_point == msg->DETECTED) {
    detected_type = "friendly_fortress_gain_point";
    RCLCPP_INFO(logger_, "检测到: 己方堡垒增益点");
    setOutput("detected_rfid", detected_type);
    return BT::NodeStatus::SUCCESS;
  } else if (msg->center_gain_point == msg->DETECTED) {
    detected_type = "center_gain_point";
    RCLCPP_INFO(logger_, "检测到: 中心增益点");
    setOutput("detected_rfid", detected_type);
    return BT::NodeStatus::SUCCESS;
  } else if (msg->friendly_supply_zone_non_exchange == msg->DETECTED) {
    detected_type = "friendly_supply_zone_non_exchange";
    RCLCPP_INFO(logger_, "检测到: 己方补给区");
    setOutput("detected_rfid", detected_type);
    return BT::NodeStatus::SUCCESS;
  } else if (msg->friendly_supply_zone_exchange == msg->DETECTED) {
    detected_type = "friendly_supply_zone_exchange";
    RCLCPP_INFO(logger_, "检测到: 己方兑换补给区");
    setOutput("detected_rfid", detected_type);
    return BT::NodeStatus::SUCCESS;
  }

  // 检查是否有其他RFID被检测到
  std::vector<std::string> other_rfids;
  if (msg->base_gain_point) other_rfids.push_back("base_gain_point");
  if (msg->central_highland_gain_point) other_rfids.push_back("central_highland_gain_point");
  if (msg->enemy_central_highland_gain_point) other_rfids.push_back("enemy_central_highland_gain_point");
  if (msg->friendly_trapezoidal_highland_gain_point) other_rfids.push_back("friendly_trapezoidal_highland_gain_point");
  if (msg->enemy_trapezoidal_highland_gain_point) other_rfids.push_back("enemy_trapezoidal_highland_gain_point");
  if (msg->friendly_fly_ramp_front_gain_point) other_rfids.push_back("friendly_fly_ramp_front_gain_point");
  if (msg->friendly_fly_ramp_back_gain_point) other_rfids.push_back("friendly_fly_ramp_back_gain_point");
  if (msg->enemy_fly_ramp_front_gain_point) other_rfids.push_back("enemy_fly_ramp_front_gain_point");
  if (msg->enemy_fly_ramp_back_gain_point) other_rfids.push_back("enemy_fly_ramp_back_gain_point");
  if (msg->friendly_central_highland_lower_gain_point) other_rfids.push_back("friendly_central_highland_lower_gain_point");
  if (msg->friendly_central_highland_upper_gain_point) other_rfids.push_back("friendly_central_highland_upper_gain_point");
  if (msg->enemy_central_highland_lower_gain_point) other_rfids.push_back("enemy_central_highland_lower_gain_point");
  if (msg->enemy_central_highland_upper_gain_point) other_rfids.push_back("enemy_central_highland_upper_gain_point");
  if (msg->friendly_highway_lower_gain_point) other_rfids.push_back("friendly_highway_lower_gain_point");
  if (msg->friendly_highway_upper_gain_point) other_rfids.push_back("friendly_highway_upper_gain_point");
  if (msg->enemy_highway_lower_gain_point) other_rfids.push_back("enemy_highway_lower_gain_point");
  if (msg->enemy_highway_upper_gain_point) other_rfids.push_back("enemy_highway_upper_gain_point");
  if (msg->friendly_outpost_gain_point) other_rfids.push_back("friendly_outpost_gain_point");
  if (msg->friendly_big_resource_island) other_rfids.push_back("friendly_big_resource_island");
  if (msg->enemy_big_resource_island) other_rfids.push_back("enemy_big_resource_island");

  if (!other_rfids.empty()) {
    RCLCPP_WARN(logger_, "[OTHER] 检测到非目标RFID:");
    for (const auto& rfid : other_rfids) {
      RCLCPP_WARN(logger_, "  - %s", rfid.c_str());
    }
    setOutput("detected_rfid", "other");
    return BT::NodeStatus::FAILURE;
  }

  // 没有检测到任何RFID
  setOutput("detected_rfid", "none");
  return BT::NodeStatus::FAILURE;
}

BT::PortsList GetDetectedRfidAction::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RfidStatus>(
      "key_port", "{@referee_rfidStatus}", "RfidStatus port on blackboard"),
    BT::OutputPort<std::string>(
      "detected_rfid", "检测到的RFID类型: friendly_fortress_gain_point, "
      "center_gain_point, friendly_supply_zone_non_exchange, "
      "friendly_supply_zone_exchange, 或 none"),
  };
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::GetDetectedRfidAction>("GetDetectedRfid");
}
