**First: Prepare terminal** 
```bash
distrobox enter noble_robotics -- bash -c "source /opt/ros/jazzy/setup.bash && cd ~/hexadrone_ws && exec bash"
```

**Launch Webots in ROS2:**
```bash
ros2 launch hexadrone_webots launch.py
```

**Rebuild and launch Webots in ROS2:**
```bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
ros2 launch hexadrone_webots launch.py
```

**Launch Webots in ROS2 with full rebuild:**
```bash
source /opt/ros/jazzy/setup.bash
rm -rf build/ install/ log/ && colcon build --symlink-install
source install/setup.bash
ros2 launch hexadrone_webots launch.py
```

**"Clean" Webots launch:**
```bash
webots ~/hexadrone_ws/src/hexadrone_webots/worlds/hexaworld.wbt
```

**Standard posture:**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0]}"
```

**Arming/Disarming posture**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.00, -0.70,  0.45,   # LM
  0.00,  0.70, -0.45,   # RM
  0.00, -0.70,  0.45,   # RF
  0.00,  0.70, -0.45,   # LF
  0.00, -0.70,  0.45,   # RB
  0.00,  0.70, -0.45    # LB
]}"
```

**Max Crouch**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.00, -1.57, -1.57,   # LM
  0.00,  1.57,  1.57,   # RM
  0.00, -1.57, -1.57,   # RF
  0.00,  1.57,  1.57,   # LF
  0.00, -1.57, -1.57,   # RB
  0.00,  1.57,  1.57    # LB
]}"
```

**High Stance**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.00,  0.70,  0.60,   # LM
  0.00, -0.70, -0.60,   # RM
  0.00,  0.70,  0.60,   # RF
  0.00, -0.70, -0.60,   # LF
  0.00,  0.70,  0.60,   # RB
  0.00, -0.70, -0.60    # LB
]}"
```

**High Twist**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.26,  0.70,  0.60,   # LM
  0.26, -0.70, -0.60,   # RM
  0.26,  0.70,  0.60,   # RF
  0.26, -0.70, -0.60,   # LF
  0.26,  0.70,  0.60,   # RB
  0.26, -0.70, -0.60    # LB
]}"
```

**Walking High (Alpha)**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.26,  0.70,  0.60,   # LM
  0.00, -0.45, -0.60,   # RM
  0.26,  0.70,  0.60,   # RF
  0.00, -0.45, -0.60,   # LF
  0.26,  0.70,  0.60,   # RB
  0.00, -0.45, -0.60    # LB
]}"
```

**Walking High (Beta)**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.00,  0.45,  0.60,   # LM
  0.26, -0.70, -0.60,   # RM
  0.00,  0.45,  0.60,   # RF
  0.26, -0.70, -0.60,   # LF
  0.00,  0.45,  0.60,   # RB
  0.26, -0.70, -0.60    # LB
]}"
```
