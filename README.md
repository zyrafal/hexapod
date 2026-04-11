# Setup
Install the required packages (on Ubuntu):
```
sudo apt update
sudo apt install -y podman podman-compose
```

# Usage

| Command | Description |
|---|---|
| `make build` | Build or rebuild the container image |
| `make shell` | Start the container (if not running) and open a bash shell |
| `make stop` | Stop the container |

Inside the shell, launch the Webots simulation:
```
ros2 launch hexadrone_webots launch.py
```
