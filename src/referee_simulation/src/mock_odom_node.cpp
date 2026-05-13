#include <chrono>
#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"

class MockOdomNode : public rclcpp::Node
{
public:
  MockOdomNode()
  : Node("mock_odom_node"), current_x_(0.0), current_y_(0.0),
    target_x_(0.0), target_y_(0.0), has_target_(false)
  {
    declare_parameter("speed", 1.0);
    declare_parameter("update_rate", 10.0);
    declare_parameter("tolerance", 0.1);

    double rate = get_parameter("update_rate").as_double();
    int period_ms = static_cast<int>(1000.0 / rate);

    goal_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>(
      "goal_pose", 10,
      [this](const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        target_x_ = msg->pose.position.x;
        target_y_ = msg->pose.position.y;
        has_target_ = true;
        RCLCPP_INFO(get_logger(), "New goal: (%.2f, %.2f)", target_x_, target_y_);
      });

    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/odometry", 10);

    timer_ = create_wall_timer(
      std::chrono::milliseconds(period_ms),
      [this]() { timer_callback(); });

    RCLCPP_INFO(get_logger(), "Mock Odom ready, %.1f Hz", rate);
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goal_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  double current_x_, current_y_;
  double target_x_, target_y_;
  bool has_target_;

  void timer_callback()
  {
    if (has_target_) {
      double speed = get_parameter("speed").as_double();
      double tolerance = get_parameter("tolerance").as_double();
      double rate = get_parameter("update_rate").as_double();

      double dx = target_x_ - current_x_;
      double dy = target_y_ - current_y_;
      double dist = std::hypot(dx, dy);

      if (dist > tolerance) {
        double step = speed / rate;
        double frac = std::min(step / dist, 1.0);
        current_x_ += dx * frac;
        current_y_ += dy * frac;
      } else {
        current_x_ = target_x_;
        current_y_ = target_y_;
      }
    }

    auto msg = nav_msgs::msg::Odometry();
    msg.header.stamp = now();
    msg.header.frame_id = "odom";
    msg.child_frame_id = "base_link";
    msg.pose.pose.position.x = current_x_;
    msg.pose.pose.position.y = current_y_;
    msg.pose.pose.position.z = 0.0;
    msg.pose.pose.orientation.w = 1.0;

    odom_pub_->publish(msg);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MockOdomNode>());
  rclcpp::shutdown();
  return 0;
}
