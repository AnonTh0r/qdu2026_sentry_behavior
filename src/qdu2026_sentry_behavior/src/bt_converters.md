#include <behaviortree_cpp_v3/behavior_tree.h>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <string>

namespace BT
{

// 注意：这里必须用 BT::StringView，保持模板签名一致
template <>
inline geometry_msgs::msg::PoseStamped convertFromString(BT::StringView str)
{
  // 支持分号或逗号
  auto parts = BT::splitString(str, ';');
  if (parts.size() != 3)
  {
    parts = BT::splitString(str, ',');
    if (parts.size() != 3)
    {
      throw BT::RuntimeError("Invalid input for PoseStamped: expected format x;y;yaw or x,y,yaw, got ", str);
    }
  }

  geometry_msgs::msg::PoseStamped pose;
  pose.header.frame_id = "map";

  // BT::StringView 不是 std::string，要先转换
  pose.pose.position.x = std::stod(std::string(parts[0]));
  pose.pose.position.y = std::stod(std::string(parts[1]));
  double yaw = std::stod(std::string(parts[2]));

  tf2::Quaternion q;
  q.setRPY(0.0, 0.0, yaw);
  pose.pose.orientation = tf2::toMsg(q);

  return pose;
}

}  // namespace BT
