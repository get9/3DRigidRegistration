cmake_minimum_required(VERSION 3.0)

project(RescaleIntensities)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

add_executable(rescaleIntensities RescaleIntensities.cxx )
target_link_libraries(rescaleIntensities ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
