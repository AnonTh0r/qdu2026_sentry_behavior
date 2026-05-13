#include "qdu2026_sentry_behavior/plugins/action/pub_nav2_goal.hpp"

#include <string>
#include <sstream>

namespace qdu2026_sentry_behavior
{

PubNav2GoalAction::PubNav2GoalAction(
  const std::string & name, const BT::NodeConfig & conf, const BT::RosNodeParams & params)
: BT::ConditionNode(name, conf), node_(params.nh)
{
  republish_ms_ = conf.input_ports.count("republish_ms")
                    ? static_cast<int>(std::stod(conf.input_ports.at("republish_ms")))
                    : 0;

  auto portIt = config().input_ports.find("topic_name");
  if (portIt != config().input_ports.end()) {
    const std::string & bb_topic_name = portIt->second;
    if (!bb_topic_name.empty() && bb_topic_name != "__default__placeholder__") {
      if (!isBlackboardPointer(bb_topic_name)) {
        createPublisher(bb_topic_name);
      } else {
        topic_name_may_change_ = true;
      }
      return;
    }
  }
  if (params.default_port_value.empty()) {
    throw std::logic_error(
      "Both [topic_name] in the InputPort and the RosNodeParams are empty.");
  }
  createPublisher(params.default_port_value);
}

bool PubNav2GoalAction::createPublisher(const std::string & topic_name)
{
  if (topic_name.empty()) {
    throw BT::RuntimeError("topic_name is empty");
  }
  publisher_ = node_->create_publisher<geometry_msgs::msg::PoseStamped>(topic_name, 1);
  prev_topic_name_ = topic_name;
  return true;
}

bool PubNav2GoalAction::setMessage(geometry_msgs::msg::PoseStamped & msg)
{
  std::string goal_str;
  if (!getInput("goal", goal_str)) {
    RCLCPP_ERROR(node_->get_logger(), "PubNav2Goal: 'goal' input missing");
    return false;
  }

  double x, y;
  try {
    std::stringstream ss(goal_str);
    std::string token;

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    x = std::stod(token);

    if (!std::getline(ss, token, ';')) throw std::runtime_error("bad format");
    y = std::stod(token);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(node_->get_logger(), "PubNav2Goal: invalid 'goal' = '%s' (%s)",
                 goal_str.c_str(), e.what());
    return false;
  }

  msg.header.frame_id = "map";
  msg.header.stamp.sec = 0;
  msg.header.stamp.nanosec = 0;

  msg.pose.position.x = x;
  msg.pose.position.y = y;
  msg.pose.position.z = 0.0;

  msg.pose.orientation.w = 1.0;

  return true;
}

BT::NodeStatus PubNav2GoalAction::tick()
{
  if (!publisher_ || (status() == BT::NodeStatus::IDLE && topic_name_may_change_)) {
    std::string topic_name;
    getInput("topic_name", topic_name);
    if (prev_topic_name_ != topic_name) {
      createPublisher(topic_name);
    }
  }

  if (republish_ms_ > 0) {
    auto now = node_->now();
    if (last_publish_time_.nanoseconds() == 0 ||
        (now - last_publish_time_).seconds() * 1000.0 >= republish_ms_) {
      geometry_msgs::msg::PoseStamped msg;
      if (!setMessage(msg)) {
        return BT::NodeStatus::FAILURE;
      }
      publisher_->publish(msg);
      last_publish_time_ = now;
    }
    return BT::NodeStatus::RUNNING;
  }

  geometry_msgs::msg::PoseStamped msg;
  if (!setMessage(msg)) {
    return BT::NodeStatus::FAILURE;
  }
  publisher_->publish(msg);
  return BT::NodeStatus::SUCCESS;
}

BT::PortsList PubNav2GoalAction::providedPorts()
{
  BT::PortsList ports = {
    BT::InputPort<std::string>(
      "goal", "0;0;0",
      "Expected goal pose that send to nav2. Format `x;y;yaw` in map frame"),
    BT::InputPort<std::string>(
      "topic_name", "__default__placeholder__",
      "Topic name to publish to"),
    BT::InputPort<std::string>(
      "republish_ms", "0",
      "If > 0, republish at this interval (ms) and stay RUNNING"),
  };
  return ports;
}

}  // namespace qdu2026_sentry_behavior

#include "behaviortree_ros2/plugins.hpp"
CreateRosNodePlugin(qdu2026_sentry_behavior::PubNav2GoalAction, "PubNav2Goal");
