from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='referee_simulation',
            executable='referee_simulation_node',
            name='referee_simulation',
            output='screen',
        ),
        Node(
            package='referee_simulation',
            executable='mock_navigate_through_poses',
            name='mock_navigate_through_poses',
            output='screen',
            parameters=[{'delay_per_pose': 1.0}],
        ),
        Node(
            package='referee_simulation',
            executable='mock_odom_node',
            name='mock_odom_node',
            output='screen',
            parameters=[{
                'speed': 1.0,
                'update_rate': 10.0,
                'tolerance': 0.1,
            }],
        ),
    ])
