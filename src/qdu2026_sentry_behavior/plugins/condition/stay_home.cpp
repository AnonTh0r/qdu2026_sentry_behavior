// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "qdu2026_sentry_behavior/plugins/condition/stay_home.hpp"

namespace qdu2026_sentry_behavior
{

StayHomeCondition::StayHomeCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus StayHomeCondition::tick()
{
  int hp_min = 350;
  getInput("hp_min", hp_min);

  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    RCLCPP_ERROR(logger_, "Failed to read 'key_port'");
    return BT::NodeStatus::RUNNING;
  }

  const int hp = msg.current_hp;
  if (hp >= hp_min) {
    RCLCPP_INFO(logger_, "HP recovered to %d (>= %d), leaving home", hp, hp_min);
    return BT::NodeStatus::SUCCESS;
  }

  RCLCPP_DEBUG(logger_, "Staying home, HP=%d (waiting for %d)", hp, hp_min);
  return BT::NodeStatus::RUNNING;
}

BT::PortsList StayHomeCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("hp_min", 350, "HP threshold to leave home"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::StayHomeCondition>("StayHome");
}
