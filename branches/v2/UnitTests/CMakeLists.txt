INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

LINK_DIRECTORIES(../Cracker-common)

ADD_EXECUTABLE(unittests
	Main.cpp
	Plugin.cpp
)

IF(WIN32)
	TARGET_LINK_LIBRARIES(unittests
		
	)
ELSE()
	TARGET_LINK_LIBRARIES(unittests
		m pthread dl
	)
ENDIF()