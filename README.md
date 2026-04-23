# 🤖 Hexadrone

A versatile, ROS 2 and ESP32 compatible software stack for a 18-DOF hexapod robot (drone). This repository contains the unified C++ brain, kinematics engine, Webots simulation environment, and the physical PlatformIO hardware layer.

📖 **For full hardware specifications, control mappings, architecture details, and system logic, please read the [Documentation](DOCUMENTATION.md).**

---

### 🛠️ Build Instructions
This ESP32 project relies on a shared core logic library. You must clone the entire ROS 2 workspace to maintain the correct folder structure:

1. Clone the complete workspace: `git clone yehorderehus/hexadrone`
2. Ensure `hexadrone_core` and `hexadrone_esp32` exist in the same parent directory.
3. Open the `hexadrone_esp32` folder in PlatformIO.
4. Click **Build**. PlatformIO will automatically resolve the symlink and download all external I2C/Radio libraries.

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

---

## 📡 Wireless Connection & OTA Updates

The Hexadrone features a state-aware WiFi manager designed to prioritize radio control safety. To prevent 2.4GHz interference with the ELRS receiver, **WiFi is strictly disabled while the drone is ARMED.** To connect to the drone wirelessly for diagnostics or code updates, you must be in the **DISARMED** or **OE-Kill** state.

### 1. Connecting to the Drone
1. **Configure your Network:** Ensure your local WiFi credentials (SSID and Password) are correctly set in `include/config.h`.
2. **Disarm the Drone:** Ensure your radio's ARM switch (Switch SA) is in the low/disarmed position. 
3. **Power On:** Boot the drone. The ESP32 will automatically connect to your designated WiFi network.
4. **Access the Network:** Make sure your PC or mobile device is connected to the same WiFi network. The drone will broadcast its presence via mDNS.

### 2. Downloading Blackbox Logs
You do not need a USB cable to retrieve crash or walking data. While the drone is disarmed and connected to WiFi:
* Open any web browser on your connected device.
* Navigate to: `http://hexadrone.local/blackbox`
* The latest telemetry and system logs will display as plain text, which you can save for analysis.

### 3. Over-The-Air (OTA) Firmware Updates
You can completely rewrite the ESP32's firmware wirelessly using PlatformIO's built-in ArduinoOTA support. 

**Setup `platformio.ini`:**
Add the following two lines to your environment configuration in `platformio.ini`:
```ini
[env:esp32doit-devkit-v1]
; ... existing configurations ...

upload_protocol = espota
upload_port = hexadrone.local  ; You can also use the exact IP address if mDNS fails
```

## Flashing the Code:

- Ensure the drone is powered on and DISARMED.

- Click the standard Upload button in PlatformIO (or run pio run -t upload).

- PlatformIO will automatically compile the code, locate the drone on your local network, and push the update.

- The ESP32 will restart automatically with the new firmware.
