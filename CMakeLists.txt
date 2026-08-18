cmake_minimum_required(VERSION 3.16)
project(cam_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

#add_executable(armor_detect armor_detect.cpp)
#target_include_directories(armor_detect PRIVATE ${OpenCV_INCLUDE_DIRS})
#target_link_libraries(armor_detect PRIVATE ${OpenCV_LIBS})

add_executable(threshold_debug threshold_debug.cpp)
target_include_directories(threshold_debug PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(threshold_debug PRIVATE ${OpenCV_LIBS})
#测试调试inrange的文件