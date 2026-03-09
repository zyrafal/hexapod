**First: Enter Distrobox**
```bash
distrobox enter noble_robotics
```

**Launch Webots in ROS2:**
```bash
ros2 launch hexadrone_webots launch.py
```

**"Clean" launch:**
```bash
webots ~/hexadrone_ws/src/hexadrone_webots/worlds/hexaworld.wbt
```

**Rebuild and launch Webots in ROS2:**
```bash
source /opt/ros/jazzy/setup.bash
cd ~/hexadrone_ws
colcon build --symlink-install
source install/setup.bash
ros2 launch hexadrone_webots launch.py
```

**Launch Webots in ROS2 with "clean" build:**
```bash
source /opt/ros/jazzy/setup.bash
cd ~/hexadrone_ws
rm -rf build/ install/ log/
colcon build --symlink-install
source install/setup.bash
ros2 launch hexadrone_webots launch.py
```

**Prep before manually entering servo angles:**
```bash
source /opt/ros/jazzy/setup.bash
cd ~/hexadrone_ws
source install/setup.bash
```

**Reset servo angles:**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0]}"
```

**Stand Up**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.0,  0.7,  0.5,   # LF
  0.0, -0.7, -0.5,   # RF
  0.0,  0.7,  0.5,   # LM
  0.0, -0.7, -0.5,   # RM
  0.0,  0.7,  0.5,   # LB
  0.0, -0.7, -0.5    # RB
]}"
```

**Stable Ready**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.3,  0.4,  0.3,   # LF
 -0.3, -0.4, -0.3,   # RF
  0.0,  0.4,  0.3,   # LM
  0.0, -0.4, -0.3,   # RM
 -0.3,  0.4,  0.3,   # LB
  0.3, -0.4, -0.3    # RB
]}"
```

**Stable Twist**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.5,  0.4,  0.4,   # LF
  0.5, -0.4, -0.4,   # RF
  0.5,  0.4,  0.4,   # LM
  0.5, -0.4, -0.4,   # RM
  0.5,  0.4,  0.4,   # LB
  0.5, -0.4, -0.4    # RB
]}"
```

**Max Crouch**
```bash
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [
  0.0, -1.4, -1.4,   # LF
  0.0,  1.4,  1.4,   # RF
  0.0, -1.4, -1.4,   # LM
  0.0,  1.4,  1.4,   # RM
  0.0, -1.4, -1.4,   # LB
  0.0,  1.4,  1.4    # RB
]}"
```
