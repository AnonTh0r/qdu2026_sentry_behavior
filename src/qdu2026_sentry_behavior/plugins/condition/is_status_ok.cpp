
#include "qdu2026_sentry_behavior/plugins/condition/is_status_ok.hpp"

#include <chrono>

namespace qdu2026_sentry_behavior
{




IsStatusOKCondition::IsStatusOKCondition(const std::string & name, const BT::NodeConfig & config)
: BT::ConditionNode(name, config)
{
}

BT::NodeStatus IsStatusOKCondition::tick()
{
  int hp_min = 300;
  getInput("hp_min", hp_min);

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

  const int hp = msg.current_hp;

  if (hp >= hp_min) {
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::FAILURE;
}



BT::PortsList IsStatusOKCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::RobotStatus>(
      "key_port", "{@referee_robotStatus}", "RobotStatus port on blackboard"),
    BT::InputPort<int>("hp_min", 300, "Minimum HP. NOTE: Sentry init/max HP is 400")};
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsStatusOKCondition>("IsStatusOK");
}
