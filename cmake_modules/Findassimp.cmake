find_path(ASSIMP_INCLUDE_DIRS
	NAMES
		assimp/Importer.hpp
	PATHS
		$ENV{WIN_LOCAL_DIR}/assimp/include
		"C:/Program Files/Assimp/include"
		"F:/Program Files/Assimp/include"
)

find_library(ASSIMP_LIBRARIES
	NAMES
		assimp
	PATHS
		$ENV{WIN_LOCAL_DIR}/assimp/assimp-3.2/lib/
		"C:/Program Files/Assimp/lib/x64"
		"F:/Program Files/Assimp/lib/x64"
)

message("Found ASSIMP: LIB: ${ASSIMP_LIBRARIES} INCLUDE: ${ASSIMP_INCLUDE_DIRS}")
