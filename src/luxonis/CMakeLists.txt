cmake_minimum_required(VERSION 3.0.2)
project(luxonis)

find_package(catkin REQUIRED COMPONENTS
  roscpp
)
# set(OpenCV_DIR /home/cona/open_library/opencv-3.4.16/install/share/OpenCV)
set(depthai_DIR /home/cona/coga-robotics/depthai-core/install/lib/cmake/depthai)

find_package(catkin REQUIRED COMPONENTS
  roscpp
)
find_package(OpenCV REQUIRED)
# find_package(depthai CONFIG REQUIRED)
find_package(depthai REQUIRED)

catkin_package(
 INCLUDE_DIRS include
 LIBRARIES luxonis
 CATKIN_DEPENDS roscpp
 DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  # ${depthai_INCLUDE_DIRS}
)

link_libraries(
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
  # ${depthai_LIBRARIES}
  depthai::opencv
)

add_executable(oak-1_w src/oak-1_w.cpp)