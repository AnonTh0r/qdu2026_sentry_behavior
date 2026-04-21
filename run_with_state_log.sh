#!/bin/bash

# 只保存行为树状态变化日志
LOG_DIR="$HOME/code/qdu2026_behavior_rmul/logs"
mkdir -p "$LOG_DIR"

LOG_FILE="$LOG_DIR/bt_states_$(date +%Y%m%d_%H%M%S).log"

echo "=========================================="
echo "Behavior Tree State Logger"
echo "=========================================="
echo "Log file: $LOG_FILE"
echo "Only saving state transitions (IDLE/RUNNING/SUCCESS/FAILURE)"
echo "Press Ctrl+C to stop"
echo ""

# 启动并只保存状态变化
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py 2>&1 | \
    grep --line-buffered -E "IDLE|RUNNING|SUCCESS|FAILURE" | \
    tee "$LOG_FILE"

echo ""
echo "=========================================="
echo "Log saved to: $LOG_FILE"
echo "=========================================="
