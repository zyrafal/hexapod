# Use the official ROS 2 Jazzy Desktop image (Ubuntu 24.04)
FROM docker.io/osrf/ros:jazzy-desktop

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV WEBOTS_HOME=/usr/local/webots
ENV PATH=$PATH:$WEBOTS_HOME

# Fixed dependencies for Ubuntu 24.04
RUN apt-get update && apt-get install -y \
    wget \
    git \
    python3-pip \
    # Updated Graphics/GUI libraries
    libgl1 \
    libglu1-mesa \
    libglx-mesa0 \
    libxcomposite1 \
    libxcursor1 \
    libxtst6 \
    libnss3 \
    # Updated Sound/XSLT libraries for 24.04
    libasound2t64 \
    libxslt1.1 \
    ros-jazzy-ros2controlcli \
    ros-jazzy-ros2-controllers \
    && rm -rf /var/lib/apt/lists/*

# Download and Install Webots R2025b
# Note: Using the official Debian package for Webots
RUN wget https://github.com/cyberbotics/webots/releases/download/R2025a/webots_2025a_amd64.deb \
    && apt-get update \
    && apt-get install -y ./webots_2025a_amd64.deb \
    && rm webots_2025a_amd64.deb \
    && rm -rf /var/lib/apt/lists/*

# Install webots_ros2 interface
RUN apt-get update && apt-get install -y \
    ros-jazzy-webots-ros2 \
    && rm -rf /var/lib/apt/lists/*


# Environment Setup
RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc && \
    echo "source ~/hexadrone_ws/install/setup.bash" >> ~/.bashrc

ENTRYPOINT ["/bin/bash"]
