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

#ifndef qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_POSE_HPP_
#define qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_POSE_HPP_

#include <string>

#include "behaviortree_ros2/bt_topic_pub_node.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

namespace qdu2026_sentry_behavior
{

/**
 * @brief Publishes a full PoseStamped message to Nav2 goal topic
 * Similar to COD's PubNav2Goal - accepts complete pose with orientation
 */
class PubNav2GoalPoseAction
  : public BT::RosTopicPubNode<geometry_msgs::msg::PoseStamped>
{
public:
  PubNav2GoalPoseAction(
    const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params);

  static BT::PortsList providedPorts();

  bool setMessage(geometry_msgs::msg::PoseStamped & msg) override;
};

}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_NAV2_GOAL_POSE_HPP_
