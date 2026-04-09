```
sudo apt update
sudo apt install -y podman podman-compose
podman-compose up --build
podman-compose up -d
podman exec -it jazzy_webots_sim bash

(Pak v prostredi, mel by se ukazat prompt "root@host:/# ")
ros2 launch hexadrone_webots launch.py
```
