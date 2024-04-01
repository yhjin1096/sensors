cmake_minimum_required(VERSION 3.0.2)
project(astar_fisheye_stereo)

set(OpenCV_DIR /home/cona/open_library/opencv-3.4.16/install/share/OpenCV)
find_package(catkin REQUIRED COMPONENTS
  roscpp
)
find_package(OpenCV REQUIRED)

catkin_package(
 INCLUDE_DIRS include
 LIBRARIES astar_fisheye_stereo
 CATKIN_DEPENDS roscpp
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

add_executable(ste_3 src/ste_3.cpp)