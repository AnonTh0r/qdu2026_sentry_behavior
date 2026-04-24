#include "qdu2026_sentry_behavior/plugins/action/pub_nav2_goal.hpp"

#include "qdu2026_sentry_behavior/custom_types.hpp"
#include <string>
#include <sstream>


namespace qdu2026_sentry_behavior
{

PubNav2GoalAction::PubNav2GoalAction(
  const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params)
: RosTopicPubNode<geometry_msgs::msg::PoseStamped>(name, conf, params)
{
}

// bool PubNav2GoalAction::setMessage(geometry_msgs::msg::PoseStamped & msg)
// {
//   auto goal = getInput<geometry_msgs::msg::PoseStamped>("goal");

//   msg.header.stamp = now();
//   msg.header.frame_id = "map";
//   msg.pose = goal->pose;
//   return true;
// }
bool PubNav2GoalAction::setMessage(geometry_msgs::msg::PoseStamped & msg)
{
  // 1. 读取字符串端口
  std::string goal_str;
  if (!getInput("goal", goal_str))
  {
    RCLCPP_ERROR(node_->get_logger(), "PubNav2Goal: 'goal' input missing");
    return false;
  }

  // 2. 解析 x;y
  double x, y;
  try
  {
    std::stringstream ss(goal_str);
    std::string token;

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    x = std::stod(token);

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    y = std::stod(token);
  }
  catch (const std::exception & e)
  {
    RCLCPP_ERROR(node_->get_logger(), "PubNav2Goal: invalid 'goal' = '%s' (%s)",
                 goal_str.c_str(), e.what());
    return false;
  }

  // 3. 填 PoseStamped —— 直接用 Nav2 的 global_frame: "map"
  msg.header.frame_id = "map";          // 和 bt_navigator.global_frame 保持一致
  // msg.header.stamp = now();
  
  // 关键：把时间戳设为 0，让 Nav2 当成“用最新 TF”
  msg.header.stamp.sec = 0;
  msg.header.stamp.nanosec = 0;

  msg.pose.position.x = x;
  msg.pose.position.y = y;
  msg.pose.position.z = 0.0;

  msg.pose.orientation.w = 1.0;

  RCLCPP_INFO(node_->get_logger(), "PubNav2Goal publishing: goal='%s' (x=%.2f, y=%.2f)",
              goal_str.c_str(), x, y);

  return true;
}

// BT::PortsList PubNav2GoalAction::providedPorts()
// {
//   BT::PortsList additional_ports = {
//     BT::InputPort<geometry_msgs::msg::PoseStamped>(
//       "goal", "0;0;0", "Expected goal pose that send to nav2. Fill with format `x;y;yaw`"),
//   };
//   return providedBasicPorts(additional_ports);
// }
BT::PortsList PubNav2GoalAction::providedPorts()
{
  BT::PortsList additional_ports = {
    BT::InputPort<std::string>(
      "goal",
      "0;0;0",   // 默认发原点，可按需修改
      "Expected goal pose that send to nav2. "
      "Format `x;y;yaw` in map frame"),
  };
  return providedBasicPorts(additional_ports);
}


}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::PubNav2GoalAction, "PubNav2Goal");
