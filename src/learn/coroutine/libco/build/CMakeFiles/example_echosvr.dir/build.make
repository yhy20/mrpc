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
CMAKE_SOURCE_DIR = /root/mrpc/src/learn/coroutine/libco

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/mrpc/src/learn/coroutine/libco/build

# Include any dependencies generated for this target.
include CMakeFiles/example_echosvr.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/example_echosvr.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/example_echosvr.dir/flags.make

CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o: CMakeFiles/example_echosvr.dir/flags.make
CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o: ../example_echosvr.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/mrpc/src/learn/coroutine/libco/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o -c /root/mrpc/src/learn/coroutine/libco/example_echosvr.cpp

CMakeFiles/example_echosvr.dir/example_echosvr.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/example_echosvr.dir/example_echosvr.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/mrpc/src/learn/coroutine/libco/example_echosvr.cpp > CMakeFiles/example_echosvr.dir/example_echosvr.cpp.i

CMakeFiles/example_echosvr.dir/example_echosvr.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/example_echosvr.dir/example_echosvr.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/mrpc/src/learn/coroutine/libco/example_echosvr.cpp -o CMakeFiles/example_echosvr.dir/example_echosvr.cpp.s

# Object files for target example_echosvr
example_echosvr_OBJECTS = \
"CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o"

# External object files for target example_echosvr
example_echosvr_EXTERNAL_OBJECTS =

../bin/example_echosvr: CMakeFiles/example_echosvr.dir/example_echosvr.cpp.o
../bin/example_echosvr: CMakeFiles/example_echosvr.dir/build.make
../bin/example_echosvr: libcolib.a
../bin/example_echosvr: CMakeFiles/example_echosvr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/mrpc/src/learn/coroutine/libco/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/example_echosvr"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example_echosvr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/example_echosvr.dir/build: ../bin/example_echosvr

.PHONY : CMakeFiles/example_echosvr.dir/build

CMakeFiles/example_echosvr.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/example_echosvr.dir/cmake_clean.cmake
.PHONY : CMakeFiles/example_echosvr.dir/clean

CMakeFiles/example_echosvr.dir/depend:
	cd /root/mrpc/src/learn/coroutine/libco/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/mrpc/src/learn/coroutine/libco /root/mrpc/src/learn/coroutine/libco /root/mrpc/src/learn/coroutine/libco/build /root/mrpc/src/learn/coroutine/libco/build /root/mrpc/src/learn/coroutine/libco/build/CMakeFiles/example_echosvr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/example_echosvr.dir/depend

