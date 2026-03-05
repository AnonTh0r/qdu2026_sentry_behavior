cmake_minimum_required(VERSION 3.8)
project(referee_simulation)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(referee_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)  # RobotStatus 里用到 Pose 的话需要
find_package(std_msgs REQUIRED)


add_executable(referee_simulation_node
  src/referee_simulation_node.cpp
)

ament_target_dependencies(referee_simulation_node
  rclcpp
  referee_interfaces
  geometry_msgs
  std_msgs
)

install(TARGETS
  referee_simulation_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
