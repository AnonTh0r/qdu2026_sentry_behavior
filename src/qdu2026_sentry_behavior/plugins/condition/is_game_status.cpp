#include "qdu2026_sentry_behavior/plugins/condition/is_game_status.hpp"

#include <chrono>

namespace qdu2026_sentry_behavior
{

IsGameStatusCondition::IsGameStatusCondition(
  const std::string & name, const BT::NodeConfig & config)
: BT::SimpleConditionNode(name, std::bind(&IsGameStatusCondition::checkGameStart, this), config)
{
}

BT::NodeStatus IsGameStatusCondition::checkGameStart()
{
  int expected_game_progress, min_remain_time, max_remain_time;

  // 方案：直接从 blackboard 读取，不使用 getInput
  auto bb = config().blackboard;
  if (!bb) {
    RCLCPP_ERROR(logger_, "✗ Blackboard is NULL!");
    return BT::NodeStatus::FAILURE;
  }

  // 直接读取 referee_gameStatus
  referee_interfaces::msg::GameStatus msg;
  try {
    msg = bb->get<referee_interfaces::msg::GameStatus>("referee_gameStatus");
  } catch (const std::exception& e) {
    static auto last_error = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_error).count() >= 1) {
      RCLCPP_ERROR(logger_, "✗ Failed to read 'referee_gameStatus': %s", e.what());
      last_error = now;
    }
    return BT::NodeStatus::FAILURE;
  }

  static auto last_info = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(now - last_info).count() >= 2) {
    RCLCPP_INFO(logger_, "✓ GameStatus: progress=%d, time=%d", msg.game_progress, msg.stage_remain_time);
    last_info = now;
  }

  getInput("expected_game_progress", expected_game_progress);
  getInput("min_remain_time", min_remain_time);
  getInput("max_remain_time", max_remain_time);

  const bool is_progress_match = (msg.game_progress == expected_game_progress);
  const bool is_time_in_range =
    (msg.stage_remain_time >= min_remain_time) && (msg.stage_remain_time <= max_remain_time);

  return (is_progress_match && is_time_in_range) ? BT::NodeStatus::SUCCESS
                                                 : BT::NodeStatus::FAILURE;
}

BT::PortsList IsGameStatusCondition::providedPorts()
{
  return {
    BT::InputPort<referee_interfaces::msg::GameStatus>(
      "key_port", "{@referee_gameStatus}", "GameStatus port on blackboard"),
    BT::InputPort<int>("expected_game_progress", 4, "Expected game progress stage"),
    BT::InputPort<int>("min_remain_time", 0, "Minimum remaining time (s)"),
    BT::InputPort<int>("max_remain_time", 420, "Maximum remaining time (s)"),
  };
}
}  // namespace qdu2026_sentry_behavior

#include "behaviortree_cpp/bt_factory.h"
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<qdu2026_sentry_behavior::IsGameStatusCondition>("IsGameStatus");
}
