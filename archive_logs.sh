#!/bin/bash

# 归档 ROS2 日志到项目目录
LOG_DIR="$HOME/code/qdu2026_behavior_rmul/logs"
ROS_LOG_DIR="$HOME/.ros/log"

mkdir -p "$LOG_DIR"

echo "Archiving behavior tree logs..."

# 复制所有 sentry_behavior 日志
cp "$ROS_LOG_DIR"/*sentry_behavior*.log "$LOG_DIR/" 2>/dev/null

# 统计
COUNT=$(ls "$LOG_DIR"/*sentry_behavior*.log 2>/dev/null | wc -l)

echo "Archived $COUNT log files to $LOG_DIR"
echo ""
echo "Recent logs:"
ls -lht "$LOG_DIR"/*sentry_behavior*.log | head -5
