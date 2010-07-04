INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

IF(UNIX)
	ADD_LIBRARY( crackthread-generic SHARED
		CrackThread-generic.cpp
		CrackThread-generic.h
		UnitTests.cpp
	)

	TARGET_LINK_LIBRARIES( crackthread-generic
		m pthread dl cracker-common
	)
ELSE()
	ADD_LIBRARY( crackthread-generic SHARED
		CrackThread-generic.cpp
		CrackThread-generic.h
		UnitTests.cpp
		dllmain.cpp
	)

	TARGET_LINK_LIBRARIES( crackthread-generic
		cracker-common
	)
	
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PIC_FLAG} /DCRACKTHREAD_GENERIC_EXPORTS"  )
ENDIF()