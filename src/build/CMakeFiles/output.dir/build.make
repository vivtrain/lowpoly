# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/projects/lowpoly/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/projects/lowpoly/src/build

# Include any dependencies generated for this target.
include CMakeFiles/output.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/output.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/output.dir/flags.make

CMakeFiles/output.dir/main.cc.o: CMakeFiles/output.dir/flags.make
CMakeFiles/output.dir/main.cc.o: ../main.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/lowpoly/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/output.dir/main.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/output.dir/main.cc.o -c /home/ubuntu/projects/lowpoly/src/main.cc

CMakeFiles/output.dir/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/output.dir/main.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/lowpoly/src/main.cc > CMakeFiles/output.dir/main.cc.i

CMakeFiles/output.dir/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/output.dir/main.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/lowpoly/src/main.cc -o CMakeFiles/output.dir/main.cc.s

CMakeFiles/output.dir/main.cc.o.requires:

.PHONY : CMakeFiles/output.dir/main.cc.o.requires

CMakeFiles/output.dir/main.cc.o.provides: CMakeFiles/output.dir/main.cc.o.requires
	$(MAKE) -f CMakeFiles/output.dir/build.make CMakeFiles/output.dir/main.cc.o.provides.build
.PHONY : CMakeFiles/output.dir/main.cc.o.provides

CMakeFiles/output.dir/main.cc.o.provides.build: CMakeFiles/output.dir/main.cc.o


# Object files for target output
output_OBJECTS = \
"CMakeFiles/output.dir/main.cc.o"

# External object files for target output
output_EXTERNAL_OBJECTS =

output: CMakeFiles/output.dir/main.cc.o
output: CMakeFiles/output.dir/build.make
output: /usr/lib/libopencv_vstab.so.2.4.13
output: /usr/lib/libopencv_imuvstab.so.2.4.13
output: /usr/lib/libopencv_facedetect.so.2.4.13
output: /usr/lib/libopencv_esm_panorama.so.2.4.13
output: /usr/lib/libopencv_detection_based_tracker.so.2.4.13
output: /usr/lib/libopencv_videostab.so.2.4.13
output: /usr/lib/libopencv_ts.a
output: /usr/lib/libopencv_superres.so.2.4.13
output: /usr/lib/libopencv_contrib.so.2.4.13
output: /usr/lib/libopencv_tegra.so.2.4.13
output: /usr/lib/libopencv_stitching.so.2.4.13
output: /usr/lib/libopencv_gpu.so.2.4.13
output: /usr/lib/libopencv_photo.so.2.4.13
output: /usr/lib/libopencv_legacy.so.2.4.13
output: /usr/local/cuda/lib64/libcufft.so
output: /usr/lib/libopencv_video.so.2.4.13
output: /usr/lib/libopencv_objdetect.so.2.4.13
output: /usr/lib/libopencv_ml.so.2.4.13
output: /usr/lib/libopencv_calib3d.so.2.4.13
output: /usr/lib/libopencv_features2d.so.2.4.13
output: /usr/lib/libopencv_highgui.so.2.4.13
output: /usr/lib/libopencv_imgproc.so.2.4.13
output: /usr/lib/libopencv_flann.so.2.4.13
output: /usr/lib/libopencv_core.so.2.4.13
output: /usr/local/cuda/lib64/libcudart.so
output: /usr/local/cuda/lib64/libnppc.so
output: /usr/local/cuda/lib64/libnppi.so
output: /usr/local/cuda/lib64/libnpps.so
output: CMakeFiles/output.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/projects/lowpoly/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable output"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/output.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/output.dir/build: output

.PHONY : CMakeFiles/output.dir/build

CMakeFiles/output.dir/requires: CMakeFiles/output.dir/main.cc.o.requires

.PHONY : CMakeFiles/output.dir/requires

CMakeFiles/output.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/output.dir/cmake_clean.cmake
.PHONY : CMakeFiles/output.dir/clean

CMakeFiles/output.dir/depend:
	cd /home/ubuntu/projects/lowpoly/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/projects/lowpoly/src /home/ubuntu/projects/lowpoly/src /home/ubuntu/projects/lowpoly/src/build /home/ubuntu/projects/lowpoly/src/build /home/ubuntu/projects/lowpoly/src/build/CMakeFiles/output.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/output.dir/depend

