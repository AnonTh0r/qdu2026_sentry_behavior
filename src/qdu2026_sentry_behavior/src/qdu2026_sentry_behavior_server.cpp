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

#include "qdu2026_sentry_behavior/qdu2026_sentry_behavior_server.hpp"

#include <filesystem>
#include <fstream>

#include "auto_aim_interfaces/msg/armors.hpp"
#include "auto_aim_interfaces/msg/target.hpp"
#include "behaviortree_cpp/xml_parsing.h"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "referee_interfaces/msg/buff.hpp"
#include "referee_interfaces/msg/event_data.hpp"
#include "referee_interfaces/msg/game_robot_hp.hpp"
#include "referee_interfaces/msg/game_status.hpp"
#include "referee_interfaces/msg/ground_robot_position.hpp"
#include "referee_interfaces/msg/rfid_status.hpp"
#include "referee_interfaces/msg/robot_status.hpp"
#include "referee_interfaces/msg/set_pose.hpp"
#include "referee_interfaces/msg/operating_mode.hpp"

#include "qdu2026_sentry_behavior/plugins/condition/is_status_ok.hpp"




namespace qdu2026_sentry_behavior
{

template <typename T>
void SentryBehaviorServer::subscribe(
  const std::string & topic, const std::string & bb_key, const rclcpp::QoS & qos)
{
  auto sub = node()->create_subscription<T>(
    topic, qos,
    [this, bb_key](const typename T::SharedPtr msg)
    {
      // {@ key} 读的是 rootBlackboard()（最顶层，无父节点），即 globalBlackboard()
      // 所以必须写到 globalBlackboard() 才能被 {@ key} 读到
      globalBlackboard()->set(bb_key, *msg);
    });

  subscriptions_.push_back(sub);
}





SentryBehaviorServer::SentryBehaviorServer(const rclcpp::NodeOptions & options)
: TreeExecutionServer(options)
{
  node()->declare_parameter("use_cout_logger", false);
  node()->get_parameter("use_cout_logger", use_cout_logger_);


  // // ⭐ 关键：把 TreeExecutionServer 的 globalBlackboard 注入到 IsStatusOK 节点里
  // qdu2026_sentry_behavior::IsStatusOKCondition::setGlobalBlackboard(globalBlackboard());


  subscribe<referee_interfaces::msg::EventData>("/referee/event_data", "referee_eventData");
  subscribe<referee_interfaces::msg::GameRobotHP>("/referee/all_robot_hp", "referee_allRobotHP");
  subscribe<referee_interfaces::msg::GameStatus>("/referee/game_status", "referee_gameStatus");
  subscribe<referee_interfaces::msg::GroundRobotPosition>(
    "/referee/ground_robot_position", "referee_groundRobotPosition");
  subscribe<referee_interfaces::msg::RfidStatus>("/referee/rfid_status", "referee_rfidStatus");
  subscribe<referee_interfaces::msg::RobotStatus>("/referee/robot_status", "referee_robotStatus");
  subscribe<referee_interfaces::msg::Buff>("/referee/buff", "referee_buff");
  subscribe<referee_interfaces::msg::OperatingMode>("/referee/operating_mode", "referee_operatingMode");
  subscribe<referee_interfaces::msg::SetPose>("/referee/set_pose", "referee_pose");



  auto detector_qos = rclcpp::SensorDataQoS();
  subscribe<auto_aim_interfaces::msg::Armors>("/detector/armors", "detector_armors", detector_qos);
  auto tracker_qos = rclcpp::SensorDataQoS();
  subscribe<auto_aim_interfaces::msg::Target>("/tracker/target", "tracker_target", tracker_qos);

  auto costmap_qos = rclcpp::QoS(rclcpp::KeepLast(1)).transient_local().reliable();
  subscribe<nav_msgs::msg::OccupancyGrid>(
    "/global_costmap/costmap", "nav_globalCostmap", costmap_qos);


    
}

bool SentryBehaviorServer::onGoalReceived(
  const std::string & tree_name, const std::string & payload)
{
  RCLCPP_INFO(
    node()->get_logger(), "onGoalReceived with tree name '%s' with payload '%s'", tree_name.c_str(),
    payload.c_str());
  return true;
}

// void SentryBehaviorServer::onTreeCreated(BT::Tree & tree)
// {
//   if (use_cout_logger_) {
//     logger_cout_ = std::make_shared<BT::StdCoutLogger>(tree);
//   }
//   tick_count_ = 0;

//   // 1) 记录当前这棵树的 root blackboard
//   {
//     std::lock_guard<std::mutex> lock(bb_mutex_);
//     current_tree_bb_ = tree.rootBlackboard();
//   }

//   // 2) 启动期兜底：如果 globalBlackboard 里已经有一份 RobotStatus，就拷一份到 treeBB
//   auto global_bb = globalBlackboard();
//   if (global_bb && current_tree_bb_) {
//     try {
//       auto init_status =
//         global_bb->get<referee_interfaces::msg::RobotStatus>("referee_robotStatus");
//       current_tree_bb_->set("referee_robotStatus", init_status);

//       RCLCPP_INFO(
//         node()->get_logger(),
//         "onTreeCreated: copy initial RobotStatus hp=%d from globalBB to treeBB=%p",
//         init_status.current_hp,
//         static_cast<void*>(current_tree_bb_.get()));
//     } catch (const std::exception &)
//     {
//       // 没有这个 key 或类型不匹配就算了，不影响后面实时订阅
//     }
//   }

//   RCLCPP_INFO(
//     node()->get_logger(),
//     "onTreeCreated: globalBB=%p, treeBB=%p",
//     static_cast<void*>(globalBlackboard().get()),
//     static_cast<void*>(current_tree_bb_.get()));
// }

void SentryBehaviorServer::onTreeCreated(BT::Tree & tree)
{
  if (use_cout_logger_) {
    logger_cout_ = std::make_shared<BT::StdCoutLogger>(tree);
  }
  tick_count_ = 0;

  RCLCPP_INFO(node()->get_logger(), "onTreeCreated: globalBB=%p",
    static_cast<void*>(globalBlackboard().get()));
}



std::optional<BT::NodeStatus> SentryBehaviorServer::onLoopAfterTick(BT::NodeStatus /*status*/)
{
  ++tick_count_;
  return std::nullopt;
}

std::optional<std::string> SentryBehaviorServer::onTreeExecutionCompleted(
  BT::NodeStatus status, bool was_cancelled)
{
  RCLCPP_INFO(
    node()->get_logger(), "onTreeExecutionCompleted with status=%d (canceled=%d) after %d ticks",
    static_cast<int>(status), was_cancelled, tick_count_);
  logger_cout_.reset();
  std::string result = treeName() +
                       " tree completed with status=" + std::to_string(static_cast<int>(status)) +
                       " after " + std::to_string(tick_count_) + " ticks";
  return result;
}

}  // namespace qdu2026_sentry_behavior

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::NodeOptions options;
  auto action_server = std::make_shared<qdu2026_sentry_behavior::SentryBehaviorServer>(options);

  RCLCPP_INFO(action_server->node()->get_logger(), "Starting SentryBehaviorServer");

  rclcpp::executors::MultiThreadedExecutor exec(
    rclcpp::ExecutorOptions(), 0, false, std::chrono::milliseconds(250));
  exec.add_node(action_server->node());
  exec.spin();
  exec.remove_node(action_server->node());

  // Groot2 editor requires a model of your registered Nodes.
  // You don't need to write that by hand, it can be automatically
  // generated using the following command.
  std::string xml_models = BT::writeTreeNodesModelXML(action_server->factory());

  // Save the XML models to a file
  std::ofstream file(std::filesystem::path(ROOT_DIR) / "behavior_trees" / "models.xml");
  file << xml_models;

  rclcpp::shutdown();
}
