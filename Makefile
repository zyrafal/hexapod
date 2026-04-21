.PHONY: build shell stop deploy posture-prone posture-standard posture-crouch posture-high high-twist walk-alpha walk-beta

# Core Container Commands
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

deploy:
	cd src/hexadrone_esp32 && pio run --target upload

# Posture Aliases (in radians)

posture-prone:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.00,  0.70, -0.45,   # LF (Group A) \
	  0.00,  0.70, -0.45,   # RM (Group A) \
	  0.00,  0.70, -0.45,   # LB (Group A) \
	  0.00, -0.70,  0.45,   # RF (Group B) \
	  0.00, -0.70,  0.45,   # LM (Group B) \
	  0.00, -0.70,  0.45    # RB (Group B) \
	]}\""

posture-standard:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.00,  0.00,  0.00,   # LF \
	  0.00,  0.00,  0.00,   # RM \
	  0.00,  0.00,  0.00,   # LB \
	  0.00,  0.00,  0.00,   # RF \
	  0.00,  0.00,  0.00,   # LM \
	  0.00,  0.00,  0.00    # RB \
	]}\""

posture-crouch:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.00,  1.57,  1.57,   # LF \
	  0.00,  1.57,  1.57,   # RM \
	  0.00,  1.57,  1.57,   # LB \
	  0.00, -1.57, -1.57,   # RF \
	  0.00, -1.57, -1.57,   # LM \
	  0.00, -1.57, -1.57    # RB \
	]}\""

posture-high:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.00, -0.70, -0.60,   # LF \
	  0.00, -0.70, -0.60,   # RM \
	  0.00, -0.70, -0.60,   # LB \
	  0.00,  0.70,  0.60,   # RF \
	  0.00,  0.70,  0.60,   # LM \
	  0.00,  0.70,  0.60    # RB \
	]}\""

high-twist:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.26, -0.70, -0.60,   # LF (Coxa Twist) \
	  0.26, -0.70, -0.60,   # RM (Coxa Twist) \
	  0.26, -0.70, -0.60,   # LB (Coxa Twist) \
	  0.26,  0.70,  0.60,   # RF (Coxa Twist) \
	  0.26,  0.70,  0.60,   # LM (Coxa Twist) \
	  0.26,  0.70,  0.60    # RB (Coxa Twist) \
	]}\""

walk-alpha:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.00, -0.45, -0.60,   # LF (A - Swing) \
	  0.00, -0.45, -0.60,   # RM (A - Swing) \
	  0.00, -0.45, -0.60,   # LB (A - Swing) \
	  0.26,  0.70,  0.60,   # RF (B - Stance) \
	  0.26,  0.70,  0.60,   # LM (B - Stance) \
	  0.26,  0.70,  0.60    # RB (B - Stance) \
	]}\""

walk-beta:
	podman exec jazzy_webots_sim bash -c "source /opt/ros/jazzy/setup.bash && \
	ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray \"{data: [ \
	  0.26, -0.70, -0.60,   # LF (A - Stance) \
	  0.26, -0.70, -0.60,   # RM (A - Stance) \
	  0.26, -0.70, -0.60,   # LB (A - Stance) \
	  0.00,  0.45,  0.60,   # RF (B - Swing) \
	  0.00,  0.45,  0.60,   # LM (B - Swing) \
	  0.00,  0.45,  0.60    # RB (B - Swing) \
	]}\""
