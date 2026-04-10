cmake_minimum_required(VERSION 3.8)
project(hexadrone_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

# 1. Define the library and its source files
add_library(${PROJECT_NAME} SHARED
  src/kinematics.cpp
  src/gait_engine.cpp
  src/brain.cpp
)

# 2. Tell CMake where the headers are
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 3. Export the library so other packages can find it
ament_export_libraries(${PROJECT_NAME})
ament_export_include_directories(include)

# 4. Standard ROS 2 Install rules
install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

ament_package()
