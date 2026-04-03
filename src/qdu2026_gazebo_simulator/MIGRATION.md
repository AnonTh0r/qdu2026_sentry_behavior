# 仿真包迁移指南

## 已完成的工作

✅ 创建了 `qdu2026_gazebo_simulator` 包基本结构
✅ 复制了配置文件、资源文件、rviz配置
✅ 修改了所有 launch 文件中的包名
✅ 将 `rmoss_interfaces` 替换为 `referee_interfaces`
✅ 复制并修改了 referee_system 脚本

## 关键修改点总结

### 消息接口映射
| 原接口 (rmoss_interfaces) | 新接口 (referee_interfaces/referee_simulation) |
|---------------------------|-----------------------------------------------|
| GimbalCmd | referee_interfaces::msg::GimbalCmd |
| Gimbal | referee_simulation::msg::Gimbal |
| ChassisCmd | 需要在 referee_interfaces 中定义 |
| RobotStatus | referee_simulation::msg::RobotStatus |

### 需要创建的组件

1. **robot_base 节点** (替代 rmoss_gz_base)
   - 订阅: `cmd_vel`, `robot_base/chassis_cmd`, `robot_base/gimbal_cmd`
   - 发布: `robot_base/gimbal_state`
   - 功能: 连接 ROS topic 和 Gazebo actuators

2. **robot_description 包**
   - 提供机器人 URDF/SDF 模型
   - xmacro 模板文件

3. **Gazebo 插件** (如果需要自定义控制)
   - 底盘控制器
   - 云台控制器

## 下一步工作

### 立即需要:
```bash
# 1. 检查 referee_interfaces 是否包含所有需要的消息
cd /home/yelle233/qdu2026_behavior_rmul/src/referee_interfaces
ls msg/

# 2. 如果缺少 ChassisCmd.msg,需要添加
```

### spawn_robots.launch.py 需要完善:
- 当前是占位符,需要根据实际的 robot_description 包完善
- 参考原始文件的 robot spawning 逻辑

### 配置文件可能需要调整:
- `config/base_params.yaml` - 机器人参数
- `config/gz_world.yaml` - 世界和机器人配置
- `config/ros_gz_bridge.yaml` - ROS-Gazebo 桥接配置
