#!/bin/bash

echo "=== 诊断 Behavior 和 Simulation 通信问题 ==="
echo ""

echo "1. 检查话题列表"
echo "---"
ros2 topic list | grep referee
echo ""

echo "2. 检查 /referee/game_status 的发布者和订阅者"
echo "---"
ros2 topic info /referee/game_status
echo ""

echo "3. 检查 /referee/robot_status 的发布者和订阅者"
echo "---"
ros2 topic info /referee/robot_status
echo ""

echo "4. 查看 game_status 最新消息（1条）"
echo "---"
timeout 2 ros2 topic echo /referee/game_status --once
echo ""

echo "5. 查看 robot_status 最新消息（1条）"
echo "---"
timeout 2 ros2 topic echo /referee/robot_status --once
echo ""

echo "6. 检查话题发布频率"
echo "---"
echo "game_status 频率:"
timeout 5 ros2 topic hz /referee/game_status
echo ""
echo "robot_status 频率:"
timeout 5 ros2 topic hz /referee/robot_status
echo ""

echo "=== 诊断完成 ==="
