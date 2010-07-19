###############################################################################
#CMake build script for RPISEC DEFCON hash cracker

###############################################################################
#Core setup
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( cracker )

INCLUDE (CheckCXXSourceCompiles)
INCLUDE (FindMPI)

###############################################################################
#Build settings - configurable
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET( LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

SET(EXTRA_WARNINGS "-Wcast-align -Winit-self -Wmissing-declarations")
SET(EXTRA_WARNINGS "${EXTRA_WARNINGS} -Wswitch -Wunsafe-loop-optimizations -Wwrite-strings ")
SET(WARNINGS "-Wall -Wextra -pedantic ${EXTRA_WARNINGS}")
SET( CMAKE_CXX_FLAGS "${WARNINGS} ${MPI_COMPILE_FLAGS} -O3 -march=opteron-sse3 -mtune=opteron-sse3 -m64")
SET( CMAKE_LD_FLAGS "-m64 ${MPI_LINK_FLAGS}")

#Grab a config.h if we have one
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${MPI_INCLUDE_PATH})

###############################################################################
#Enable PIC compilation for C++
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
SET(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fPIC"  )

ADD_SUBDIRECTORY(src)
