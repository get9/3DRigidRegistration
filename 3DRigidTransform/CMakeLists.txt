cmake_minimum_required(VERSION 3.0)

project(3DRigidTransformation)

find_package(ITK REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -g")
set(CMAKE_BUILD_TYPE "Debug")

add_executable(3DRigidTransform ResampleImageFilter4.cxx )
target_link_libraries(3DRigidTransform ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
