cmake_minimum_required(VERSION 2.8)
 
project(ShrinkImageFilter)
 
find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -g")
set(CMAKE_BUILD_TYPE "Debug")
include(${ITK_USE_FILE})
 
add_executable(ShrinkImageFilter ShrinkImageFilter.cxx)
target_link_libraries(ShrinkImageFilter ${ITK_LIBRARIES})
