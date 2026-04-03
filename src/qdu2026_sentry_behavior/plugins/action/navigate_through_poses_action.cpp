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

#include "qdu2026_sentry_behavior/plugins/action/navigate_through_poses_action.hpp"

#include <string>
#include <vector>

#include "qdu2026_sentry_behavior/waypoint_utils.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{

NavigateThroughPosesAction::NavigateThroughPosesAction(
  const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params)
: RosActionNode<nav2_msgs::action::NavigateThroughPoses>(name, conf, params)
{
}

bool NavigateThroughPosesAction::setGoal(Goal & goal)
{
  std::string waypoint_file;
  if (!getInput("waypoint_file", waypoint_file)) {
    RCLCPP_ERROR(logger(), "NavigateThroughPoses: 'waypoint_file' input missing");
    return false;
  }

  std::string frame_id = "map";
  getInput("frame_id", frame_id);

  std::vector<geometry_msgs::msg::PoseStamped> waypoints;
  if (!loadWaypointsFromCSV(waypoint_file, waypoints, frame_id)) {
    RCLCPP_ERROR(
      logger(), "NavigateThroughPoses: failed to load '%s'", waypoint_file.c_str());
    return false;
  }

  if (waypoints.empty()) {
    RCLCPP_ERROR(
      logger(), "NavigateThroughPoses: no waypoints loaded from '%s'",
      waypoint_file.c_str());
    return false;
  }

  rclcpp::Time now = rclcpp::Clock().now();
  for (auto & wp : waypoints) {
    if (wp.header.frame_id.empty()) {
      wp.header.frame_id = frame_id;
    }
    wp.header.stamp = now;
  }

  total_waypoints_ = static_cast<int>(waypoints.size());
  goal.poses = waypoints;
  RCLCPP_INFO(
    logger(), "NavigateThroughPoses: sending %d waypoints", total_waypoints_);
  return true;
}

void NavigateThroughPosesAction::onHalt()
{
  RCLCPP_WARN(logger(), "NavigateThroughPosesAction halted");
}

BT::NodeStatus NavigateThroughPosesAction::onResultReceived(const WrappedResult & wr)
{
  if (wr.code != rclcpp_action::ResultCode::SUCCEEDED) {
    RCLCPP_WARN(
      logger(), "NavigateThroughPoses: result code %d", static_cast<int>(wr.code));
    return BT::NodeStatus::FAILURE;
  }

  RCLCPP_INFO(logger(), "NavigateThroughPoses: all waypoints traversed");
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus NavigateThroughPosesAction::onFeedback(
  const std::shared_ptr<const nav2_msgs::action::NavigateThroughPoses::Feedback> feedback)
{
  int remaining = static_cast<int>(feedback->number_of_poses_remaining);
  int current = total_waypoints_ - remaining;
  RCLCPP_DEBUG(
    logger(), "NavigateThroughPoses: waypoint %d/%d", current, total_waypoints_);
  setOutput("current_waypoint", current);
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus NavigateThroughPosesAction::onFailure(BT::ActionNodeErrorCode error)
{
  RCLCPP_ERROR(
    logger(), "NavigateThroughPosesAction failed with error code: %d", error);
  return BT::NodeStatus::FAILURE;
}

BT::PortsList NavigateThroughPosesAction::providedPorts()
{
  BT::PortsList additional_ports = {
    BT::InputPort<std::string>("waypoint_file", "CSV file path. Format: id,x,y,z,qx,qy,qz,qw"),
    BT::InputPort<std::string>("frame_id", "map", "Coordinate frame, default map"),
    BT::OutputPort<int>("current_waypoint", "Current waypoint index from feedback"),
  };
  return providedBasicPorts(additional_ports);
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(
  qdu2026_sentry_behavior::NavigateThroughPosesAction, "NavigateThroughPoses");
