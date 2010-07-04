INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(
   OUTPUT CoreHashes-CUDA.o
   COMMAND nvcc -c "${CMAKE_CURRENT_SOURCE_DIR}/CoreHashes-CUDA.cu" -o ${CMAKE_CURRENT_BINARY_DIR}/CoreHashes-CUDA.o -I ${CMAKE_BINARY_DIR} -Xcompiler ${PIC_FLAG} ${ARCH_SWITCH}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CoreHashes-CUDA.cu ${CMAKE_CURRENT_SOURCE_DIR}/CoreHashes-CUDA.h
   )
   
ADD_CUSTOM_COMMAND(
   OUTPUT md5.o
   COMMAND nvcc -c "${CMAKE_CURRENT_SOURCE_DIR}/md5.cu" -o ${CMAKE_CURRENT_BINARY_DIR}/md5.o -I ${CMAKE_BINARY_DIR} -Xcompiler  ${PIC_FLAG} ${ARCH_SWITCH}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/md5.cu ${CMAKE_CURRENT_SOURCE_DIR}/md5_kernel.cu
   )
   
ADD_CUSTOM_COMMAND(
   OUTPUT sha1.o
   COMMAND nvcc -c "${CMAKE_CURRENT_SOURCE_DIR}/sha1.cu" -o ${CMAKE_CURRENT_BINARY_DIR}/sha1.o -I ${CMAKE_BINARY_DIR} -Xcompiler ${PIC_FLAG} ${ARCH_SWITCH}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sha1.cu ${CMAKE_CURRENT_SOURCE_DIR}/sha1_kernel.cu
   )

ADD_LIBRARY(corehashes-cuda SHARED
	UnitTests.cpp
	CoreHashes-CUDA.o
	md5.o
	sha1.o
)

IF(UNIX)
	TARGET_LINK_LIBRARIES(corehashes-cuda
		m pthread dl cracker-common cuda /usr/local/cuda/lib/libcudart.so
	)
ELSE()
	TARGET_LINK_LIBRARIES(corehashes-cuda
		cracker-common cuda cudart
	)
ENDIF()
