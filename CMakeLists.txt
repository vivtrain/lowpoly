# Project: lowpoly
cmake_minimum_required(VERSION 3.16)
project(lowpoly LANGUAGES CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apply compile flags to to all targets
add_compile_options(-Wall -Wextra -g)

# Generate compile_commands.json for LSPs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Provide ${OpenCV_INCLUDE_DIRS}, ${OpenCV_LIBS}
find_package(OpenCV REQUIRED)

# Log OpenCV status
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# Put all binaries in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Delaunay module ##############################################################
# Create a target library for Delaunay
add_library(delaunay STATIC
  src/delaunay/delaunay.cpp
  src/delaunay/quad_edge_ref.cpp
)
# Tell CMake where necessary headers are, expose these to anyone who links
target_include_directories(delaunay
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
# Tell CMake where to put libs
set_target_properties(delaunay PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/delaunay
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/delaunay
)
# Link Delaunay against OpenCV libs, expose to anyone who links
target_link_libraries(delaunay PUBLIC ${OpenCV_LIBS})
# Create unit tests for this library
add_executable(test_delaunay tests/delaunay/test_delaunay.cpp)
# Link this against the Delaunay library
target_link_libraries(test_delaunay PRIVATE delaunay)
# Place the binary in build/bin/tests/
set_target_properties(test_delaunay PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/
)
# End Delaunay #################################################################

# Main Executable ##############################################################
# Define an executable target for generate
file(GLOB SRC_FILES "src/*.cpp")
add_executable(generate ${SRC_FILES})
# Include the project-wide includes and OpenCV
target_include_directories(generate
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${OpenCV_INCLUDE_DIRS}
)
# Link target against OpenCV and Delaunay
target_link_libraries(generate
  PRIVATE
    delaunay
    ${OpenCV_LIBS}
)
# End Main Executable ##########################################################
