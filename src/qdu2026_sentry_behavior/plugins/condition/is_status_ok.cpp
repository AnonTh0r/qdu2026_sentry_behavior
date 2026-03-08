
#include "qdu2026_sentry_behavior/plugins/condition/is_status_ok.hpp"

namespace qdu2026_sentry_behavior
{




IsStatusOKCondition::IsStatusOKCondition(const std::string & name, const BT::NodeConfig & config)
: BT::SimpleConditionNode(name, std::bind(&IsStatusOKCondition::checkRobotStatus, this), config)
{
}

BT::NodeStatus IsStatusOKCondition::checkRobotStatus()
{
  
  // RCLCPP_ERROR(logger_, "IsStatusOK *** TICK ***");
  // RCLCPP_INFO(
  //   logger_,
  //   "IsStatusOK: my_bb=%p",
  //   static_cast<void*>(config().blackboard.get()));

  int hp_min   = 300;
  int heat_max = 350;
  int ammo_min = 0;

  getInput("hp_min", hp_min);
  getInput("heat_max", heat_max);
  getInput("ammo_min", ammo_min);

  auto msg = getInput<referee_interfaces::msg::RobotStatus>("key_port");
  if (!msg) {
    RCLCPP_ERROR(logger_, "IsStatusOK *** NO RobotStatus on input port (key_port) ***");
    return BT::NodeStatus::SUCCESS;
  }

  // const auto& msg = in.value();

  // RCLCPP_ERROR(
  //   logger_,
  //   "IsStatusOK: got RobotStatus from input port (hp=%d)",
  //   msg->current_hp);

  const int hp = msg->current_hp;
  const bool is_heat_ok = (msg->shooter_17mm_1_barrel_heat <= heat_max);
  const bool is_ammo_ok = (msg->projectile_allowance_17mm >= ammo_min);

  // RCLCPP_ERROR(
  //   logger_,
  //   "IsStatusOK DEBUG: hp=%d, hp_base_=%d, hp_min=%d, heat_ok=%d, ammo_ok=%d",
  //   hp, hp_base_, hp_min, is_heat_ok, is_ammo_ok);

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
      "key_port","RobotStatus port on blackboard"),
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
