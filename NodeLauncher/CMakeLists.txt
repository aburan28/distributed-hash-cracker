INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

LINK_DIRECTORIES(../Cracker-common)

ADD_EXECUTABLE( nodelauncher
	NodeLauncher.cpp
	XmlParser.cpp
)

IF(UNIX)
	TARGET_LINK_LIBRARIES(nodelauncher
		m pthread dl cracker-common
	)
ELSE()
	TARGET_LINK_LIBRARIES(nodelauncher
		cracker-common
	)
ENDIF()