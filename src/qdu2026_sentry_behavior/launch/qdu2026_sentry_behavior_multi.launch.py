# qdu2026_sentry_behavior_multi.launch.py
#
# 一次启动一个 server + 两个 client，
# 分别执行 sentry_multi_points_nav 和 navigation_armor 两棵行为树。

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, PushRosNamespace, SetRemap
from launch_ros.descriptions import ParameterFile
from nav2_common.launch import RewrittenYaml


def generate_launch_description():
    # 包路径
    bringup_dir = get_package_share_directory("qdu2026_sentry_behavior")

    # Launch 参数
    namespace = LaunchConfiguration("namespace")
    use_sim_time = LaunchConfiguration("use_sim_time")
    params_file = LaunchConfiguration("params_file")
    log_level = LaunchConfiguration("log_level")

    # 两棵 BT 的名字（注意是 BehaviorTree 的 ID，不带 .xml 后缀）
    tree_1 = LaunchConfiguration("tree_1")
    tree_2 = LaunchConfiguration("tree_2")

    # 把 use_sim_time 写进 YAML 里
    param_substitutions = {"use_sim_time": use_sim_time}

    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=params_file,
            root_key=namespace,
            param_rewrites=param_substitutions,
            convert_types=True,
        ),
        allow_substs=True,
    )

    # 日志相关环境变量
    stdout_linebuf_envvar = SetEnvironmentVariable(
        "RCUTILS_LOGGING_BUFFERED_STREAM", "1"
    )
    colorized_output_envvar = SetEnvironmentVariable("RCUTILS_COLORIZED_OUTPUT", "1")

    # Launch 参数声明
    declare_namespace_cmd = DeclareLaunchArgument(
        "namespace",
        default_value="",
        description="Top-level namespace",
    )

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false",
        description="Use simulation (Gazebo) clock if true",
    )

    declare_params_file_cmd = DeclareLaunchArgument(
        "params_file",
        default_value=os.path.join(bringup_dir, "params", "sentry_behavior.yaml"),
        description="Full path to the ROS2 parameters file to use for all launched nodes",
    )

    declare_log_level_cmd = DeclareLaunchArgument(
        "log_level",
        default_value="info",
        description="log level",
    )

    # ⭐ 两棵行为树的 ID，默认就是你说的两个新树
    declare_tree_1_cmd = DeclareLaunchArgument(
        "tree_1",
        default_value="sentry_multi_points_nav",
        description="First Behavior Tree ID to execute",
    )

    declare_tree_2_cmd = DeclareLaunchArgument(
        "tree_2",
        default_value="navigation_armor",
        description="Second Behavior Tree ID to execute",
    )

    # 组合动作：统一 namespace + remap + 节点
    bringup_cmd_group = GroupAction(
        [
            PushRosNamespace(namespace=namespace),
            SetRemap("/tf", "tf"),
            SetRemap("/tf_static", "tf_static"),

            # === 行为树 Server（只起一个，共享给两个 client） ===
            Node(
                package="qdu2026_sentry_behavior",
                executable="qdu2026_sentry_behavior_server",
                name="qdu2026_sentry_behavior_server",
                output="screen",
                parameters=[configured_params,{"use_sim_time": True}],
                arguments=["--ros-args", "--log-level", log_level],
            ),

            # === 受击旋转（独立后台节点，行为树外） ===
            Node(
                package="qdu2026_sentry_behavior",
                executable="spin_on_attack_node",
                name="spin_on_attack_node",
                output="screen",
                parameters=[configured_params],
            ),

            # === Client 1：跑 sentry_multi_points_nav ===
            Node(
                package="qdu2026_sentry_behavior",
                executable="qdu2026_sentry_behavior_client",
                name="qdu2026_sentry_behavior_client_multi_points",
                output="screen",
                parameters=[
                    configured_params,
                    # 用 Launch 参数覆盖 YAML 里原来的 target_tree
                    {"target_tree": tree_1},
                ],
                arguments=["--ros-args", "--log-level", log_level],
            ),

            # # === Client 2：跑 navigation_armor ===
            # Node(
            #     package="qdu2026_sentry_behavior",
            #     executable="qdu2026_sentry_behavior_client",
            #     name="qdu2026_sentry_behavior_client_navigation_armor",
            #     output="screen",
            #     parameters=[
            #         configured_params,
            #         {"target_tree": tree_2},
            #     ],
            #     arguments=["--ros-args", "--log-level", log_level],
            # ),
            # Node(
            #     package="qdu2026_sentry_behavior",
            #     executable="qdu2026_sentry_behavior_server",
            #     name="qdu2026_sentry_behavior_server",
            #     parameters=[{"use_sim_time": True}],
    
            # ),

        ]
    )

    # LaunchDescription
    ld = LaunchDescription()

    # 环境变量
    ld.add_action(stdout_linebuf_envvar)
    ld.add_action(colorized_output_envvar)

    # Launch 参数
    ld.add_action(declare_namespace_cmd)
    ld.add_action(declare_use_sim_time_cmd)
    ld.add_action(declare_params_file_cmd)
    ld.add_action(declare_log_level_cmd)
    ld.add_action(declare_tree_1_cmd)
    ld.add_action(declare_tree_2_cmd)

    # 组动作
    ld.add_action(bringup_cmd_group)

    return ld
