cmake_minimum_required(VERSION 3.0.2)
project(nvidia_zed)

find_package(catkin REQUIRED COMPONENTS
  roscpp
)
set(OpenCV_DIR /home/cona/open_library/opencv-3.4.16/install/share/OpenCV)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  cv_bridge
  image_transport
)
# find_package(OpenCV REQUIRED)

catkin_package(
 INCLUDE_DIRS include
 LIBRARIES nvidia_zed
 CATKIN_DEPENDS roscpp cv_bridge image_transport
 DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

link_libraries(
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

add_executable(zed2i src/zed2i.cpp)
add_executable(zed2i_pub src/zed2i_pub.cpp)