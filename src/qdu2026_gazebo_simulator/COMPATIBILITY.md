# 导航包与仿真兼容性分析

## ✅ 可以直接使用

### 1. 底盘控制 - 完全兼容
- **行为树使用**: `cmd_vel` (geometry_msgs/Twist)
- **仿真提供**: `cmd_vel` (通过 simple_robot_base)
- **状态**: ✅ 完全对齐

### 2. 云台控制 - 完全兼容
- **行为树使用**: `robot_base/gimbal_cmd` (referee_interfaces/GimbalCmd)
- **仿真提供**: `robot_base/gimbal_cmd` (通过 simple_robot_base)
- **状态**: ✅ 完全对齐

### 3. 命名空间
- 仿真中每个机器人有独立命名空间 (如 `red_standard_robot1`)
- 启动导航时需要指定相同的命名空间

## 🚀 启动方式

```bash
# 终端1: 启动仿真
cd /home/yelle233/qdu2026_behavior_rmul
source install/setup.bash
ros2 launch qdu2026_gazebo_simulator bringup_sim.launch.py

# 终端2: 启动行为树 (需要指定命名空间)
ros2 launch qdu2026_sentry_behavior qdu2026_sentry_behavior_launch.py \
    namespace:=red_standard_robot1 \
    use_sim_time:=true
```

## ⚠️ 注意事项

1. **use_sim_time** - 必须设置为 true
2. **命名空间** - 必须与仿真中的机器人名称一致
3. **导航栈** - 如果使用 Nav2,也需要在相同命名空间下启动

## 验证方法

```bash
# 检查topic
ros2 topic list | grep red_standard_robot1

# 应该看到:
# /red_standard_robot1/cmd_vel
# /red_standard_robot1/robot_base/gimbal_cmd
```
