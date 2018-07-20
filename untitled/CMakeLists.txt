cmake_minimum_required(VERSION 3.10)
project(untitled)

set(CMAKE_CXX_STANDARD 11)

list(APPEND untitled_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/libydlidar_driver.so)

INCLUDE_DIRECTORIES(
        include
)

AUX_SOURCE_DIRECTORY(third AllPart)

add_executable(untitled main.cpp ${AllPart})

TARGET_LINK_LIBRARIES(untitled ${untitled_LIBRARIES} pthread)