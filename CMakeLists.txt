cmake_minimum_required(VERSION 3.16)
project(cam_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

#========================================

#定义的两个开关
option(BUILD_ARMOR_DETECT "是否编译主程序armor detect" ON)
option(BUILD_THRESHOLD_DEBUG "是否编译调试颜色工具 threshold_debug" ON)

#主程序
if(BUILD_ARMOR_DETECT)
    add_executable(armor_detect armor_detect.cpp)
    target_include_directories(armor_detect PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(armor_detect PRIVATE ${OpenCV_LIBS})
endif()

#测试调试inrange的文件
if(BUILD_THRESHOLD_DEBUG)
    add_executable(threshold_debug threshold_debug.cpp)
    target_include_directories(threshold_debug PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(threshold_debug PRIVATE ${OpenCV_LIBS})
endif()