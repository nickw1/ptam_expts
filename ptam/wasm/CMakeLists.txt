# Based on cmake file from:
# https://github.com/mpizenberg/emscripten-opencv
cmake_minimum_required( VERSION 3.1 )
project( ptam_wasm )

# Use C++ 11 by default
# Now use C++ 14, I think some of the PTAM code needs it
set( CMAKE_CXX_STANDARD 14 )

set( opencvdir "/home/nick/src/opencv" )
set( ptamdir "/home/nick/src/ptam_plus" )
# Set Release as default build type - not now, we want debug mode
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif(NOT CMAKE_BUILD_TYPE)

# Does not work
# find_package( OpenCV REQUIRED PATHS third-party/opencv-4.1.0/build_wasm NO_DEFAULT_PATH)

# Needed for opencv2/opencv.hpp
include_directories( "${opencvdir}/include" )

# Needed by opencv.hpp for opencv2/opencv_modules.hpp
include_directories( "${opencvdir}//build_wasm" )

# Needed by opencv_modules.hpp for every module
file( GLOB opencv_include_modules "${opencvdir}/modules/*/include" )
include_directories( ${opencv_include_modules} ${ptamdir} "${ptamdir}/external" "${ptamdir}/external/libcvd" "${ptamdir}/external/gvars-3.0" )

add_compile_options("-fexceptions")
# not needd if EMSCRIPTEN_KEEPALIVE is included
#add_definitions("-s EXPORTED_FUNCTIONS='[\"_receiveData\",\"_test1\"]'")
add_definitions("-s NO_DISABLE_EXCEPTION_CATCHING")
# Our hello world executable
add_executable( ptam_wasm ptam_wasm.cpp )

# Link to opencv.js precompiled libraries
file( GLOB opencv_js "${opencvdir}/build_wasm/lib/*.a" )
set( ptam_lib "${ptamdir}/ptam/libPTAM.a" )
file( GLOB_RECURSE ptam_libs_external "${ptamdir}/external/*.a" )
#file( GLOB ptam_libs_opencv "${ptamdir}/opencv-3*/build-opencv/3rdparty/lib/*.a")
 
target_link_libraries( ptam_wasm ${ptam_libs_external} ${ptam_lib} ${opencv_js} )
