cmake_minimum_required(VERSION 3.8)
project(hexadrone_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Find Dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(hexadrone_core REQUIRED)

# 2. Define the Node
add_executable(teleop_node src/teleop_node.cpp)

# 3. Link Dependencies
ament_target_dependencies(teleop_node
  rclcpp
  sensor_msgs
  hexadrone_core
)

target_include_directories(teleop_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(teleop_node
  rclcpp
  sensor_msgs
  hexadrone_core
)

# 4. Install Logic
install(TARGETS
  teleop_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
