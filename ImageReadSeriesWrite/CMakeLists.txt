cmake_minimum_required(VERSION 3.1)

project(ImageReadSeriesImageWrite)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -g")
set(CMAKE_BUILD_TYPE "Release")

add_executable(mhd2slices ImageReadImageSeriesWrite.cxx)
target_link_libraries(mhd2slices ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
