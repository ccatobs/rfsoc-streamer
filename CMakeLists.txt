# Modeled after CMakeLists.txt in smurf-streamer
# Designed to be run in a Dockerfile - PATHS MAY NEED CHANGED TO WORK LOCALLY!
# ALSO NEED TO SET ENV VARIABLES!

# Check cmake version
cmake_minimum_required (VERSION 3.1)
include(InstallRequiredSystemLibraries)

# cmake policies -- best to keep these in sync with spt3g!
if(POLICY CMP0060) # Suppress cmake stripping full paths from libraries in some cases
  cmake_policy(SET CMP0060 NEW)
endif()
cmake_policy(SET CMP0012 NEW) # Allow use of true in boolean expressions
if(POLICY CMP0042) # Enable RPATH on OSX
   cmake_policy(SET CMP0042 NEW)
endif()

# Project name
project (RfsocStreamer)

# C/C++
enable_language(CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wno-deprecated -O3")

#####################################
# Link SPT3g Libraries
#####################################
set(SPT3G_DIR $ENV{SPT3G_SOFTWARE_PATH})
set(SO3G_DIR $ENV{SO3G_DIR})
link_directories(${SPT3G_DIR}/build/spt3g)

#####################################
# Find Support Libraries
#####################################
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS thread)

# #####################################
# # Find python3
# #####################################
# set(DO_PYTHON 0)
#
find_package(PythonInterp 3.6 REQUIRED)
find_package(PythonLibs 3.6 REQUIRED)

find_package (Python3 COMPONENTS Interpreter Development)

#####################################
# Setup build
#####################################
include_directories(/usr/local/include/)

# Include files
include_directories(${SPT3G_DIR}/core/include/core)
include_directories(${SPT3G_DIR}/core/include)
include_directories(${SPT3G_DIR}/core/src)
include_directories(${SO3G_DIR}/include)


include_directories(/usr/local/include/)
include_directories(${PROJECT_SOURCE_DIR}/include/)

# Set output to TOP/lib, remove lib prefix
set_target_properties(RfsocStreamer PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set_target_properties(RfsocStreamer PROPERTIES PREFIX "")
set_target_properties(RfsocStreamer PROPERTIES OUTPUT_NAME "ccatrfsoccore")

# Link to spt3g and so3g
TARGET_LINK_LIBRARIES(RfsocStreamer LINK_PUBLIC ${SPT3G_DIR}/build/spt3g/libspt3g-core.so)
TARGET_LINK_LIBRARIES(RfsocStreamer LINK_PUBLIC ${SO3G_DIR}/build/so3g/libso3g.so)