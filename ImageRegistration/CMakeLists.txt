cmake_minimum_required(VERSION 3.0)

project(ImageRegistration)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_BUILD_TYPE "Release")

add_executable(register VolumeRegistration.cxx )
target_link_libraries(register  ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
