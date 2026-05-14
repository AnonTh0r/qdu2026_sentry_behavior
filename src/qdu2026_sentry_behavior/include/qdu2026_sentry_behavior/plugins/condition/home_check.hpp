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

#ifndef qdu2026_sentry_behavior__PLUGINS__CONDITION__HOME_CHECK_HPP_
#define qdu2026_sentry_behavior__PLUGINS__CONDITION__HOME_CHECK_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "referee_interfaces/msg/game_robot_hp.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{

/**
 * @brief Checks if base and outpost HP are above safe thresholds.
 * Returns SUCCESS when both are safe, FAILURE otherwise.
 */
class HomeCheckCondition : public BT::ConditionNode
{
public:
  HomeCheckCondition(const std::string & name, const BT::NodeConfig & config);

  static BT::PortsList providedPorts();

  BT::NodeStatus tick() override;

private:
  rclcpp::Logger logger_ = rclcpp::get_logger("HomeCheckCondition");
};

}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__CONDITION__HOME_CHECK_HPP_
