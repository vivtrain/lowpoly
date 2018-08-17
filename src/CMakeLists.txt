cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


project(lowpoly)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB lowpoly_SRC
    "*.cc"
    "*.h"
)

add_executable(output ${lowpoly_SRC})

target_link_libraries(output ${OpenCV_LIBS})

