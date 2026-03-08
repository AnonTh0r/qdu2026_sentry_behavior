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

#ifndef qdu2026_sentry_behavior__PLUGINS__CONDITION__IS_STATUS_OK_HPP_
#define qdu2026_sentry_behavior__PLUGINS__CONDITION__IS_STATUS_OK_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "referee_interfaces/msg/game_status.hpp"
#include "referee_interfaces/msg/robot_status.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{
/**
 * @brief A BT::ConditionNode that get GameStatus from port and
 * returns SUCCESS when current game status and remain time is expected
 */
class IsStatusOKCondition : public BT::SimpleConditionNode
{
public:
  IsStatusOKCondition(const std::string & name, const BT::NodeConfig & config);

  /**
   * @brief Creates list of BT ports
   * @return BT::PortsList Containing node-specific ports
   */
  static BT::PortsList providedPorts();

  // ⭐ 新增：让 server 能把 globalBlackboard() 传进来
  static void setGlobalBlackboard(const BT::Blackboard::Ptr& bb)
  {
    s_global_bb_ = bb;
  }

private:
  /**
   * @brief Tick function for game status ports
   */

    // ⭐ 新增：所有 IsStatusOKCondition 实例共享的一份 global blackboard
  inline static BT::Blackboard::Ptr s_global_bb_;

  int hp_base_ = 400;  // 新增变量，初始化为最大血量（400）
  BT::NodeStatus checkRobotStatus();

  rclcpp::Logger logger_ = rclcpp::get_logger("IsStatusOKCondition");
};
}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__CONDITION__IS_STATUS_OK_HPP_
