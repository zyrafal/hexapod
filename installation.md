# 🚀 Hexadrone Workspace Installation Guide
## By AI
### 🛠 Prerequisites
* **Ubuntu 24.04**
* **ROS 2 Jazzy**
* **Webots R2025b**

---

### 🚀 Setup Script

*(Note: Make sure your `hexadrone_ws` folder is already placed in your home directory `~/` before running this. If it's not, move it there first.)*

```bash
# 1. Install necessary ROS 2 build tools
sudo apt update
sudo apt install -y python3-colcon-common-extensions python3-rosdep

# 2. Navigate to the workspace
cd ~/hexadrone_ws

# 3. Initialize and update rosdep (|| true prevents crash if already initialized)
sudo rosdep init || true
rosdep update

# 4. Install all package dependencies
rosdep install --from-paths src -y --ignore-src

# 5. Build the workspace using symlinks for easy editing
colcon build --symlink-install

# 6. Automatically source the workspace in your .bashrc (if not already there)
if ! grep -q "source ~/hexadrone_ws/install/setup.bash" ~/.bashrc; then
  echo "source ~/hexadrone_ws/install/setup.bash" >> ~/.bashrc
fi

# 7. Apply the source to the current terminal
source ~/.bashrc
