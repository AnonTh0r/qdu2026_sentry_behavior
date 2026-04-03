#!/bin/bash

WORKSPACE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SESSION="behavior_session"

# 如果 session 已存在则先删除
tmux kill-session -t "$SESSION" 2>/dev/null

# 窗口1：referee_simulation
tmux new-session -d -s "$SESSION" -n "referee" \
  "source /opt/ros/humble/setup.bash && source '$WORKSPACE_DIR/install/setup.bash' && ros2 launch referee_simulation referee_simulation.launch.py; exec bash"

# 窗口2：fake nav2
tmux new-window -t "$SESSION" -n "nav2" \
  "source /opt/ros/humble/setup.bash && python3 '$WORKSPACE_DIR/fake_nav2_server.py'; exec bash"

# 窗口3：behavior
tmux new-window -t "$SESSION" -n "behavior" \
  "source /opt/ros/humble/setup.bash && source '$WORKSPACE_DIR/install/setup.bash' && ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py; exec bash"

tmux attach-session -t "$SESSION"
