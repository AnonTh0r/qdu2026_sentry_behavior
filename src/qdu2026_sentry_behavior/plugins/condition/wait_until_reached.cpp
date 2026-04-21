// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "qdu2026_sentry_behavior/plugins/condition/wait_until_reached.hpp"
#include <cmath>

namespace qdu2026_sentry_behavior
{

WaitUntilReachedCondition::WaitUntilReachedCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus WaitUntilReachedCondition::tick()
{
  // 读取目标位置（字符串格式 "x;y;yaw"）
  std::string goal_str;
  if (!getInput("goal", goal_str)) {
    RCLCPP_ERROR(logger_, "Failed to read 'goal' input");
    return BT::NodeStatus::FAILURE;
  }

  // 解析目标坐标
  double goal_x, goal_y;
  try {
    std::stringstream ss(goal_str);
    std::string token;

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    goal_x = std::stod(token);

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    goal_y = std::stod(token);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(logger_, "Invalid goal format '%s': %s", goal_str.c_str(), e.what());
    return BT::NodeStatus::FAILURE;
  }

  // 读取当前位置（从黑板的 odom 消息）
  nav_msgs::msg::Odometry odom;
  if (!getInput("odom", odom)) {
    RCLCPP_WARN(logger_, "Failed to read 'odom', waiting...");
    return BT::NodeStatus::RUNNING;
  }

  // 读取容差
  double tolerance = 0.5;
  getInput("tolerance", tolerance);

  // 计算距离
  double current_x = odom.pose.pose.position.x;
  double current_y = odom.pose.pose.position.y;
  double distance = std::sqrt(
    std::pow(goal_x - current_x, 2) +
    std::pow(goal_y - current_y, 2)
  );

  if (distance <= tolerance) {
    RCLCPP_INFO(logger_, "Reached goal (%.2f, %.2f), distance: %.3fm",
                goal_x, goal_y, distance);
    return BT::NodeStatus::SUCCESS;
  }

  RCLCPP_DEBUG(logger_, "Moving to goal (%.2f, %.2f), distance: %.3fm",
               goal_x, goal_y, distance);
  return BT::NodeStatus::RUNNING;
}

BT::PortsList WaitUntilReachedCondition::providedPorts()
{
  return {
    BT::InputPort<std::string>("goal", "Target position in format 'x;y;yaw'"),
    BT::InputPort<nav_msgs::msg::Odometry>("odom", "{@odom}", "Current odometry from blackboard"),
    BT::InputPort<double>("tolerance", 0.5, "Distance tolerance in meters"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::WaitUntilReachedCondition>("WaitUntilReached");
}
