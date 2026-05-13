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
#include "behaviortree_ros2/ros_node_params.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

namespace qdu2026_sentry_behavior
{

class WaitUntilReachedCondition : public BT::ConditionNode
{
public:
  WaitUntilReachedCondition(
    const std::string & name, const BT::NodeConfig & config, const BT::RosNodeParams & params);
  static BT::PortsList providedPorts();

private:
  BT::NodeStatus tick() override;

  std::atomic<int> goal_status_{0};
  rclcpp::Logger logger_ = rclcpp::get_logger("WaitUntilReached");
};

}  // namespace qdu2026_sentry_behavior

#endif  // QDU2026_SENTRY_BEHAVIOR__PLUGINS__CONDITION__WAIT_UNTIL_REACHED_HPP_
