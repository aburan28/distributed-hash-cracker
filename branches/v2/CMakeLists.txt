CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( Cracker )

INCLUDE (CheckLibraryExists)
INCLUDE (CheckFunctionExists)  
INCLUDE (CheckCXXSourceCompiles)

# Some settings we'd like CMake to adhere to
#SET( CMAKE_CURRENT_BINARY_DIR build )
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET( LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
IF(UNIX)
	SET( CMAKE_CXX_FLAGS_DEBUG "-g3" )
ENDIF()

#Set platform-specific flags
IF(UNIX)
	SET( PIC_FLAG "-fPIC")
ELSE()
	SET( PIC_FLAG "")
ENDIF()

#Try to figure out what CPU we have
SET(AMD64 0)
SET(X86 0)
MESSAGE(STATUS "Autodetecting CPU type")
IF(UNIX)
	IF("$ENV{TARGET_ARCH}" STREQUAL "X86")
		MESSAGE(STATUS "CPU type overridden by TARGET_ARCH environment variable - x86")
		SET( ARCH_SWITCH "-m32")
		SET( X86 1)	
	ELSEIF("$ENV{TARGET_ARCH}" STREQUAL "AMD64")
		MESSAGE(STATUS "CPU type overridden by TARGET_ARCH environment variable - amd64")
		SET( ARCH_SWITCH "-m64")
		SET( AMD64 1)	
	ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
		MESSAGE(STATUS "Autodetecting CPU type - amd64")
		SET( ARCH_SWITCH "-m64")
		SET( AMD64 1)
	ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
		MESSAGE(STATUS "Autodetecting CPU type - x86")
		SET( ARCH_SWITCH "-m32")
		SET(X86 1)
	ELSE()
		MESSAGE(ERROR "Could not autodetect CPU type!")
	ENDIF()
ELSE()
	#Windows only has 32 bit builds atm
	MESSAGE(STATUS "Autodetecting CPU type - x86")
	SET( ARCH_SWITCH "/EHsc")
	SET(X86 1)
ENDIF()

#See if we have NASM installed
MESSAGE(STATUS "Check for NASM")
FIND_PROGRAM(NASM NAMES nasm PATH /usr/bin) 
IF (NASM)
	SET(NASMFOUND 1)
	MESSAGE(STATUS "Check for NASM - found")
ELSE()
	SET(NASMFOUND 0)
	MESSAGE(STATUS "Check for NASM - not found")
ENDIF()

#See if we have CUDA installed
MESSAGE(STATUS "Check for CUDA")
FIND_PROGRAM(NVCC NAMES nvcc PATH /usr/local/cuda/bin) 
IF (NVCC)
	SET(CUDA_ENABLED 1)
	MESSAGE(STATUS "Check for NVCC - found")
ELSE()
	SET(CUDA_ENABLED 0)
	MESSAGE(STATUS "Check for NVCC - not found")
ENDIF()

#See if CUDA is set up for the current architecture
MESSAGE(STATUS "Checking CUDA configuration")
IF(CUDA_ENABLED)
	SET(CMAKE_REQUIRED_FLAGS ${ARCH_SWITCH})
	SET(CMAKE_REQUIRED_LIBRARIES "cuda")
	CHECK_CXX_SOURCE_COMPILES("#include <cuda.h>\nint main(int argc, char* argv[]) { cuMemcpy2D(0); return 0; }" CUDA_COMPILES)
	
	IF(CUDA_COMPILES)
	ELSE()
		MESSAGE(STATUS "CUDA test code failed to compile - disabling CUDA.")
		MESSAGE(STATUS "Check that you have the appropriate 32/64 bit version of the CUDA libraries installed.")
		SET(CUDA_ENABLED 0)
	ENDIF()
ENDIF()


SET( CMAKE_CXX_FLAGS ${ARCH_SWITCH})

SET( OS_SWITCH "")

# Determine system type
SET(SELINUX 0)
INCLUDE( CheckFunctionExists )

IF( UNIX )
	CHECK_FUNCTION_EXISTS( clock_gettime LINUX )
	IF( !LINUX )
		CHECK_FUNCTION_EXISTS( gettimeofday BSD )
		SET(BSD 1)
	ELSE()
		SET(LINUX 1)
		MESSAGE(STATUS "Check for SELinux")
		FIND_PROGRAM(SELINUX_PATH "/usr/sbin/selinuxenabled")
		IF(SELINUX_PATH)
			MESSAGE(STATUS "Check for SELinux - found")
			SET(SELINUX 1)
		ELSE()
			MESSAGE(STATUS "Check for SELinux - not found")
			SET(SELINUX 0)
		ENDIF()
	ENDIF( !LINUX )
ENDIF( UNIX )

IF( APPLE )
	SET( MACOSX 1 )
	SET( OS_SWITCH "read_only_relocs suppress")
ENDIF( APPLE )

IF( WIN32)
	SET( WINDOWS 1 )
ENDIF()

CONFIGURE_FILE(config.h.in ${CMAKE_BINARY_DIR}/config.h)

SET( CMAKE_CXX_FLAGS ${ARCH_SWITCH} ${OS_SWTICH})

# Build
ADD_SUBDIRECTORY( Cracker-common )

ADD_SUBDIRECTORY( CoreHashes-generic )
IF(CUDA_ENABLED)
	ADD_SUBDIRECTORY( CoreHashes-CUDA )
ELSE()
	MESSAGE(STATUS "CUDA disabled, skipping CoreHashes-CUDA")
ENDIF()

ADD_SUBDIRECTORY( CrackThread-generic )
IF(CUDA_ENABLED)
	ADD_SUBDIRECTORY( CrackThread-CUDA )
ELSE()
	MESSAGE(STATUS "CUDA disabled, skipping CrackThread-CUDA")
ENDIF()

ADD_SUBDIRECTORY( CrackMaster )
ADD_SUBDIRECTORY( ComputeNode )

ADD_SUBDIRECTORY( NodeLauncher )

ADD_SUBDIRECTORY( UnitTests )
