// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef QDU2026_SENTRY_BEHAVIOR__PLUGINS__CONDITION__WAIT_UNTIL_REACHED_HPP_
#define QDU2026_SENTRY_BEHAVIOR__PLUGINS__CONDITION__WAIT_UNTIL_REACHED_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{

class WaitUntilReachedCondition : public BT::ConditionNode
{
public:
  WaitUntilReachedCondition(const std::string & name, const BT::NodeConfig & config);
  static BT::PortsList providedPorts();

private:
  BT::NodeStatus tick() override;
  rclcpp::Logger logger_ = rclcpp::get_logger("WaitUntilReached");
};

}  // namespace qdu2026_sentry_behavior

#endif  // QDU2026_SENTRY_BEHAVIOR__PLUGINS__CONDITION__WAIT_UNTIL_REACHED_HPP_
