cmake_minimum_required(VERSION 3.0)

project(ExtractSandGrainCentroids)

find_package(ITK REQUIRED)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

add_executable(extractSandGrainCentroids ExtractSandGrainCentroids.cxx )
add_executable(transformPointSet TransformPointSet.cxx )
add_executable(registerPointSets RegisterPointSets.cxx )
target_link_libraries(extractSandGrainCentroids ${ITK_LIBRARIES})
target_link_libraries(transformPointSet ${ITK_LIBRARIES})
target_link_libraries(registerPointSets ${ITK_LIBRARIES})

include(${ITK_USE_FILE})
