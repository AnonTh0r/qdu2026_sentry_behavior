#include <rclcpp/rclcpp.hpp>
#include <example_interfaces/msg/float32.hpp>
#include <referee_interfaces/msg/robot_status.hpp>

class SpinOnAttackNode : public rclcpp::Node
{
public:
  SpinOnAttackNode() : Node("spin_on_attack")
  {
    spin_speed_ = this->declare_parameter("spin_speed", 3.0);
    cooldown_ms_ = this->declare_parameter("cooldown_ms", 2000);
    topic_name_ = this->declare_parameter("spin_topic", "cmd_spin");

    spin_pub_ = this->create_publisher<example_interfaces::msg::Float32>(topic_name_, 1);

    status_sub_ = this->create_subscription<referee_interfaces::msg::RobotStatus>(
      "/referee/robot_status", rclcpp::SensorDataQoS(),
      std::bind(&SpinOnAttackNode::onRobotStatus, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "SpinOnAttack ready, speed=%.1f, cooldown=%dms",
                spin_speed_, cooldown_ms_);
  }

private:
  void onRobotStatus(const referee_interfaces::msg::RobotStatus::SharedPtr msg)
  {
    const bool hit = msg->is_hp_deduced && msg->hp_deduction_reason == msg->ARMOR_HIT;

    if (hit) {
      auto now = this->now();
      if ((now - last_trigger_time_).seconds() * 1000.0 > cooldown_ms_) {
        last_trigger_time_ = now;
        spinning_ = true;
        spin_end_time_ = now + rclcpp::Duration::from_seconds(cooldown_ms_ / 1000.0);
        RCLCPP_INFO(this->get_logger(), "Armor hit on id=%d, spinning!", msg->armor_id);
      }
    }

    // Publish spin or stop
    example_interfaces::msg::Float32 out;
    if (spinning_ && this->now() < spin_end_time_) {
      out.data = static_cast<float>(spin_speed_);
    } else {
      spinning_ = false;
      out.data = 0.0f;
    }
    spin_pub_->publish(out);
  }

  double spin_speed_;
  int cooldown_ms_;
  std::string topic_name_;
  bool spinning_{false};
  rclcpp::Time last_trigger_time_{0, 0, RCL_ROS_TIME};
  rclcpp::Time spin_end_time_{0, 0, RCL_ROS_TIME};

  rclcpp::Publisher<example_interfaces::msg::Float32>::SharedPtr spin_pub_;
  rclcpp::Subscription<referee_interfaces::msg::RobotStatus>::SharedPtr status_sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SpinOnAttackNode>());
  rclcpp::shutdown();
  return 0;
}
