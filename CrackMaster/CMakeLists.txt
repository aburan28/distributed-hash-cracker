
INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

LINK_DIRECTORIES(../Cracker-common)

ADD_EXECUTABLE(master
	CommandLine.cpp
	CrackMaster.cpp
	MessageQueue.cpp
	Mutex.cpp
	Scheduler.cpp
	UnitTests.cpp
)

IF(UNIX)
	TARGET_LINK_LIBRARIES(master
		m pthread dl cracker-common rt
	)
ELSE()
	TARGET_LINK_LIBRARIES(master
		cracker-common
	)
ENDIF()
