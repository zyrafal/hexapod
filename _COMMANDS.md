# 🤖 Simulation Command Reference

## 🚀 Environment Control
Execute these from your host terminal to manage the containerized ROS 2 Jazzy environment using **Makefile**.

| Target | Description |
|---|---|
| `make shell` | Start container, auto-build ROS 2 packages, and enter **fish** shell |
| `webots` | **(Inside shell)** Alias to launch the Webots simulation with teleop nodes |
| `make deploy` | Compile and upload firmware directly to the ESP32 hardware |
| `make stop` | Kill all container processes and stop the simulation |

---

## 🦾 Posture Aliases (Manual Debug)
Run these from a **second host terminal tab** while the simulation is active to test joint mapping and power groups. **(might be immediately reset by an active brain cycle)**

| Command | Action |
|---|---|
| `make posture-prone` | Safety position used for Arming/Disarming |
| `make posture-standard` | Resets all 18 joints to kinematic zero (straight legs) |
| `make posture-crouch` | Minimum chassis height (maximum leg compression) |
| `make posture-high` | Maximum chassis elevation stance |
| `make high-twist` | High stance with added horizontal Coxa swing |
| `make walk-alpha` | Sets Group A (LF, RM, LB) to Swing phase |
| `make walk-beta` | Sets Group B (RF, LM, RB) to Swing phase |

---

## 📐 Joint Sequence Reference
For manual `ros2 topic pub` debugging, use this interleaved joint order:

1.  **Group A (Power Group 1):** LF (0-2), RM (3-5), LB (6-8)
2.  **Group B (Power Group 2):** RF (9-11), LM (12-14), RB (15-17)

*Order per leg: Coxa, Femur, Tibia*.

---

## 🕹️ Interactive Testing
The robot can be controlled dynamically via keyboard or joystick through the `teleop_node`.

* **Keyboard:** Click the Webots window. Use **WASD** to walk/turn and keys **1, 2, 3** to change standing height.
* **Joystick:** Connect a **RadioMaster Pocket** in Joystick mode. Mapping details are located in `_DOCUMENTATION.md`.

---
