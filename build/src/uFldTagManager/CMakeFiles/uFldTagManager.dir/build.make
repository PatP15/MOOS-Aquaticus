# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pmp2149/moos-ivp-aquaticus-pmp2149

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pmp2149/moos-ivp-aquaticus-pmp2149/build

# Include any dependencies generated for this target.
include src/uFldTagManager/CMakeFiles/uFldTagManager.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/uFldTagManager/CMakeFiles/uFldTagManager.dir/compiler_depend.make

# Include the progress variables for this target.
include src/uFldTagManager/CMakeFiles/uFldTagManager.dir/progress.make

# Include the compile flags for this target's objects.
include src/uFldTagManager/CMakeFiles/uFldTagManager.dir/flags.make

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/flags.make
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o: ../src/uFldTagManager/TagManager.cpp
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pmp2149/moos-ivp-aquaticus-pmp2149/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o -MF CMakeFiles/uFldTagManager.dir/TagManager.cpp.o.d -o CMakeFiles/uFldTagManager.dir/TagManager.cpp.o -c /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager.cpp

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uFldTagManager.dir/TagManager.cpp.i"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager.cpp > CMakeFiles/uFldTagManager.dir/TagManager.cpp.i

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uFldTagManager.dir/TagManager.cpp.s"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager.cpp -o CMakeFiles/uFldTagManager.dir/TagManager.cpp.s

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/flags.make
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o: ../src/uFldTagManager/TagManager_Info.cpp
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pmp2149/moos-ivp-aquaticus-pmp2149/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o -MF CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o.d -o CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o -c /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager_Info.cpp

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.i"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager_Info.cpp > CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.i

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.s"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/TagManager_Info.cpp -o CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.s

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/flags.make
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o: ../src/uFldTagManager/VTag.cpp
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pmp2149/moos-ivp-aquaticus-pmp2149/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o -MF CMakeFiles/uFldTagManager.dir/VTag.cpp.o.d -o CMakeFiles/uFldTagManager.dir/VTag.cpp.o -c /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/VTag.cpp

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uFldTagManager.dir/VTag.cpp.i"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/VTag.cpp > CMakeFiles/uFldTagManager.dir/VTag.cpp.i

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uFldTagManager.dir/VTag.cpp.s"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/VTag.cpp -o CMakeFiles/uFldTagManager.dir/VTag.cpp.s

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/flags.make
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o: ../src/uFldTagManager/main.cpp
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pmp2149/moos-ivp-aquaticus-pmp2149/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o -MF CMakeFiles/uFldTagManager.dir/main.cpp.o.d -o CMakeFiles/uFldTagManager.dir/main.cpp.o -c /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/main.cpp

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uFldTagManager.dir/main.cpp.i"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/main.cpp > CMakeFiles/uFldTagManager.dir/main.cpp.i

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uFldTagManager.dir/main.cpp.s"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager/main.cpp -o CMakeFiles/uFldTagManager.dir/main.cpp.s

# Object files for target uFldTagManager
uFldTagManager_OBJECTS = \
"CMakeFiles/uFldTagManager.dir/TagManager.cpp.o" \
"CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o" \
"CMakeFiles/uFldTagManager.dir/VTag.cpp.o" \
"CMakeFiles/uFldTagManager.dir/main.cpp.o"

# External object files for target uFldTagManager
uFldTagManager_EXTERNAL_OBJECTS =

../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager.cpp.o
../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/TagManager_Info.cpp.o
../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/VTag.cpp.o
../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/main.cpp.o
../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/build.make
../bin/uFldTagManager: /home/pmp2149/moos-ivp/build/MOOS/MOOSCore/lib/libMOOS.a
../bin/uFldTagManager: src/uFldTagManager/CMakeFiles/uFldTagManager.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pmp2149/moos-ivp-aquaticus-pmp2149/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ../../../bin/uFldTagManager"
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uFldTagManager.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/uFldTagManager/CMakeFiles/uFldTagManager.dir/build: ../bin/uFldTagManager
.PHONY : src/uFldTagManager/CMakeFiles/uFldTagManager.dir/build

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/clean:
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager && $(CMAKE_COMMAND) -P CMakeFiles/uFldTagManager.dir/cmake_clean.cmake
.PHONY : src/uFldTagManager/CMakeFiles/uFldTagManager.dir/clean

src/uFldTagManager/CMakeFiles/uFldTagManager.dir/depend:
	cd /home/pmp2149/moos-ivp-aquaticus-pmp2149/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pmp2149/moos-ivp-aquaticus-pmp2149 /home/pmp2149/moos-ivp-aquaticus-pmp2149/src/uFldTagManager /home/pmp2149/moos-ivp-aquaticus-pmp2149/build /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager /home/pmp2149/moos-ivp-aquaticus-pmp2149/build/src/uFldTagManager/CMakeFiles/uFldTagManager.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/uFldTagManager/CMakeFiles/uFldTagManager.dir/depend

