INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

LINK_DIRECTORIES( ../Cracker-common )

ADD_EXECUTABLE( computenode
	CommandLine.cpp
	ComputeNode.cpp
	DirList.cpp
	Plugin.cpp
	UnitTests.cpp
)

IF(UNIX)
	TARGET_LINK_LIBRARIES( computenode
		m pthread dl cracker-common
	)
ELSE()
	TARGET_LINK_LIBRARIES( computenode
		cracker-common
	)
ENDIF()
