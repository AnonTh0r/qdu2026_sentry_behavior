// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "qdu2026_sentry_behavior/plugins/condition/wait_until_reached.hpp"

namespace qdu2026_sentry_behavior
{

WaitUntilReachedCondition::WaitUntilReachedCondition(
  const std::string & name, const BT::NodeConfig & config, const BT::RosNodeParams & params)
: BT::ConditionNode(name, config)
{
  auto node = params.nh.lock();
  node->create_subscription<std_msgs::msg::Int32>(
    "/goal_arrive", 10,
    [this](const std_msgs::msg::Int32::SharedPtr msg) {
      goal_status_.store(msg->data);
    });
}

BT::NodeStatus WaitUntilReachedCondition::tick()
{
  int status = goal_status_.load();

  if (status == 1) {
    goal_status_.store(0);
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::RUNNING;
}

BT::PortsList WaitUntilReachedCondition::providedPorts()
{
  return {};
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::WaitUntilReachedCondition, "WaitUntilReached");
