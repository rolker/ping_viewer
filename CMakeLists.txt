cmake_minimum_required(VERSION 3.1.0)

project(sonardyne_raw)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
#endif()

find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

add_executable(ping_viewer
  main.cpp
  ping_base.cpp
  ping_view.cpp
  ping_view.ui
  ping_viewer.cpp
  ping_viewer.ui
  raw_ping.cpp
  wave_view.cpp
  waveform.cpp
)

target_link_libraries(ping_viewer Qt5::Widgets Qt5::Charts)

add_executable(echogram_viewer
  colormaps.qrc
  echogram_viewer.cpp
  echogram_viewer.ui
  echogram.cpp
  ping.cpp
  ping_base.cpp
  ping_view.cpp
  ping_view.ui
  raw_ping.cpp
  wave_view.cpp
  waveform.cpp
)

target_link_libraries(echogram_viewer Qt5::Widgets Qt5::Charts)


find_package(catkin REQUIRED COMPONENTS rosbag marine_acoustic_msgs tf2_msgs)

include_directories(${catkin_INCLUDE_DIRS})

add_executable(pings_to_bag
  pings_to_bag.cpp
  ping.cpp
  ping_base.cpp
  raw_ping.cpp
)

target_link_libraries(pings_to_bag ${catkin_LIBRARIES})
