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

#include "qdu2026_sentry_behavior/plugins/action/follow_waypoints_action.hpp"

#include <string>
#include <vector>

#include "qdu2026_sentry_behavior/waypoint_utils.hpp"
#include "rclcpp/rclcpp.hpp"

namespace qdu2026_sentry_behavior
{

FollowWaypointsAction::FollowWaypointsAction(
  const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params)
: RosActionNode<nav2_msgs::action::FollowWaypoints>(name, conf, params)
{
}

bool FollowWaypointsAction::setGoal(Goal & goal)
{
  std::string waypoint_file;
  if (!getInput("waypoint_file", waypoint_file)) {
    RCLCPP_ERROR(logger(), "FollowWaypoints: 'waypoint_file' input missing");
    return false;
  }

  std::string frame_id = "map";
  getInput("frame_id", frame_id);

  std::vector<geometry_msgs::msg::PoseStamped> waypoints;
  if (!loadWaypointsFromCSV(waypoint_file, waypoints, frame_id)) {
    RCLCPP_ERROR(logger(), "FollowWaypoints: failed to load '%s'", waypoint_file.c_str());
    return false;
  }

  if (waypoints.empty()) {
    RCLCPP_ERROR(logger(), "FollowWaypoints: no waypoints loaded from '%s'", waypoint_file.c_str());
    return false;
  }

  rclcpp::Time now = rclcpp::Clock().now();
  for (auto & wp : waypoints) {
    if (wp.header.frame_id.empty()) {
      wp.header.frame_id = frame_id;
    }
    wp.header.stamp = now;
  }

  goal.poses = waypoints;
  RCLCPP_INFO(logger(), "FollowWaypoints: sending %zu waypoints", waypoints.size());
  return true;
}

void FollowWaypointsAction::onHalt()
{
  RCLCPP_WARN(logger(), "FollowWaypointsAction halted");
}

BT::NodeStatus FollowWaypointsAction::onResultReceived(const WrappedResult & wr)
{
  if (wr.code != rclcpp_action::ResultCode::SUCCEEDED) {
    RCLCPP_WARN(
      logger(), "FollowWaypoints: result code %d", static_cast<int>(wr.code));
    return BT::NodeStatus::FAILURE;
  }

  if (wr.result && !wr.result->missed_waypoints.empty()) {
    RCLCPP_WARN(
      logger(), "FollowWaypoints: %zu waypoint(s) missed",
      wr.result->missed_waypoints.size());
    return BT::NodeStatus::FAILURE;
  }

  RCLCPP_INFO(logger(), "FollowWaypoints: all waypoints reached");
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus FollowWaypointsAction::onFeedback(
  const std::shared_ptr<const nav2_msgs::action::FollowWaypoints::Feedback> feedback)
{
  RCLCPP_DEBUG(
    logger(), "FollowWaypoints: current waypoint %u",
    static_cast<unsigned>(feedback->current_waypoint));
  setOutput("current_waypoint", static_cast<int>(feedback->current_waypoint));
  return BT::NodeStatus::RUNNING;
}

BT::NodeStatus FollowWaypointsAction::onFailure(BT::ActionNodeErrorCode error)
{
  RCLCPP_ERROR(logger(), "FollowWaypointsAction failed with error code: %d", error);
  return BT::NodeStatus::FAILURE;
}

BT::PortsList FollowWaypointsAction::providedPorts()
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
CreateRosNodePlugin(qdu2026_sentry_behavior::FollowWaypointsAction, "FollowWaypoints");
