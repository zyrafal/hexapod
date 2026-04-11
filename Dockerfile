FROM docker.io/osrf/ros:jazzy-desktop

ENV DEBIAN_FRONTEND=noninteractive
ENV WEBOTS_HOME=/usr/local/webots
ENV PATH=$PATH:$WEBOTS_HOME

# Install system dependencies and ROS 2 control packages
RUN apt-get update && apt-get install -y \
    wget \
    git \
    python3-pip \
    fish \
    libgl1 \
    libglu1-mesa \
    libglx-mesa0 \
    libxcomposite1 \
    libxcursor1 \
    libxtst6 \
    libnss3 \
    libasound2t64 \
    libxslt1.1 \
    ros-jazzy-ros2controlcli \
    ros-jazzy-ros2-controllers \
    && rm -rf /var/lib/apt/lists/*

# Install Webots R2025a and its ROS 2 interface
RUN wget https://github.com/cyberbotics/webots/releases/download/R2025a/webots_2025a_amd64.deb \
    && apt-get update \
    && apt-get install -y ./webots_2025a_amd64.deb ros-jazzy-webots-ros2 \
    && rm webots_2025a_amd64.deb \
    && rm -rf /var/lib/apt/lists/*

# Install bass (lets fish source bash scripts, needed for ROS 2 setup)
RUN mkdir -p ~/.config/fish/functions && \
    git clone --depth 1 https://github.com/edc/bass /tmp/bass && \
    cp /tmp/bass/functions/bass.fish ~/.config/fish/functions/ && \
    cp /tmp/bass/functions/__bass.py ~/.config/fish/functions/ && \
    rm -rf /tmp/bass

# Source ROS 2 and workspace on shell start; add launch alias
RUN mkdir -p ~/.config/fish && \
    echo "bass source /opt/ros/jazzy/setup.bash" >> ~/.config/fish/config.fish && \
    echo "bass source ~/hexadrone_ws/install/setup.bash" >> ~/.config/fish/config.fish && \
    echo "function webots; ros2 launch hexadrone_webots launch.py; end" >> ~/.config/fish/config.fish

RUN mkdir -p /root/.local/share/fish

WORKDIR /root/hexadrone_ws
ENTRYPOINT ["/usr/bin/fish"]
