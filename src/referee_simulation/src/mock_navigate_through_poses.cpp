#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "std_msgs/msg/empty.hpp"
#include "nav2_msgs/action/navigate_through_poses.hpp"

using NavigateThroughPoses = nav2_msgs::action::NavigateThroughPoses;
using GoalHandle = rclcpp_action::ServerGoalHandle<NavigateThroughPoses>;

class MockNavigateThroughPoses : public rclcpp::Node
{
public:
  MockNavigateThroughPoses()
  : Node("mock_navigate_through_poses"), trigger_fail_(false)
  {
    declare_parameter("delay_per_pose", 1.0);

    action_server_ = rclcpp_action::create_server<NavigateThroughPoses>(
      this, "navigate_through_poses",
      std::bind(&MockNavigateThroughPoses::handleGoal, this,
        std::placeholders::_1, std::placeholders::_2),
      std::bind(&MockNavigateThroughPoses::handleCancel, this,
        std::placeholders::_1),
      std::bind(&MockNavigateThroughPoses::handleAccepted, this,
        std::placeholders::_1));

    fail_sub_ = create_subscription<std_msgs::msg::Empty>(
      "mock_nav/trigger_fail", 10,
      [this](const std_msgs::msg::Empty::SharedPtr) {
        trigger_fail_.store(true);
        RCLCPP_WARN(get_logger(), "Failure triggered! Will abort current goal.");
      });

    RCLCPP_INFO(get_logger(), "Mock NavigateThroughPoses server ready");
    RCLCPP_INFO(get_logger(),
      "  Send to 'mock_nav/trigger_fail' to simulate failure");
  }

private:
  rclcpp_action::Server<NavigateThroughPoses>::SharedPtr action_server_;
  rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr fail_sub_;
  std::atomic<bool> trigger_fail_;

  rclcpp_action::GoalResponse handleGoal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const NavigateThroughPoses::Goal> goal)
  {
    RCLCPP_INFO(get_logger(), "Received goal with %zu poses",
      goal->poses.size());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handleCancel(
    const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "Cancel requested");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handleAccepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    std::thread([this, goal_handle]() { execute(goal_handle); }).detach();
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    double delay = get_parameter("delay_per_pose").as_double();
    auto & poses = goal_handle->get_goal()->poses;
    int total = static_cast<int>(poses.size());

    RCLCPP_INFO(get_logger(), "Executing: %d poses, %.1fs each", total, delay);

    auto feedback = std::make_shared<NavigateThroughPoses::Feedback>();
    auto result = std::make_shared<NavigateThroughPoses::Result>();

    for (int i = 0; i < total; ++i) {
      if (goal_handle->is_canceling()) {
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Goal canceled");
        return;
      }

      if (trigger_fail_.exchange(false)) {
        goal_handle->abort(result);
        RCLCPP_WARN(get_logger(), "Simulated failure at pose %d", i);
        return;
      }

      feedback->number_of_poses_remaining = total - i - 1;
      goal_handle->publish_feedback(feedback);
      RCLCPP_INFO(get_logger(), "  Pose %d/%d: (%.2f, %.2f) remaining: %d",
        i + 1, total,
        poses[i].pose.position.x, poses[i].pose.position.y,
        feedback->number_of_poses_remaining);

      std::this_thread::sleep_for(
        std::chrono::milliseconds(static_cast<int>(delay * 1000)));
    }

    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "All poses reached");
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MockNavigateThroughPoses>());
  rclcpp::shutdown();
  return 0;
}
