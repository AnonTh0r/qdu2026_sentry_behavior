启动裁判系统模拟：
# 终端 1：启动导航 + 行为树

# 终端 2：在同一个工作空间环境下，给 referee_simulation 起 ns
ros2 run referee_simulation referee_simulation_node \
  --ros-args -r __ns:=/red_standard_robot1
# 动态修改参数也要加命名空间
ros2 topic pub /red_standard_robot1/referee_simulation/set_hp \
  std_msgs/msg/Int32 "{data: 500}" --once
