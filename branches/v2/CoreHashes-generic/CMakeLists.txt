INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

IF(WINDOWS)
	SET(BIN_FORMAT win32)
ELSEIF(AMD64)
	SET(BIN_FORMAT elf64)
ELSE()
	SET(BIN_FORMAT elf)
ENDIF()

IF(MACOSX)
 SET(BIN_FORMAT macho)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PIC_FLAG}"  )
SET(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${PIC_FLAG}"  )

IF(X86)

ADD_CUSTOM_COMMAND(
   OUTPUT MD5_x86.o
   COMMAND nasm -f ${BIN_FORMAT} ${CMAKE_CURRENT_SOURCE_DIR}/MD5_x86.asm -o ${CMAKE_CURRENT_BINARY_DIR}/MD5_x86.o
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/MD5_x86.asm
   )

ADD_LIBRARY(corehashes-generic SHARED
	CoreHashes-generic.cpp
	MD5.cpp
	MD5crypt.cpp
	SHA1.cpp
	UnitTests.cpp
	${CMAKE_CURRENT_BINARY_DIR}/MD5_x86.o
)

ELSEIF(AMD64)

ADD_CUSTOM_COMMAND(
   OUTPUT MD5_amd64.o
   COMMAND nasm -f ${BIN_FORMAT} ${CMAKE_CURRENT_SOURCE_DIR}/MD5_amd64.asm -o ${CMAKE_CURRENT_BINARY_DIR}/MD5_amd64.o
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/MD5_amd64.asm
   )

ADD_LIBRARY(corehashes-generic SHARED
	CoreHashes-generic.cpp
	MD5.cpp
	MD5crypt.cpp
	SHA1.cpp
	UnitTests.cpp
	${CMAKE_CURRENT_BINARY_DIR}/MD5_amd64.o
	)

ELSE()

ADD_LIBRARY(corehashes-generic SHARED
	CoreHashes-generic.cpp
	MD5.cpp
	MD5crypt.cpp
	SHA1.cpp
	UnitTests.cpp
)

ENDIF()

IF(UNIX)
	TARGET_LINK_LIBRARIES(corehashes-generic
		m pthread dl cracker-common
	)
ELSE()
	TARGET_LINK_LIBRARIES(corehashes-generic
		cracker-common
	)
ENDIF()


IF(SELINUX)
ADD_CUSTOM_TARGET (corehashes-selinux-permissions ALL)
ADD_CUSTOM_COMMAND(
	SOURCE corehashes-selinux-permissions
	TARGET corehashes-selinux-permissions
	COMMAND chcon -t textrel_shlib_t ${CMAKE_CURRENT_BINARY_DIR}/libcorehashes-generic.so
	DEPENDS corehashes-generic)
ENDIF()

# TODO
MESSAGE(STATUS
	"null.spu and md5.spu are not produced yet!"
)
