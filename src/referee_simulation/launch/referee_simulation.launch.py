from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess
import os


def generate_launch_description():
    fake_nav2_script = os.path.join(
        os.path.dirname(__file__), '../../../../..', 'fake_nav2_server.py')

    return LaunchDescription([
        Node(
            package='referee_simulation',
            executable='referee_simulation_node',
            name='referee_simulation',
            output='screen',
            parameters=[],
        ),
        ExecuteProcess(
            cmd=['python3', fake_nav2_script],
            output='screen',
        ),
    ])
