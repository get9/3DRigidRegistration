cmake_minimum_required(VERSION 2.8)
 
project(ResampleSliceData)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
 
add_executable(ResampleSliceData ResampleSliceData.cc)
target_link_libraries(ResampleSliceData ${ITK_LIBRARIES})
