#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from nav2_msgs.action import NavigateToPose
import time

class FakeNav2Server(Node):
    def __init__(self):
        super().__init__('fake_nav2_server')
        self._action_server = ActionServer(
            self,
            NavigateToPose,
            'navigate_to_pose',
            self.execute_callback)
        self.get_logger().info('Fake Nav2 action server started')

    def execute_callback(self, goal_handle):
        x = goal_handle.request.pose.pose.position.x
        y = goal_handle.request.pose.pose.position.y
        self.get_logger().info(f'Navigating to ({x:.2f}, {y:.2f})...')

        # 模拟导航耗时3秒
        for i in range(30):
            if goal_handle.is_cancel_requested:
                goal_handle.canceled()
                self.get_logger().info('Goal canceled')
                return NavigateToPose.Result()
            feedback = NavigateToPose.Feedback()
            feedback.distance_remaining = 3.0 * (1.0 - i / 30.0)
            goal_handle.publish_feedback(feedback)
            time.sleep(0.1)

        goal_handle.succeed()
        self.get_logger().info(f'Reached ({x:.2f}, {y:.2f})')
        return NavigateToPose.Result()

def main():
    rclpy.init()
    node = FakeNav2Server()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()
