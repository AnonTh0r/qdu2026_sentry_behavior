// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "qdu2026_sentry_behavior/plugins/action/write_status.hpp"

namespace qdu2026_sentry_behavior
{

WriteStatusAction::WriteStatusAction(
  const std::string & name, const BT::NodeConfig & config)
: BT::SyncActionNode(name, config)
{
}

BT::NodeStatus WriteStatusAction::tick()
{
  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    return BT::NodeStatus::SUCCESS;
  }

  setOutput("hp", static_cast<int>(msg.current_hp));
  setOutput("max_hp", static_cast<int>(msg.maximum_hp));
  return BT::NodeStatus::SUCCESS;
}

BT::PortsList WriteStatusAction::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus from blackboard"),
    BT::OutputPort<int>("hp", "{hp}", "Current HP"),
    BT::OutputPort<int>("max_hp", "{max_hp}", "Maximum HP"),
  };
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::WriteStatusAction>("WriteStatus");
}
