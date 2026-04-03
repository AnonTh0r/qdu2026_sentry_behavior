# QDU2026 Gazebo Simulator

从 rmu_gazebo_simulator 移植的仿真包,用于 QDU2026 哨兵机器人项目。

## 主要修改

### 1. 包名修改
- `rmu_gazebo_simulator` → `qdu2026_gazebo_simulator`

### 2. 消息接口对齐
- `rmoss_interfaces` → `referee_interfaces`
- 使用 `referee_simulation` 包中的消息定义

### 3. 关键消息类型映射
```
rmoss_interfaces::msg::GimbalCmd  → referee_interfaces::msg::GimbalCmd
rmoss_interfaces::msg::Gimbal     → referee_simulation::msg::Gimbal
rmoss_interfaces::msg::RobotStatus → referee_simulation::msg::RobotStatus
```

### 4. 依赖包调整
移除的依赖:
- rmoss_core
- rmoss_gz_base
- rmoss_gz_cam
- rmoss_gz_plugins
- rmoss_gz_resources
- rmoss_gz_bridge
- pb2025_robot_description

保留的依赖:
- ros_gz_sim
- ros_gz_bridge
- referee_interfaces
- referee_simulation

## 待完成工作

### 必须完成:
1. **创建 robot_base 节点** - 替代 rmoss_gz_base 的功能
2. **创建 robot_description 包** - 提供机器人模型
3. **实现 Gazebo 插件** - 底盘/云台控制器

### 可选:
- 复制 player_web 和 referee_web 脚本(如需要)
- 调整配置文件中的参数

## 使用方法

```bash
# 编译
cd /home/yelle233/qdu2026_behavior_rmul
colcon build --packages-select qdu2026_gazebo_simulator

# 启动仿真
ros2 launch qdu2026_gazebo_simulator bringup_sim.launch.py
```

## 文件结构
```
qdu2026_gazebo_simulator/
├── launch/
│   ├── bringup_sim.launch.py
│   ├── gazebo.launch.py
│   ├── spawn_robots.launch.py
│   ├── referee_system.launch.py
│   └── rviz.launch.py
├── config/
│   ├── gz_world.yaml
│   ├── base_params.yaml
│   ├── referee_system_1v1.yaml
│   └── ros_gz_bridge.yaml
├── scripts/
│   └── referee_system/
│       ├── simple_competition_1v1.py
│       └── referee_system_client.py
├── resource/
│   ├── worlds/
│   ├── ign/
│   └── models/
└── rviz/
```
