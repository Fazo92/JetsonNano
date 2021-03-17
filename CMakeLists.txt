cmake_minimum_required(VERSION 3.10)

project(hello)

SET("OpenCV_DIR" "/home/dlinano/opencv/opencv")
find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)
find_package (Threads REQUIRED)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include ${realsense_INCLUDE_DIR})
add_executable(cv_hello main.cpp client.cpp rsfeatures.cpp)
target_link_libraries(cv_hello ${OpenCV_LIBS} ${realsense2_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
