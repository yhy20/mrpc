# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /usr/local/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/sylar/build

# Include any dependencies generated for this target.
include src/base/tests/CMakeFiles/timestamp_test.dir/depend.make

# Include the progress variables for this target.
include src/base/tests/CMakeFiles/timestamp_test.dir/progress.make

# Include the compile flags for this target's objects.
include src/base/tests/CMakeFiles/timestamp_test.dir/flags.make

src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o: src/base/tests/CMakeFiles/timestamp_test.dir/flags.make
src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o: ../src/base/tests/TimeStamp_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o"
	cd /root/sylar/build/src/base/tests && g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o -c /root/sylar/src/base/tests/TimeStamp_test.cc

src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.i"
	cd /root/sylar/build/src/base/tests && g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/sylar/src/base/tests/TimeStamp_test.cc > CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.i

src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.s"
	cd /root/sylar/build/src/base/tests && g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/sylar/src/base/tests/TimeStamp_test.cc -o CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.s

# Object files for target timestamp_test
timestamp_test_OBJECTS = \
"CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o"

# External object files for target timestamp_test
timestamp_test_EXTERNAL_OBJECTS =

../bin/tests/base/timestamp_test: src/base/tests/CMakeFiles/timestamp_test.dir/TimeStamp_test.cc.o
../bin/tests/base/timestamp_test: src/base/tests/CMakeFiles/timestamp_test.dir/build.make
../bin/tests/base/timestamp_test: ../lib/libmrpc_base.so
../bin/tests/base/timestamp_test: src/base/tests/CMakeFiles/timestamp_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../../bin/tests/base/timestamp_test"
	cd /root/sylar/build/src/base/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/timestamp_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/base/tests/CMakeFiles/timestamp_test.dir/build: ../bin/tests/base/timestamp_test

.PHONY : src/base/tests/CMakeFiles/timestamp_test.dir/build

src/base/tests/CMakeFiles/timestamp_test.dir/clean:
	cd /root/sylar/build/src/base/tests && $(CMAKE_COMMAND) -P CMakeFiles/timestamp_test.dir/cmake_clean.cmake
.PHONY : src/base/tests/CMakeFiles/timestamp_test.dir/clean

src/base/tests/CMakeFiles/timestamp_test.dir/depend:
	cd /root/sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/sylar /root/sylar/src/base/tests /root/sylar/build /root/sylar/build/src/base/tests /root/sylar/build/src/base/tests/CMakeFiles/timestamp_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/base/tests/CMakeFiles/timestamp_test.dir/depend

