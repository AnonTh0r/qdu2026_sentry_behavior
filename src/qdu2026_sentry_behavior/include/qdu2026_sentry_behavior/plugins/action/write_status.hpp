// Copyright 2025 Lihan Chen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef QDU2026_SENTRY_BEHAVIOR__PLUGINS__ACTION__WRITE_STATUS_HPP_
#define QDU2026_SENTRY_BEHAVIOR__PLUGINS__ACTION__WRITE_STATUS_HPP_

#include <string>

#include "behaviortree_cpp/action_node.h"
#include "referee_interfaces/msg/robot_status.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{

class WriteStatusAction : public BT::SyncActionNode
{
public:
  WriteStatusAction(const std::string & name, const BT::NodeConfig & config);
  static BT::PortsList providedPorts();

private:
  BT::NodeStatus tick() override;
  rclcpp::Logger logger_ = rclcpp::get_logger("WriteStatus");
};

}  // namespace qdu2026_sentry_behavior

#endif  // QDU2026_SENTRY_BEHAVIOR__PLUGINS__ACTION__WRITE_STATUS_HPP_
