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

#include "qdu2026_sentry_behavior/plugins/action/pub_spin_speed.hpp"

namespace qdu2026_sentry_behavior
{

PublishSpinSpeedAction::PublishSpinSpeedAction(
  const std::string & name, const BT::NodeConfig & config, const BT::RosNodeParams & params)
: BT::StatefulActionNode(name, config), node_(params.nh)
{
  std::string topic_name = params.default_port_value;
  auto it = config.input_ports.find("topic_name");
  if (it != config.input_ports.end() && !it->second.empty() &&
      it->second != "__default__placeholder__")
  {
    topic_name = it->second;
  }
  publisher_ = node_->create_publisher<example_interfaces::msg::Float32>(topic_name, 1);
}

BT::PortsList PublishSpinSpeedAction::providedPorts()
{
  return {
    BT::InputPort<std::string>("topic_name", "__default__placeholder__", "Topic name"),
    BT::InputPort<double>("spin_speed", 0.0, "Angular Z velocity (rad/s)"),
  };
}

BT::NodeStatus PublishSpinSpeedAction::onStart()
{
  return onRunning();
}

BT::NodeStatus PublishSpinSpeedAction::onRunning()
{
  double spin_speed = 0.0;
  getInput("spin_speed", spin_speed);
  example_interfaces::msg::Float32 msg;
  msg.data = static_cast<float>(spin_speed);
  publisher_->publish(msg);
  return BT::NodeStatus::RUNNING;
}

void PublishSpinSpeedAction::onHalted()
{
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::PublishSpinSpeedAction, "PublishSpinSpeed");
