#!/bin/bash

# 创建日志目录
LOG_DIR="$HOME/code/qdu2026_behavior_rmul/logs"
mkdir -p "$LOG_DIR"

# 生成日志文件名
LOG_FILE="$LOG_DIR/bt_log_$(date +%Y%m%d_%H%M%S).log"

echo "Starting behavior tree with logging..."
echo "Log file: $LOG_FILE"
echo "Press Ctrl+C to stop"
echo ""

# 启动并保存日志
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py 2>&1 | tee "$LOG_FILE"

echo ""
echo "Log saved to: $LOG_FILE"
