INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

IF(WINDOWS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DCRACKERCOMMON_EXPORTS")
ENDIF()
	

ADD_LIBRARY( cracker-common SHARED
	BaseNInteger.cpp
	HashingAlgorithm.cpp
	NetworkSocket.cpp
	Thread.cpp
	UnitTests.cpp
)

IF(WIN32)
	TARGET_LINK_LIBRARIES( cracker-common
		ws2_32 )
ELSE()
	TARGET_LINK_LIBRARIES( cracker-common
		m pthread dl
	)
ENDIF()