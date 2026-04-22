# 🤖 Hexadrone

A versatile, ROS 2 and ESP32 compatible software stack for a 18-DOF hexapod robot (drone). This repository contains the unified C++ brain, kinematics engine, Webots simulation environment, and the physical PlatformIO hardware layer.

📖 **For full hardware specifications, control mappings, architecture details, and system logic, please read the [Documentation](DOCUMENTATION.md).**

---

## 💻 Simulation Command Reference

Execute these from your host terminal to manage the containerized ROS 2 Jazzy environment using **Makefile**.

| Command           | Action                                                                 |
|-------------------|------------------------------------------------------------------------|
| **`make up`**     | **Start** the Podman container in the background.                      |
| **`make build`**  | **Compile** C++ logic for `hexadrone_core` and `hexadrone_controller`. |
| **`make run`**    | **Launch** Webots and ROS 2 nodes. Press **Ctrl+C** to exit.           |
| **`make deploy`** | **Upload** firmware directly to the physical ESP32 hardware.           |
| **`make stop`**   | **Kill** all container processes and stop the environment.             |
| **`make shell`**  | **Enter** the interactive **fish** shell inside the container.         |

---

## 🛠️ Debugging & Diagnostics (Shell Tab)
Use these commands inside `make shell` while the simulation is running to inspect the drone's state.

| Command                                                 | Purpose                                                             |
|---------------------------------------------------------|---------------------------------------------------------------------|
| `ros2 topic echo /joy`                                  | Monitors raw input from the RadioMaster controller.                 |
| `ros2 topic echo /forward_position_controller/commands` | Monitors the 18 servo angles being sent to Webots.                  |
| `ros2 topic hz /joy`                                    | Verifies that the teleop loop is running at the expected frequency. |
| `ros2 node info /teleop_node`                           | Shows which topics the Brain is currently reading and writing.      |
| `ros2 param list`                                       | Lists all active configuration parameters for the servos.           |
| `ros2 doctor`                                           | Runs a global diagnostic on the ROS 2 container environment.        |
