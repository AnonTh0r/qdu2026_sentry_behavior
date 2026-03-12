
#include "qdu2026_sentry_behavior/plugins/condition/is_status_ok.hpp"

#include <chrono>

namespace qdu2026_sentry_behavior
{




IsStatusOKCondition::IsStatusOKCondition(const std::string & name, const BT::NodeConfig & config)
: BT::SimpleConditionNode(name, std::bind(&IsStatusOKCondition::checkRobotStatus, this), config)
{
}

BT::NodeStatus IsStatusOKCondition::checkRobotStatus()
{
  int hp_min   = 300;
  int heat_max = 350;
  int ammo_min = 0;

  getInput("hp_min", hp_min);
  getInput("heat_max", heat_max);
  getInput("ammo_min", ammo_min);

  // 使用 getInput 读取 key_port，支持 {@xxx} 语法从 globalBlackboard 读取
  referee_interfaces::msg::RobotStatus msg;
  if (!getInput("key_port", msg)) {
    static auto last_error = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_error).count() >= 1) {
      RCLCPP_ERROR(logger_, "✗ Failed to read 'key_port' (referee_robotStatus)");
      last_error = now;
    }
    return BT::NodeStatus::FAILURE;
  }

  static auto last_info = std::chrono::steady_clock::now();
  auto now2 = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(now2 - last_info).count() >= 2) {
    RCLCPP_INFO(logger_, "✓ RobotStatus: hp=%d", msg.current_hp);
    last_info = now2;
  }

  const int hp = msg.current_hp;
  const bool is_heat_ok = (msg.shooter_17mm_1_barrel_heat <= heat_max);
  const bool is_ammo_ok = (msg.projectile_allowance_17mm >= ammo_min);

  if (hp >= hp_min && is_heat_ok && is_ammo_ok) {
    RCLCPP_INFO(logger_, "[IsStatusOK SIMPLE] RESULT: SUCCESS");
    return BT::NodeStatus::SUCCESS;
  } else {
    RCLCPP_INFO(logger_, "[IsStatusOK SIMPLE] RESULT: FAILURE");
    return BT::NodeStatus::FAILURE;
  }
}



BT::PortsList IsStatusOKCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("hp_min", 300, "Minimum HP. NOTE: Sentry init/max HP is 400"),
    BT::InputPort<int>("heat_max", 350, "Maximum heat. NOTE: Sentry heat limit is 400"),
    BT::InputPort<int>("ammo_min", 0, "Lower then minimum ammo will return FAILURE")};
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsStatusOKCondition>("IsStatusOK");
}
