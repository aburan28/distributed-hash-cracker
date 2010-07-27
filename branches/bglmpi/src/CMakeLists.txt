###############################################################################
#C++ compilation and linking
ADD_EXECUTABLE(mpicrack
	BaseN.cpp
	MD5.cpp	
	benchmarks.cpp
	main.cpp
)

TARGET_LINK_LIBRARIES(mpicrack
	m ${MPI_LIBRARIES} rt)
