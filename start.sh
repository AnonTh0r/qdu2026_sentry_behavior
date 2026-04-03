#!/bin/bash

WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# source ROS2 和工作空间
source /opt/ros/humble/setup.bash
source "$WORKSPACE_DIR/install/setup.bash"

# 启动 Groot2
/home/alis/Groot2/bin/groot2 &

# 启动 behavior launch
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py
