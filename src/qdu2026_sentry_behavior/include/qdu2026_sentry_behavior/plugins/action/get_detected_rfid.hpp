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

#ifndef qdu2026_sentry_behavior__PLUGINS__ACTION__GET_DETECTED_RFID_HPP_
#define qdu2026_sentry_behavior__PLUGINS__ACTION__GET_DETECTED_RFID_HPP_

#include <string>

#include "behaviortree_cpp/action_node.h"
#include "referee_interfaces/msg/rfid_status.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{
/**
 * @brief A BT::SyncActionNode that reads RfidStatus from blackboard
 * and outputs the detected RFID type as a string
 */
class GetDetectedRfidAction : public BT::SyncActionNode
{
public:
  GetDetectedRfidAction(const std::string & name, const BT::NodeConfig & config);

  /**
   * @brief Creates list of BT ports
   * @return BT::PortsList Containing node-specific ports
   */
  static BT::PortsList providedPorts();

private:
  /**
   * @brief Tick function to read RFID status and output detected type
   */
  BT::NodeStatus tick() override;

  rclcpp::Logger logger_ = rclcpp::get_logger("GetDetectedRfidAction");
};
}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__ACTION__GET_DETECTED_RFID_HPP_
