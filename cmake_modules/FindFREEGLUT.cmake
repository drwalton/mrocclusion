find_path(FREEGLUT_INCLUDE_DIRS
	NAMES
		GL/freeglut.h
	PATHS
		C:/local/freeglut/freeglut-2.8.1/include
		F:/local/freeglut/freeglut-2.8.1/include
		G:/local/freeglut/freeglut-2.8.1/include
)

find_library(FREEGLUT_LIBRARIES
	NAMES
		freeglut
	PATHS
		C:/local/freeglut/freeglut-2.8.1/lib/x64
		F:/local/freeglut/freeglut-2.8.1/lib/x64
		G:/local/freeglut/freeglut-2.8.1/lib/x64
)

