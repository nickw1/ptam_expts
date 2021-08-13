# Based on cmake file from:
# https://github.com/mpizenberg/emscripten-opencv
cmake_minimum_required( VERSION 3.12 )
project( ptam_wasm )

# Use C++ 11 by default
# Now use C++ 14, I think some of the PTAM code needs it
set( CMAKE_CXX_STANDARD 14 )

set( opencvdir "/home/nick/src/opencv" )
set( ptamdir "/home/nick/src/ptam_plus" )


# Needed for opencv2/opencv.hpp
include_directories( "${opencvdir}/include" )

# Needed by opencv.hpp for opencv2/opencv_modules.hpp
include_directories( "${opencvdir}//build_wasm" )

# Needed by opencv_modules.hpp for every module
file( GLOB opencv_include_modules "${opencvdir}/modules/*/include" )
include_directories( ${opencv_include_modules} ${ptamdir} "${ptamdir}/external" "${ptamdir}/external/libcvd" "${ptamdir}/external/gvars-3.0" )

add_compile_options("-fexceptions")
add_compile_options("-pthread")
#add_compile_options("-s PTHREAD_POOL_SIZE_STRICT=2")
#add_link_options("-s PTHREAD_POOL_SIZE_STRICT=2")
add_link_options("SHELL: -s LLD_REPORT_UNDEFINED")
add_link_options("-fexceptions")
#add_link_options("-s INITIAL_MEMORY=32000000")
add_link_options("SHELL: -s PTHREAD_POOL_SIZE=4")
#add_link_options("SHELL: -s ALLOW_MEMORY_GROWTH=1")
add_link_options("SHELL: -s INITIAL_MEMORY=512mb")
add_link_options("-pthread")
# the below is useful to debug pointer errors but doesn't seem to link properly
# https://emscripten.org/docs/debugging/Sanitizers.html 
#add_link_options("-fsanitize=address")
add_link_options("--bind")
#add_compile_options("-O3")
#add_link_options("-s LLD_REPORT_UNDEFINED")

# Our hello world executable
add_executable( ptam_wasm ptam_wasm.cpp )

# Link to opencv.js precompiled libraries
file( GLOB opencv_js "${opencvdir}/build_wasm/lib/*.a" )
set( ptam_lib "${ptamdir}/ptam/libPTAM.a" )
#file( GLOB_RECURSE ptam_libs_external "${ptamdir}/external/*.a" )
target_link_libraries(ptam_wasm ${ptam_lib} "${ptamdir}/external/libcvd/liblibcvd.a" "${ptamdir}/external/gvars-3.0/libgvars.a" "${ptamdir}/external/clapack-3.2.1/SRC/liblapack.a" "${ptamdir}/external/clapack-3.2.1/BLAS/SRC/libblas.a" "${ptamdir}/external/clapack-3.2.1/F2CLIBS/libf2c/libf2c.a" ${opencv_js} "-lpthread")
#target_link_libraries(ptam_wasm -L${ptamdir}/ptam -L${ptamdir}/external/libcvd -L${ptamdir}/external/gvars-3.0 -L${ptamdir}/external/clapack-3.2.1/SRC -L${ptamdir}/external/clapack-3.2.1/BLAS/SRC -L${ptamdir}/external/clapack-3.2.1/F2CLIBS/libf2c  -lPTAM -llibcvd -lgvars -llapack -lblas -lf2c)
