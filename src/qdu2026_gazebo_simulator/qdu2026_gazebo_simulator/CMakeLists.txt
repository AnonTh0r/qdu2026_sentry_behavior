cmake_minimum_required(VERSION 3.8)
project(qdu2026_gazebo_simulator)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

install(DIRECTORY
  launch
  config
  resource
  rviz
  scripts
  DESTINATION share/${PROJECT_NAME}
)

install(PROGRAMS
  src/simple_robot_base.py
  DESTINATION lib/${PROJECT_NAME}
  RENAME simple_robot_base
)

ament_package()
