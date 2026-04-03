#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Bool
from referee_interfaces.msg import GimbalCmd
import ignition.transport as ign_transport


class SimpleRobotBase(Node):
    def __init__(self):
        super().__init__('simple_robot_base')

        self.declare_parameter('robot_name', 'red_standard_robot1')
        self.robot_name = self.get_parameter('robot_name').value

        # Ignition transport
        self.gz_node = ign_transport.Node()

        # Gazebo topics
        gz_chassis_topic = f"/{self.robot_name}/cmd_vel"
        gz_pitch_topic = f"/model/{self.robot_name}/joint/gimbal_pitch_joint/cmd_vel"
        gz_yaw_topic = f"/model/{self.robot_name}/joint/gimbal_yaw_joint/cmd_vel"

        # Publishers to Gazebo
        self.chassis_pub = self.gz_node.advertise(gz_chassis_topic, "ignition.msgs.Twist")
        self.pitch_pub = self.gz_node.advertise(gz_pitch_topic, "ignition.msgs.Double")
        self.yaw_pub = self.gz_node.advertise(gz_yaw_topic, "ignition.msgs.Double")

        # ROS subscribers
        self.cmd_vel_sub = self.create_subscription(Twist, 'cmd_vel', self.cmd_vel_callback, 10)
        self.gimbal_sub = self.create_subscription(GimbalCmd, 'robot_base/gimbal_cmd', self.gimbal_callback, 10)

        # Referee system subscribers
        enable_power_topic = f"/referee_system/{self.robot_name}/enable_power"
        self.enable_power_sub = self.create_subscription(Bool, enable_power_topic, self.enable_power_callback, 10)

        self.enabled = True
        self.get_logger().info(f'Simple robot base started for {self.robot_name}')

    def cmd_vel_callback(self, msg):
        if not self.enabled:
            return
        from ignition.msgs import Twist as GzTwist, Vector3d
        gz_msg = GzTwist()
        gz_msg.linear = Vector3d(x=msg.linear.x, y=msg.linear.y, z=msg.linear.z)
        gz_msg.angular = Vector3d(x=msg.angular.x, y=msg.angular.y, z=msg.angular.z)
        self.chassis_pub.publish(gz_msg)

    def gimbal_callback(self, msg):
        if not self.enabled:
            return
        from ignition.msgs import Double
        if msg.pitch_type == GimbalCmd.VELOCITY:
            pitch_msg = Double(data=msg.velocity.pitch)
            self.pitch_pub.publish(pitch_msg)
        if msg.yaw_type == GimbalCmd.VELOCITY:
            yaw_msg = Double(data=msg.velocity.yaw)
            self.yaw_pub.publish(yaw_msg)

    def enable_power_callback(self, msg):
        self.enabled = msg.data


def main(args=None):
    rclpy.init(args=args)
    node = SimpleRobotBase()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
