cmake_minimum_required(VERSION 3.8)
project(referee_simulation)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(referee_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)  # RobotStatus 里用到 Pose 的话需要
find_package(std_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav_msgs REQUIRED)


add_executable(referee_simulation_node
  src/referee_simulation_node.cpp
)

ament_target_dependencies(referee_simulation_node
  rclcpp
  referee_interfaces
  geometry_msgs
  std_msgs
)

add_executable(mock_navigate_through_poses
  src/mock_navigate_through_poses.cpp
)

ament_target_dependencies(mock_navigate_through_poses
  rclcpp
  rclcpp_action
  nav2_msgs
)

add_executable(mock_odom_node
  src/mock_odom_node.cpp
)

ament_target_dependencies(mock_odom_node
  rclcpp
  geometry_msgs
  nav_msgs
)

install(TARGETS
  referee_simulation_node
  mock_navigate_through_poses
  mock_odom_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
