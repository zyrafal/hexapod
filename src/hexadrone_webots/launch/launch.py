import os
import launch
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from webots_ros2_driver.webots_launcher import WebotsLauncher, Ros2SupervisorLauncher
from webots_ros2_driver.webots_controller import WebotsController

def generate_launch_description():
    description_dir = get_package_share_directory('hexadrone_description')
    webots_dir = get_package_share_directory('hexadrone_webots')
    
    urdf_path = os.path.join(description_dir, 'urdf', 'robot.urdf')
    world_path = os.path.join(webots_dir, 'worlds', 'hexaworld.wbt')
    ros2_control_params = os.path.join(description_dir, 'config', 'ros2_control.yaml')

    webots = WebotsLauncher(world=world_path)

    ros2_supervisor = Ros2SupervisorLauncher()

    robot_driver = WebotsController(
        robot_name='hexadrone_description',
        parameters=[
            {'robot_description': urdf_path},
            ros2_control_params
        ]
    )

    with open(urdf_path, 'r') as infp:
        robot_desc = infp.read()
        
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_desc, 'use_sim_time': True}]
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
    )

    position_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["forward_position_controller", "--controller-manager", "/controller_manager"],
    )

    return LaunchDescription([
        webots,
        ros2_supervisor,
        robot_driver,
        robot_state_publisher,
        joint_state_broadcaster_spawner,
        position_controller_spawner,
        launch.actions.RegisterEventHandler(
            event_handler=launch.event_handlers.OnProcessExit(
                target_action=webots,
                on_exit=[launch.actions.EmitEvent(event=launch.events.Shutdown())],
            )
        )
    ])
