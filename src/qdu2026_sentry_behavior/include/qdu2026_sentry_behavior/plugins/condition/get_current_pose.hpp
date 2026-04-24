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

#ifndef qdu2026_sentry_behavior__PLUGINS__CONDITION__GET_CURRENT_POSE_HPP_
#define qdu2026_sentry_behavior__PLUGINS__CONDITION__GET_CURRENT_POSE_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "referee_interfaces/msg/sentry_state.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{
/**
 * @brief A BT::ConditionNode that reads current_state from blackboard
 * and outputs the current pose mode
 */
class GetCurrentPoseCondition : public BT::ConditionNode
{
public:
  GetCurrentPoseCondition(const std::string & name, const BT::NodeConfig & config);

  static BT::PortsList providedPorts();

private:
  BT::NodeStatus tick() override;

  rclcpp::Logger logger_ = rclcpp::get_logger("GetCurrentPoseCondition");
};
}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__CONDITION__GET_CURRENT_POSE_HPP_
