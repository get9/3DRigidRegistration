cmake_minimum_required(VERSION 2.8)
 
project(ShrinkImageFilter)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
add_executable(ShrinkImageFilter ShrinkImageFilter.cxx)
target_link_libraries(ShrinkImageFilter ${ITK_LIBRARIES})
