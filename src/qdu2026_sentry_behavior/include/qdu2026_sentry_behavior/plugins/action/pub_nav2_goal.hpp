// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_HPP_
#define qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_HPP_

#include <string>

#include "behaviortree_cpp/condition_node.h"
#include "behaviortree_ros2/ros_node_params.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{
class PubNav2GoalAction : public BT::ConditionNode
{
public:
  PubNav2GoalAction(
    const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params);

  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  bool createPublisher(const std::string & topic_name);
  bool setMessage(geometry_msgs::msg::PoseStamped & goal);

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr publisher_;
  std::string prev_topic_name_;
  bool topic_name_may_change_{false};

  int republish_ms_{0};
  rclcpp::Time last_publish_time_{0, 0, RCL_ROS_TIME};
};
}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_HPP_
