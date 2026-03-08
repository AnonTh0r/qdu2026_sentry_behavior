
#include "qdu2026_sentry_behavior/plugins/action/change_pose.hpp"

namespace qdu2026_sentry_behavior
{

ChangePose::ChangePose(
  const std::string & name,
  const BT::NodeConfig & config,
  const BT::RosNodeParams & params)
: BT::RosTopicPubStatefulActionNode<referee_interfaces::msg::SetPose>(
    name, config, params)
{
}

BT::PortsList ChangePose::providedPorts()
{
  return {
    BT::InputPort<int>("current_pose", 1, "Pose mode (1: patrol, 2: attack, 3: defence)"),
    BT::InputPort<std::string>("topic_name", "/referee/set_pose", "Topic name")
  };
}

// bool ChangePose::setMessage(referee_interfaces::msg::SetPose & msg)
// {
//   int current_pose;

//   // 获取current_pose参数
//   if (!getInput("current_pose", current_pose)) {
//     RCLCPP_ERROR(node_->get_logger(), "Missing required input 'current_pose'");
//     return false;
//   }

//   // 验证取值范围 (1-3)
//   if (current_pose < 1 || current_pose > 3) {
//     RCLCPP_ERROR(
//       node_->get_logger(),
//       "Invalid current_pose value: %d (must be 1-3)", 
//       current_pose);
//     return false;
//   }

//   // 填充消息 - 只设置当前pose
//   msg.pose = static_cast<uint8_t>(current_pose);
  
//   // 如果你的msg中必须要有这三个字段，可以设置为默认值或相同值
//   msg.patrol_pose = 1;
//   msg.attack_pose = 2;
//   msg.defence_pose = 3;

//   RCLCPP_INFO(
//     node_->get_logger(),
//     "Publishing current_pose: %d", 
//     current_pose);

//   return true;
// }
bool ChangePose::setMessage(referee_interfaces::msg::SetPose & msg)
{
  int current_pose;

  if (!getInput("current_pose", current_pose)) {
    RCLCPP_ERROR(node_->get_logger(), "Missing required input 'current_pose'");
    return false;
  }

  if (current_pose < 1 || current_pose > 3) {
    RCLCPP_ERROR(
      node_->get_logger(),
      "Invalid current_pose value: %d (must be 1-3)", 
      current_pose);
    return false;
  }

  msg.pose = static_cast<uint8_t>(current_pose);
  msg.patrol_pose = 1;
  msg.attack_pose = 2;
  msg.defence_pose = 3;

  // ✅ 修改这里：根据pose_mode打印不同的姿态名称
  std::string pose_name;
  if (current_pose == 1) {
    pose_name = "巡逻姿态 (Patrol)";
  } else if (current_pose == 2) {
    pose_name = "攻击姿态 (Attack)";
  } else {
    pose_name = "防御姿态 (Defence)";
  }

  RCLCPP_INFO(
    node_->get_logger(),
    "✓ 已切换到 %s (模式: %d)", 
    pose_name.c_str(),
    current_pose);

  return true;
}


}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::ChangePose, "ChangePose");
