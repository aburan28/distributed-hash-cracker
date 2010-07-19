SET(ASM_OUTPUTS "")
IF(AMD64)
	FOREACH(SRC
		search.asm
		)
		
		#Get some file names
		STRING( REGEX REPLACE "\\.asm" ".o" DEST ${SRC})
		
		#Compilation command
		ADD_CUSTOM_COMMAND(
			OUTPUT ${DEST}
			COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} -o ${CMAKE_CURRENT_BINARY_DIR}/${DEST}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
			COMMENT "Building asm object ${DEST}"
			)

		#Add to list of outputs
		SET(ASM_OUTPUTS ${ASM_OUTPUTS} ${DEST})
	ENDFOREACH()
ENDIF()


###############################################################################
#C++ compilation and linking
ADD_EXECUTABLE(mpicrack
	
	${ASM_OUTPUTS}
	main.cpp
)

TARGET_LINK_LIBRARIES(mpicrack
	m ${MPI_LIBRARIES} rt)
