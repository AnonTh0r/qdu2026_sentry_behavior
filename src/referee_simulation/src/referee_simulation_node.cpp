#include <chrono>
#include <memory>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

#include "referee_interfaces/msg/game_status.hpp"
#include "referee_interfaces/msg/game_robot_hp.hpp"
#include "referee_interfaces/msg/robot_status.hpp"
#include "referee_interfaces/msg/buff.hpp"
#include "referee_interfaces/msg/event_data.hpp"
#include "referee_interfaces/msg/set_pose.hpp"
#include "referee_interfaces/msg/ground_robot_position.hpp"
#include "referee_interfaces/msg/rfid_status.hpp"
#include "referee_interfaces/msg/operating_mode.hpp"

using namespace std::chrono_literals;

class RefereeSimulationNode : public rclcpp::Node
{
public:
  RefereeSimulationNode();

private:
  // 定时器回调：周期性发布所有裁判消息
  void onTimer();

  // 各种消息的发布函数
  void publishGameStatus();
  void publishGameRobotHP();
  void publishRobotStatus();
  void publishBuff();
  void publishEventData();
  // void publishSetPose();
  // void publishGroundRobotPosition();
  void publishRfidStatus();
  // void publishOperatingMode();

  // 新增：动态修改 HP 的订阅回调
  void onHpCmd(const std_msgs::msg::Int32::SharedPtr msg);

  // publishers
  rclcpp::Publisher<referee_interfaces::msg::GameStatus>::SharedPtr game_status_pub_;
  rclcpp::Publisher<referee_interfaces::msg::GameRobotHP>::SharedPtr game_robot_hp_pub_;
  rclcpp::Publisher<referee_interfaces::msg::EventData>::SharedPtr event_data_pub_;
  rclcpp::Publisher<referee_interfaces::msg::RobotStatus>::SharedPtr robot_status_pub_;
  rclcpp::Publisher<referee_interfaces::msg::Buff>::SharedPtr buff_pub_;
  rclcpp::Publisher<referee_interfaces::msg::SetPose>::SharedPtr set_pose_pub_;
  rclcpp::Publisher<referee_interfaces::msg::GroundRobotPosition>::SharedPtr ground_robot_pos_pub_;
  rclcpp::Publisher<referee_interfaces::msg::RfidStatus>::SharedPtr rfid_status_pub_;
  rclcpp::Publisher<referee_interfaces::msg::OperatingMode>::SharedPtr operating_mode_pub_;

  // 新增：订阅“设置 HP”的命令
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr hp_cmd_sub_;

  // time & 状态
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Time start_time_;
  int32_t game_time_remaining_;
  uint16_t self_hp_;   // 当前模拟血量
};

RefereeSimulationNode::RefereeSimulationNode()
: Node("referee_simulation_node"),
  game_time_remaining_(300),
  self_hp_(400)   // 初始 HP，比如 400
{
  // 话题名必须和 qdu2026_sentry_behavior_server 里订阅的一致
  game_status_pub_ = create_publisher<referee_interfaces::msg::GameStatus>(
    "referee/game_status", 10);
  game_robot_hp_pub_ = create_publisher<referee_interfaces::msg::GameRobotHP>(
    "referee/all_robot_hp", 10);
  event_data_pub_ = create_publisher<referee_interfaces::msg::EventData>(
    "referee/event_data", 10);
  robot_status_pub_ = create_publisher<referee_interfaces::msg::RobotStatus>(
    "referee/robot_status", 10);
  buff_pub_ = create_publisher<referee_interfaces::msg::Buff>(
    "referee/buff", 10);
  set_pose_pub_ = create_publisher<referee_interfaces::msg::SetPose>(
    "referee/set_pose", 10);
  ground_robot_pos_pub_ = create_publisher<referee_interfaces::msg::GroundRobotPosition>(
    "referee/ground_robot_position", 10);
  rfid_status_pub_ = create_publisher<referee_interfaces::msg::RfidStatus>(
    "referee/rfid_status", 10);
  operating_mode_pub_ = create_publisher<referee_interfaces::msg::OperatingMode>(
    "referee/operating_mode", 10);

  // 新增：订阅一个控制 HP 的话题
  hp_cmd_sub_ = create_subscription<std_msgs::msg::Int32>(
    "referee_simulation/set_hp",   // 以后从终端往这个话题发值就行
    10,
    std::bind(&RefereeSimulationNode::onHpCmd, this, std::placeholders::_1));

  timer_ = create_wall_timer(
    100ms, std::bind(&RefereeSimulationNode::onTimer, this));

  start_time_ = now();
}

void RefereeSimulationNode::onTimer()
{
  auto elapsed = now() - start_time_;
  int32_t elapsed_sec = static_cast<int32_t>(elapsed.seconds());
  game_time_remaining_ = std::max(0, 300 - elapsed_sec);

  // 示例：每 0.5s 掉 50 HP（如果 >0）
  // static int tick_count = 0;
  // tick_count++;
  // if (game_time_remaining_ > 0 && self_hp_ > 0) {
  //   if (tick_count % 5 == 0) { // 5 * 100ms = 0.5 秒
  //     if (self_hp_ > 50) {
  //       self_hp_ -= 50;
  //     } else {
  //       self_hp_ = 0;
  //     }
  //     RCLCPP_INFO(get_logger(), "Sim HP: %u", self_hp_);
  //   }
  // }

  publishGameStatus();
  publishGameRobotHP();
  publishRobotStatus();
  publishBuff();
  publishEventData();
  // publishSetPose();
  // publishGroundRobotPosition();
  publishRfidStatus();
  // publishOperatingMode();
}

