find_path(ARTOOLKITPLUS_INCLUDE_DIRS
	NAMES
		ARToolKitPlus/ar.h
	PATHS
		~/lib/artoolkitplus/include
		$ENV{WIN_LOCAL_DIR}/artoolkitplus/include
		"C:/local/artoolkitplus/include"
		"F:/local/artoolkitplus/include"
		"G:/local/artoolkitplus/include"
)

find_library(ARTOOLKITPLUS_LIBRARIES
	NAMES
		ARToolKitPlus
	PATHS
		~/lib/artoolkitplus/build/lib
		"C:/local/artoolkitplus/build/lib/Release"
		"F:/local/artoolkitplus/build/lib/Release"
		"G:/local/artoolkitplus/build/lib/Release"
		$ENV{WIN_LOCAL_DIR}/artoolkitplus/build/lib/Release
)

find_library(ARTOOLKITPLUS_LIBRARIES_DEBUG
	NAMES
		ARToolKitPlus
	PATHS
		~/lib/artoolkitplus/build/lib
		"C:/local/artoolkitplus/build/lib/Debug"
		"F:/local/artoolkitplus/build/lib/Debug"
		"G:/local/artoolkitplus/build/lib/Debug"
		$ENV{WIN_LOCAL_DIR}/artoolkitplus/build/lib/Debug
)

