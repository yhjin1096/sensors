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

# for zed2i_sdk_pub
find_package(ZED 3 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)
#

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
  
  # for zed2i_sdk_pub
  ${ZED_INCLUDE_DIRS}
  ${CUDA_INCLUDE_DIRS}
  #
)

link_libraries(
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

# for zed2i_sdk_pub
link_directories(
  ${ZED_LIBRARY_DIR}
  ${CUDA_LIBRARY_DIRS}
  ${OpenCV_LIBRARY_DIRS}
)
#

add_executable(zed2i src/zed2i.cpp)
add_executable(zed2i_pub src/zed2i_pub.cpp)
add_executable(zed2i_sdk_pub src/zed2i_sdk_pub.cpp)

# for zed2i_sdk_pub
SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
TARGET_LINK_LIBRARIES(zed2i_sdk_pub ${ZED_LIBS} ${OpenCV_LIBRARIES})
#