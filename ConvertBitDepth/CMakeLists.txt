cmake_minimum_required(VERSION 3.0)

project(ConvertBitDepth)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -g")
set(CMAKE_BUILD_TYPE "Release")

add_executable(16to8 16to8.cxx )
add_executable(8to16 8to16.cxx )
target_link_libraries(16to8  ${ITK_LIBRARIES})
target_link_libraries(8to16  ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
