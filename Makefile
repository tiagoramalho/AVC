# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /home/ludee/Documents/CAV/AVC

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ludee/Documents/CAV/AVC

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ludee/Documents/CAV/AVC/CMakeFiles /home/ludee/Documents/CAV/AVC/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ludee/Documents/CAV/AVC/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named wavhist

# Build rule for target.
wavhist: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wavhist
.PHONY : wavhist

# fast build rule for target.
wavhist/fast:
	$(MAKE) -f CMakeFiles/wavhist.dir/build.make CMakeFiles/wavhist.dir/build
.PHONY : wavhist/fast

#=============================================================================
# Target rules for targets named wavcp

# Build rule for target.
wavcp: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wavcp
.PHONY : wavcp

# fast build rule for target.
wavcp/fast:
	$(MAKE) -f CMakeFiles/wavcp.dir/build.make CMakeFiles/wavcp.dir/build
.PHONY : wavcp/fast

#=============================================================================
# Target rules for targets named wavquant

# Build rule for target.
wavquant: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wavquant
.PHONY : wavquant

# fast build rule for target.
wavquant/fast:
	$(MAKE) -f CMakeFiles/wavquant.dir/build.make CMakeFiles/wavquant.dir/build
.PHONY : wavquant/fast

#=============================================================================
# Target rules for targets named wavcb

# Build rule for target.
wavcb: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wavcb
.PHONY : wavcb

# fast build rule for target.
wavcb/fast:
	$(MAKE) -f CMakeFiles/wavcb.dir/build.make CMakeFiles/wavcb.dir/build
.PHONY : wavcb/fast

#=============================================================================
# Target rules for targets named wavcmp

# Build rule for target.
wavcmp: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wavcmp
.PHONY : wavcmp

# fast build rule for target.
wavcmp/fast:
	$(MAKE) -f CMakeFiles/wavcmp.dir/build.make CMakeFiles/wavcmp.dir/build
.PHONY : wavcmp/fast

wavcb.o: wavcb.cpp.o

.PHONY : wavcb.o

# target to build an object file
wavcb.cpp.o:
	$(MAKE) -f CMakeFiles/wavcb.dir/build.make CMakeFiles/wavcb.dir/wavcb.cpp.o
.PHONY : wavcb.cpp.o

wavcb.i: wavcb.cpp.i

.PHONY : wavcb.i

# target to preprocess a source file
wavcb.cpp.i:
	$(MAKE) -f CMakeFiles/wavcb.dir/build.make CMakeFiles/wavcb.dir/wavcb.cpp.i
.PHONY : wavcb.cpp.i

wavcb.s: wavcb.cpp.s

.PHONY : wavcb.s

# target to generate assembly for a file
wavcb.cpp.s:
	$(MAKE) -f CMakeFiles/wavcb.dir/build.make CMakeFiles/wavcb.dir/wavcb.cpp.s
.PHONY : wavcb.cpp.s

wavcmp.o: wavcmp.cpp.o

.PHONY : wavcmp.o

# target to build an object file
wavcmp.cpp.o:
	$(MAKE) -f CMakeFiles/wavcmp.dir/build.make CMakeFiles/wavcmp.dir/wavcmp.cpp.o
.PHONY : wavcmp.cpp.o

wavcmp.i: wavcmp.cpp.i

.PHONY : wavcmp.i

# target to preprocess a source file
wavcmp.cpp.i:
	$(MAKE) -f CMakeFiles/wavcmp.dir/build.make CMakeFiles/wavcmp.dir/wavcmp.cpp.i
.PHONY : wavcmp.cpp.i

wavcmp.s: wavcmp.cpp.s

.PHONY : wavcmp.s

# target to generate assembly for a file
wavcmp.cpp.s:
	$(MAKE) -f CMakeFiles/wavcmp.dir/build.make CMakeFiles/wavcmp.dir/wavcmp.cpp.s
.PHONY : wavcmp.cpp.s

wavcp.o: wavcp.cpp.o

.PHONY : wavcp.o

# target to build an object file
wavcp.cpp.o:
	$(MAKE) -f CMakeFiles/wavcp.dir/build.make CMakeFiles/wavcp.dir/wavcp.cpp.o
.PHONY : wavcp.cpp.o

wavcp.i: wavcp.cpp.i

.PHONY : wavcp.i

# target to preprocess a source file
wavcp.cpp.i:
	$(MAKE) -f CMakeFiles/wavcp.dir/build.make CMakeFiles/wavcp.dir/wavcp.cpp.i
.PHONY : wavcp.cpp.i

wavcp.s: wavcp.cpp.s

.PHONY : wavcp.s

# target to generate assembly for a file
wavcp.cpp.s:
	$(MAKE) -f CMakeFiles/wavcp.dir/build.make CMakeFiles/wavcp.dir/wavcp.cpp.s
.PHONY : wavcp.cpp.s

wavhist.o: wavhist.cpp.o

.PHONY : wavhist.o

# target to build an object file
wavhist.cpp.o:
	$(MAKE) -f CMakeFiles/wavhist.dir/build.make CMakeFiles/wavhist.dir/wavhist.cpp.o
.PHONY : wavhist.cpp.o

wavhist.i: wavhist.cpp.i

.PHONY : wavhist.i

# target to preprocess a source file
wavhist.cpp.i:
	$(MAKE) -f CMakeFiles/wavhist.dir/build.make CMakeFiles/wavhist.dir/wavhist.cpp.i
.PHONY : wavhist.cpp.i

wavhist.s: wavhist.cpp.s

.PHONY : wavhist.s

# target to generate assembly for a file
wavhist.cpp.s:
	$(MAKE) -f CMakeFiles/wavhist.dir/build.make CMakeFiles/wavhist.dir/wavhist.cpp.s
.PHONY : wavhist.cpp.s

wavquant.o: wavquant.cpp.o

.PHONY : wavquant.o

# target to build an object file
wavquant.cpp.o:
	$(MAKE) -f CMakeFiles/wavquant.dir/build.make CMakeFiles/wavquant.dir/wavquant.cpp.o
.PHONY : wavquant.cpp.o

wavquant.i: wavquant.cpp.i

.PHONY : wavquant.i

# target to preprocess a source file
wavquant.cpp.i:
	$(MAKE) -f CMakeFiles/wavquant.dir/build.make CMakeFiles/wavquant.dir/wavquant.cpp.i
.PHONY : wavquant.cpp.i

wavquant.s: wavquant.cpp.s

.PHONY : wavquant.s

# target to generate assembly for a file
wavquant.cpp.s:
	$(MAKE) -f CMakeFiles/wavquant.dir/build.make CMakeFiles/wavquant.dir/wavquant.cpp.s
.PHONY : wavquant.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... wavhist"
	@echo "... wavcp"
	@echo "... rebuild_cache"
	@echo "... wavquant"
	@echo "... wavcb"
	@echo "... wavcmp"
	@echo "... edit_cache"
	@echo "... wavcb.o"
	@echo "... wavcb.i"
	@echo "... wavcb.s"
	@echo "... wavcmp.o"
	@echo "... wavcmp.i"
	@echo "... wavcmp.s"
	@echo "... wavcp.o"
	@echo "... wavcp.i"
	@echo "... wavcp.s"
	@echo "... wavhist.o"
	@echo "... wavhist.i"
	@echo "... wavhist.s"
	@echo "... wavquant.o"
	@echo "... wavquant.i"
	@echo "... wavquant.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

