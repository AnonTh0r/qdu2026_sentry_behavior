# cmake_minimum_required(VERSION 3.16)
# project(qdu2026_sentry_behavior)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ## By adding -Wall and -Werror, the compiler does not ignore warnings anymore,
# ## enforcing cleaner code.
# add_definitions(-Wall -Werror)

# ## Export compile commands for clangd
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_definitions(-DROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

# #######################
# ## Find dependencies ##
# #######################
# find_package(tf2 REQUIRED)
# find_package(tf2_geometry_msgs REQUIRED)

# find_package(ament_cmake_auto REQUIRED)
# ament_auto_find_build_dependencies()

# ###########
# ## Build ##
# ###########

# # ament_auto_add_executable(qdu2026_sentry_behavior_server 
# #   src/qdu2026_sentry_behavior_server.cpp
# #   # src/bt_converters.cpp# <- 把转换器放到可执行里
# #   )

# # ament_auto_add_library(action_client SHARED
# #   src/qdu2026_sentry_behavior_client.cpp
# #   # src/bt_converters.cpp#改动
# # )

# target_link_libraries(qdu2026_sentry_behavior_server
#   # ... 你的其他库
#   ${tf2_LIBRARIES}
# )

# install(DIRECTORY include/ DESTINATION include)

# rclcpp_components_register_node(action_client
#   PLUGIN qdu2026_sentry_behavior::SentryBehaviorClient
#   EXECUTABLE qdu2026_sentry_behavior_client
# )

# ####### Conditions #######
# ament_auto_add_library(is_game_status SHARED plugins/condition/is_game_status.cpp)
# list(APPEND plugin_libs is_game_status)

# ament_auto_add_library(is_rfid_detected SHARED plugins/condition/is_rfid_detected.cpp)
# list(APPEND plugin_libs is_rfid_detected)

# ament_auto_add_library(is_status_ok SHARED plugins/condition/is_status_ok.cpp)
# list(APPEND plugin_libs is_status_ok)

# ament_auto_add_library(is_attacked SHARED plugins/condition/is_attacked.cpp)
# list(APPEND plugin_libs is_attacked)

# ament_auto_add_library(is_detect_enemy SHARED plugins/condition/is_detect_enemy.cpp)
# list(APPEND plugin_libs is_detect_enemy)

# ####### Actions #######
# ament_auto_add_library(send_nav2_goal SHARED plugins/action/send_nav2_goal.cpp)
# list(APPEND plugin_libs send_nav2_goal)

# ament_auto_add_library(pub_nav2_goal SHARED 
#     plugins/action/pub_nav2_goal.cpp
#     # src/bt_converters.cpp##  新增这行
#        )
# list(APPEND plugin_libs pub_nav2_goal)

# ament_auto_add_library(pub_spin_speed SHARED plugins/action/pub_spin_speed.cpp)
# list(APPEND plugin_libs pub_spin_speed)

# ament_auto_add_library(pub_twist SHARED plugins/action/pub_twist.cpp)
# list(APPEND plugin_libs pub_twist)

# ament_auto_add_library(pub_gimbal_absolute SHARED plugins/action/pub_gimbal_absolute.cpp)
# list(APPEND plugin_libs pub_gimbal_absolute)

# ament_auto_add_library(pub_gimbal_velocity SHARED plugins/action/pub_gimbal_velocity.cpp)
# list(APPEND plugin_libs pub_gimbal_velocity)

# ament_auto_add_library(calculate_attack_pose SHARED plugins/action/calculate_attack_pose.cpp)
# list(APPEND plugin_libs calculate_attack_pose)

# ####### Controls #######
# ament_auto_add_library(recovery_node SHARED plugins/control/recovery_node.cpp)
# list(APPEND plugin_libs recovery_node)

# ####### Decorators #######
# ament_auto_add_library(rate_controller SHARED plugins/decorator/rate_controller.cpp)
# list(APPEND plugin_libs rate_controller)

# ament_auto_add_library(tick_after_timeout_node SHARED plugins/decorator/tick_after_timeout_node.cpp)
# list(APPEND plugin_libs tick_after_timeout_node)

# foreach(bt_plugin ${plugin_libs})
#   target_compile_definitions(${bt_plugin} PRIVATE BT_PLUGIN_EXPORT)
# endforeach()

# #############
# ## Testing ##
# #############

# if(BUILD_TESTING)
#   set(ament_cmake_clang_format_CONFIG_FILE "${CMAKE_SOURCE_DIR}/.clang-format")
#   find_package(ament_lint_auto REQUIRED)
#   list(APPEND AMENT_LINT_AUTO_EXCLUDE
#     ament_cmake_uncrustify
#     ament_cmake_flake8
#   )
#   ament_lint_auto_find_test_dependencies()
# endif()

# #############
# ## Install ##
# #############

# install(
#   TARGETS ${plugin_libs}
#   LIBRARY DESTINATION share/${PROJECT_NAME}/bt_plugins
#   ARCHIVE DESTINATION share/${PROJECT_NAME}/bt_plugins
#   RUNTIME DESTINATION share/${PROJECT_NAME}/bt_plugins
# )

# ament_auto_package(
#   INSTALL_TO_SHARE
#   behavior_trees
#   params
#   launch
# )
cmake_minimum_required(VERSION 3.16)
project(qdu2026_sentry_behavior)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## By adding -Wall and -Werror, the compiler does not ignore warnings anymore,
## enforcing cleaner code.
add_definitions(-Wall -Werror)

## Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

#######################
## Find dependencies ##
#######################
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

###########
## Build ##
###########

# ========== 修改这部分：取消注释并添加依赖 ==========
ament_auto_add_executable(qdu2026_sentry_behavior_server 
  src/qdu2026_sentry_behavior_server.cpp
)

# 为 server 添加 tf2 依赖
target_link_libraries(qdu2026_sentry_behavior_server
  ${tf2_LIBRARIES}
  ${tf2_geometry_msgs_LIBRARIES}
)

ament_auto_add_library(action_client SHARED
  src/qdu2026_sentry_behavior_client.cpp
)
# ========== 修改结束 ==========

install(DIRECTORY include/ DESTINATION include)

rclcpp_components_register_node(action_client
  PLUGIN qdu2026_sentry_behavior::SentryBehaviorClient
  EXECUTABLE qdu2026_sentry_behavior_client
)

####### Conditions #######
ament_auto_add_library(is_game_status SHARED plugins/condition/is_game_status.cpp)
list(APPEND plugin_libs is_game_status)

ament_auto_add_library(is_rfid_detected SHARED plugins/condition/is_rfid_detected.cpp)
list(APPEND plugin_libs is_rfid_detected)

ament_auto_add_library(is_status_ok SHARED plugins/condition/is_status_ok.cpp)
list(APPEND plugin_libs is_status_ok)

ament_auto_add_library(is_attacked SHARED plugins/condition/is_attacked.cpp)
list(APPEND plugin_libs is_attacked)

ament_auto_add_library(is_detect_enemy SHARED plugins/condition/is_detect_enemy.cpp)
list(APPEND plugin_libs is_detect_enemy)

ament_auto_add_library(is_hp_dropping SHARED plugins/condition/is_hp_dropping.cpp)
list(APPEND plugin_libs is_hp_dropping)

ament_auto_add_library(is_hp_low SHARED plugins/condition/is_hp_low.cpp)
list(APPEND plugin_libs is_hp_low)

ament_auto_add_library(is_zone_active SHARED plugins/condition/is_zone_active.cpp)
list(APPEND plugin_libs is_zone_active)

####### Actions #######
ament_auto_add_library(send_nav2_goal SHARED plugins/action/send_nav2_goal.cpp)
list(APPEND plugin_libs send_nav2_goal)

ament_auto_add_library(follow_waypoints_action SHARED plugins/action/follow_waypoints_action.cpp)
list(APPEND plugin_libs follow_waypoints_action)

ament_auto_add_library(navigate_through_poses_action SHARED plugins/action/navigate_through_poses_action.cpp)
list(APPEND plugin_libs navigate_through_poses_action)

# ========== 重要：为 pub_nav2_goal 添加 tf2 依赖 ==========
ament_auto_add_library(pub_nav2_goal SHARED 
    plugins/action/pub_nav2_goal.cpp
)
# 添加 tf2 依赖到这个插件
target_link_libraries(pub_nav2_goal
  ${tf2_LIBRARIES}
  ${tf2_geometry_msgs_LIBRARIES}
)
list(APPEND plugin_libs pub_nav2_goal)
# ========== 修改结束 ==========

ament_auto_add_library(pub_nav2_goal_pose SHARED plugins/action/pub_nav2_goal_pose.cpp)
list(APPEND plugin_libs pub_nav2_goal_pose)

ament_auto_add_library(pub_spin_speed SHARED plugins/action/pub_spin_speed.cpp)
list(APPEND plugin_libs pub_spin_speed)

ament_auto_add_library(pub_twist SHARED plugins/action/pub_twist.cpp)
list(APPEND plugin_libs pub_twist)

ament_auto_add_library(pub_gimbal_absolute SHARED plugins/action/pub_gimbal_absolute.cpp)
list(APPEND plugin_libs pub_gimbal_absolute)

ament_auto_add_library(pub_gimbal_velocity SHARED plugins/action/pub_gimbal_velocity.cpp)
list(APPEND plugin_libs pub_gimbal_velocity)

ament_auto_add_library(calculate_attack_pose SHARED plugins/action/calculate_attack_pose.cpp)
list(APPEND plugin_libs calculate_attack_pose)

ament_auto_add_library(change_pose SHARED plugins/action/change_pose.cpp)
list(APPEND plugin_libs change_pose)

####### Controls #######
ament_auto_add_library(recovery_node SHARED plugins/control/recovery_node.cpp)
list(APPEND plugin_libs recovery_node)

####### Decorators #######
ament_auto_add_library(rate_controller SHARED plugins/decorator/rate_controller.cpp)
list(APPEND plugin_libs rate_controller)

ament_auto_add_library(tick_after_timeout_node SHARED plugins/decorator/tick_after_timeout_node.cpp)
list(APPEND plugin_libs tick_after_timeout_node)

foreach(bt_plugin ${plugin_libs})
  target_compile_definitions(${bt_plugin} PRIVATE BT_PLUGIN_EXPORT)
endforeach()

#############
## Testing ##
#############

if(BUILD_TESTING)
  set(ament_cmake_clang_format_CONFIG_FILE "${CMAKE_SOURCE_DIR}/.clang-format")
  find_package(ament_lint_auto REQUIRED)
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_uncrustify
    ament_cmake_flake8
  )
  ament_lint_auto_find_test_dependencies()
endif()

#############
## Install ##
#############

install(
  TARGETS ${plugin_libs}
  LIBRARY DESTINATION share/${PROJECT_NAME}/bt_plugins
  ARCHIVE DESTINATION share/${PROJECT_NAME}/bt_plugins
  RUNTIME DESTINATION share/${PROJECT_NAME}/bt_plugins
)

ament_auto_package(
  INSTALL_TO_SHARE
  behavior_trees
  params
  launch
)
