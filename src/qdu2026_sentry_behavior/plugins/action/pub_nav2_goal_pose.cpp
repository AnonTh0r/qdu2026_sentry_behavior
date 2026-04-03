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

#include "qdu2026_sentry_behavior/plugins/action/pub_nav2_goal_pose.hpp"

namespace qdu2026_sentry_behavior
{

PubNav2GoalPoseAction::PubNav2GoalPoseAction(
  const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params)
: RosTopicPubNode<geometry_msgs::msg::PoseStamped>(name, conf, params)
{
}

bool PubNav2GoalPoseAction::setMessage(geometry_msgs::msg::PoseStamped & msg)
{
  auto res = getInput<geometry_msgs::msg::PoseStamped>("goal_pose");
  if (!res) {
    RCLCPP_ERROR(
      node_->get_logger(), "PubNav2GoalPose: Failed to read 'goal_pose': %s", res.error().c_str());
    return false;
  }

  msg = res.value();

  if (msg.header.frame_id.empty()) {
    std::string frame_id = "map";
    getInput("frame_id", frame_id);
    msg.header.frame_id = frame_id;
  }

  msg.header.stamp = node_->now();

  RCLCPP_INFO(
    node_->get_logger(), "PubNav2GoalPose: Publishing goal [%.2f, %.2f, %.2f] frame: %s",
    msg.pose.position.x, msg.pose.position.y, msg.pose.position.z, msg.header.frame_id.c_str());

  return true;
}

BT::PortsList PubNav2GoalPoseAction::providedPorts()
{
  return providedBasicPorts({
    BT::InputPort<geometry_msgs::msg::PoseStamped>("goal_pose", "Navigation goal pose"),
    BT::InputPort<std::string>("frame_id", "map", "Frame ID, default is map")
  });
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::PubNav2GoalPoseAction, "PubNav2GoalPose");
