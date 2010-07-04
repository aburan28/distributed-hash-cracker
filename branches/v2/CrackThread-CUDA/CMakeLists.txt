INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
   OUTPUT CrackThread.o
   COMMAND nvcc -c ${CMAKE_CURRENT_SOURCE_DIR}/CrackThread.cu -o ${CMAKE_CURRENT_BINARY_DIR}/CrackThread.o -I ${CMAKE_BINARY_DIR} -Xcompiler ${PIC_FLAG} ${ARCH_SWITCH}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CrackThread.cu ${CMAKE_CURRENT_SOURCE_DIR}/CrackThread-CUDA.h
   )

ADD_LIBRARY(crackthread-cuda SHARED
	UnitTests.cpp
	CrackThread.o
)

IF(UNIX)
	TARGET_LINK_LIBRARIES(crackthread-cuda
		m pthread dl cuda /usr/local/cuda/lib/libcudart.so)
ELSE()
	TARGET_LINK_LIBRARIES(crackthread-cuda
		cuda cudart)
ENDIF()