cmake_minimum_required(VERSION 2.8...3.15)
project(against-raksamp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE "Release")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME} PRIVATE
	source/main.cpp
	source/bitstream.cpp
) 

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)