#!/bin/bash

# 测试脚本：验证 referee_simulation 和 behavior 的话题对齐

echo "=== 检查 referee_simulation 发布的话题 ==="
echo "预期发布的话题："
echo "  /referee/game_status"
echo "  /referee/all_robot_hp"
echo "  /referee/event_data"
echo "  /referee/robot_status"
echo "  /referee/buff"
echo "  /referee/set_pose"
echo "  /referee/ground_robot_position"
echo "  /referee/rfid_status"
echo "  /referee/operating_mode"
echo ""

echo "=== 启动 referee_simulation 节点 ==="
echo "运行命令: ros2 launch referee_simulation referee_simulation.launch.py"
echo ""
echo "或者直接运行节点:"
echo "  ros2 run referee_simulation referee_simulation_node"
echo ""

echo "=== 测试话题是否发布 ==="
echo "在另一个终端运行以下命令查看话题列表:"
echo "  ros2 topic list | grep referee"
echo ""

echo "=== 查看特定话题的消息 ==="
echo "  ros2 topic echo /referee/robot_status"
echo "  ros2 topic echo /referee/game_status"
echo ""

echo "=== 手动设置 HP 进行测试 ==="
echo "  ros2 topic pub /referee_simulation/set_hp std_msgs/msg/Int32 '{data: 100}' --once"
echo ""

echo "=== 检查话题连接 ==="
echo "  ros2 topic info /referee/robot_status"
echo ""
