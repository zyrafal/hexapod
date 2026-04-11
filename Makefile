.PHONY: build shell stop

build:
	touch .container_fish_history
	podman-compose up --build --force-recreate -d

shell:
	touch .container_fish_history
	podman-compose up -d
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && colcon build --symlink-install"
	podman exec -it jazzy_webots_sim fish

stop:
	podman-compose down -t0
