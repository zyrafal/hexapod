hexadrone_ws/src/
│
├── 🧠 hexadrone_core/             <-- THE BRAIN: Pure C++ (Portable between PC & ESP32)
│   ├── include/
│   │   └── hexadrone_core/
│   │       ├── lib.hpp           // Math constants, 3D Vector structs, and float mapping
│   │       ├── state_machine.hpp // Shared Enums: ServoState {ARMED, DISARMED ...}
│   │       ├── kinematics.hpp    // FK solver: Calculates joint angles
│   │       ├── gait_engine.hpp   // Walking patterns: Elliptical paths and leg timing logic
│   │       └── brain.hpp         // Master coordinator: Processes normalized inputs to drive FK
│   ├── src/
│   │   ├── kinematics.cpp        // Implementation of Forward Kinematics
│   │   ├── gait_engine.cpp       // Implementation of Swing/Stance phases
│   │   └── brain.cpp             // High-level decision making
│   ├── CMakeLists.txt            // Standard ament_cmake (Exported for ROS2, ignored by PIO)
│   └── package.xml               // Minimal dependencies (Header-only or static library)
│
├── 🎮 hexadrone_controller/       <-- THE SIMULATOR INTERFACE: ROS2 Wrapper
│   ├── include/
│   │   └── hexadrone_controller/
│   │       └── bridge_ops.hpp    // Translates ROS2 sensor_msgs/Joy to normalized Core data
│   ├── src/
│   │   └── teleop_node.cpp       // ROS2 Node: Subscribes to /joy, updates Brain, sends to Webots
│   ├── CMakeLists.txt            // Links rclcpp, sensor_msgs, and hexadrone_core
│   └── package.xml
│
├── 🤖 hexadrone_description/      <-- THE BLUEPRINT: Robot URDF & Visual Assets
│   ├── assets/                   // .stl mesh files for visualization
│   ├── config/                   // ros2_control.yaml for joint controllers
│   ├── urdf/                     // robot.urdf (The source of truth for leg segment lengths)
│   ├── CMakeLists.txt
│   └── package.xml
│
├── 🌍 hexadrone_webots/           <-- THE ENVIRONMENT: Simulation Worlds & Config
│   ├── launch/                   // ROS2 launch.py to start simulation and controller nodes
│   ├── protos/                   // Webots robot.proto (Generated from URDF)
│   ├── worlds/                   // hexaworld.wbt (The virtual testing environment)
│   ├── CMakeLists.txt
│   └── package.xml
│
└── ⚡️ hexadrone_esp32/            <-- THE PHYSICAL HARDWARE: PlatformIO Layer
    ├── platformio.ini            // Pinned versions and lib_extra_dirs pointing to ../core
    ├── include/
    │   └── config.h              // Hardware constants: Pins (SDA, SCL, OE) and I2C Addresses
    ├── lib/
    │   ├── hexadrone_core/       // Symlink to ../../hexadrone_core/
    │   ├── comms_manager/        // WiFi, OTA Updates, and Async Web Server
    │   ├── radio_manager/        // CRSF Decoder: Normalizes RadioMaster sticks to -1.0..1.0
    │   ├── servo_manager/        // PCA9685 Interface: Sends raw PWM signals
    │   └── power_manager/        // INA228 Battery Monitor and LittleFS Blackbox Logging
    └── src/
        └── main.cpp              // The Conductor: Feeds Radio -> Brain -> Servos
