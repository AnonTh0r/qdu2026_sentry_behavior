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

#ifndef qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_GIMBAL_VELOCITY_HPP_
#define qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_GIMBAL_VELOCITY_HPP_

#include <string>

#include "behaviortree_ros2/bt_topic_pub_action_node.hpp"
#include "referee_interfaces/msg/set_pose.hpp"

namespace qdu2026_sentry_behavior
{

class ChangePose
: public BT::RosTopicPubStatefulActionNode<referee_interfaces::msg::SetPose>
{
public:
    ChangePose(
    const std::string & name, const BT::NodeConfig & config, const BT::RosNodeParams & params);

  static BT::PortsList providedPorts();

protected:
  bool setMessage(referee_interfaces::msg::SetPose & msg) override;

  // StatefulActionNode 需要实现的方法
  BT::NodeStatus onStart() override {
    // 发布消息后立即返回 SUCCESS
    return BT::NodeStatus::SUCCESS;
  }

  BT::NodeStatus onRunning() override {
    // 不应该被调用，因为 onStart 返回 SUCCESS
    return BT::NodeStatus::SUCCESS;
  }

  void onHalted() override {
    // 清理资源（如果需要）
  }
};

}  // namespace qdu2026_sentry_behavior

#endif  // qdu2026_sentry_behavior__PLUGINS__ACTION__PUB_GIMBAL_VELOCITY_HPP_
