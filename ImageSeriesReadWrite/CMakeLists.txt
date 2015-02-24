cmake_minimum_required(VERSION 3.1)

project(SeriesImageWriter)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -g")
set(CMAKE_BUILD_TYPE "Debug")

add_executable(ImageSeriesReadWrite ImageSeriesReadWrite.cxx )
target_link_libraries(ImageSeriesReadWrite  ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