void RefereeSimulationNode::onHpCmd(const std_msgs::msg::Int32::SharedPtr msg)
{
  int32_t hp = msg->data;//？？？
  if (hp < 0) hp = 0;
  if (hp > 400) hp = 400;   // 换成你机器的最大 HP 也可以

  self_hp_ = static_cast<uint16_t>(hp);
  RCLCPP_INFO(get_logger(), "Set HP manually to %d", hp);
}

void RefereeSimulationNode::publishGameStatus()
{
  referee_interfaces::msg::GameStatus msg;

  if (game_time_remaining_ <= 0) {
    msg.game_progress = referee_interfaces::msg::GameStatus::GAME_OVER;
    msg.stage_remain_time = 0;
  } else {
    msg.game_progress = referee_interfaces::msg::GameStatus::RUNNING;
    msg.stage_remain_time = game_time_remaining_;
  }

  game_status_pub_->publish(msg);
}

void RefereeSimulationNode::publishGameRobotHP()
{
  referee_interfaces::msg::GameRobotHP msg;

  // 假设自己是红方 7 号哨兵，自身 HP 用 self_hp_
  msg.red_1_robot_hp = 400;
  msg.red_2_robot_hp = 400;
  msg.red_3_robot_hp = 400;
  msg.red_4_robot_hp = 400;
  msg.red_7_robot_hp = self_hp_;
  msg.red_outpost_hp = 5000;
  msg.red_base_hp = 8000;

  msg.blue_1_robot_hp = 400;
  msg.blue_2_robot_hp = 400;
  msg.blue_3_robot_hp = 400;
  msg.blue_4_robot_hp = 400;
  msg.blue_7_robot_hp = 400;
  msg.blue_outpost_hp = 5000;
  msg.blue_base_hp = 8000;

  game_robot_hp_pub_->publish(msg);
}

void RefereeSimulationNode::publishRobotStatus()
{
  referee_interfaces::msg::RobotStatus msg;

  msg.robot_id = 7;
  msg.robot_level = 1;
  msg.current_hp = self_hp_;
  msg.maximum_hp = 400;
  msg.shooter_barrel_cooling_value = 50;
  msg.shooter_barrel_heat_limit = 200;

  msg.shooter_17mm_1_barrel_heat = 0;

  msg.robot_pos.position.x = 0.0;
  msg.robot_pos.position.y = 0.0;
  msg.robot_pos.position.z = 0.0;
  msg.robot_pos.orientation.w = 1.0;

  msg.armor_id = 0;
  msg.hp_deduction_reason = 0;
  msg.projectile_allowance_17mm = 500;
  msg.remaining_gold_coin = 0;
  msg.is_hp_deduced = (self_hp_ < msg.maximum_hp);

  robot_status_pub_->publish(msg);
}

void RefereeSimulationNode::publishBuff()
{
  referee_interfaces::msg::Buff msg;
  msg.recovery_buff = 0;
  msg.cooling_buff = 0;
  msg.defence_buff = 0;
  msg.vulnerability_buff = 0;
  msg.attack_buff = 0;
  msg.remaining_energy = 0x32;
  buff_pub_->publish(msg);
}

void RefereeSimulationNode::publishEventData()
{
  referee_interfaces::msg::EventData msg;
  msg.non_overlapping_supply_zone = 0;
  msg.overlapping_supply_zone = 0;
  msg.supply_zone = 0;
  msg.small_energy = 0;
  msg.big_energy = 0;
  msg.central_highland = 0;
  msg.trapezoidal_highland = 0;
  msg.center_gain_zone = 0;
  event_data_pub_->publish(msg);
}

// void RefereeSimulationNode::publishSetPose()
// {
//   referee_interfaces::msg::SetPose msg;
//   msg.patrol_pose = 0;
//   msg.attack_pose = 1;
//   msg.defence_pose = 2;
//   msg.pose = msg.patrol_pose;
//   set_pose_pub_->publish(msg);
// }

// void RefereeSimulationNode::publishGroundRobotPosition()
// {
//   referee_interfaces::msg::GroundRobotPosition msg;
//   msg.robot_type = 7;
//   msg.x = 0.0;
//   msg.y = 0.0;
//   msg.z = 0.0;
//   ground_robot_pos_pub_->publish(msg);
// }

void RefereeSimulationNode::publishRfidStatus()
{
  referee_interfaces::msg::RfidStatus msg;
  // 默认全 0
  rfid_status_pub_->publish(msg);
}

// void RefereeSimulationNode::publishOperatingMode()
// {
//   referee_interfaces::msg::OperatingMode msg;
//   msg.mode = 0;  // 比如：0 表示正常比赛模式
//   operating_mode_pub_->publish(msg);
// }

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RefereeSimulationNode>());
  rclcpp::shutdown();
  return 0;
}
