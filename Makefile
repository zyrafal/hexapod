.PHONY: build shell stop

build:
	podman-compose up --build -d

shell:
	touch .container_fish_history
	podman-compose up -d
	podman exec -it jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && colcon build --symlink-install; exec fish"

stop:
	podman-compose down -t0
