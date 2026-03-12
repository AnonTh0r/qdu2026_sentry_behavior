from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='referee_simulation',
            executable='referee_simulation_node',
            name='referee_simulation',
            output='screen',
            parameters=[],
        ),
    ])
