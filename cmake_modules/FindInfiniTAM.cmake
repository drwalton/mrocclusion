find_path(InfiniTAM_INCLUDE_DIR
	NAMES
		ITMLib.h
	PATHS
		~/lib/InfiniTAM/InfiniTAM/ITMLib/
		C:/local/InfiniTAM/InfiniTAM/ITMLib/
		G:/local/InfiniTAM/InfiniTAM/ITMLib/
		F:/local/InfiniTAM/InfiniTAM/ITMLib/
)

find_library(InfiniTAM_MAIN_LIB
	NAMES
		ITMLib
	PATHS
		~/lib/InfiniTAM/InfiniTAM/build/ITMLib/
		C:/local/InfiniTAM/x64/Release/
		G:/local/InfiniTAM/x64/Release/
		F:/local/InfiniTAM/InfiniTAM/build/ITMLib/Release
)

if(NOT WIN32)
find_library(InfiniTAM_UTIL_LIB
	NAMES
		Utils
	PATHS
		~/lib/InfiniTAM/InfiniTAM/build/Utils/
		C:/local/InfiniTAM/InfiniTAM/build/Utils/
)

find_library(InfiniTAM_ORUTIL_LIB
	NAMES
		ORUtils
	PATHS
		~/lib/InfiniTAM/InfiniTAM/build/ORUtils/
		C:/local/InfiniTAM/InfiniTAM/build/ORUtils/
)

find_library(InfiniTAM_ENGINE_LIB
	NAMES
		Engine
	PATHS
		~/lib/InfiniTAM/InfiniTAM/build/Engine/
		C:/local/InfiniTAM/InfiniTAM/build/Engine/
)
endif(NOT WIN32)

find_library(InfiniTAM_MAIN_LIB_DEBUG
	NAMES
		ITMLib
	PATHS
	~/lib/InfiniTAM/InfiniTAM/xcode/ITMLib/Debug
		C:/local/InfiniTAM/x64/Debug
		G:/local/InfiniTAM/x64/Debug
		F:/local/InfiniTAM/InfiniTAM/build/ITMLib/Debug
)

if(APPLE)
find_library(InfiniTAM_UTIL_LIB_DEBUG
	NAMES
		Utils
	PATHS
	~/lib/InfiniTAM/InfiniTAM/xcode/Utils/Debug
	C:/local/InfiniTAM/InfiniTAM/build/Utils/Debug
)

find_library(InfiniTAM_ORUTIL_LIB_DEBUG
	NAMES
		ORUtils
	PATHS
	~/lib/InfiniTAM/InfiniTAM/xcode/ORUtils/Debug
	C:/local/InfiniTAM/InfiniTAM/build/ORUtils/Debug
)

find_library(InfiniTAM_ENGINE_LIB_DEBUG
	NAMES
		Engine
	PATHS
	~/lib/InfiniTAM/InfiniTAM/xcode/Engine/Debug
	C:/local/InfiniTAM/InfiniTAM/build/Engine/Debug
)
endif(APPLE)

set(InfiniTAM_LIBRARIES 
	${InfiniTAM_MAIN_LIB}
	${InfiniTAM_UTIL_LIB}
	${InfiniTAM_ORUTIL_LIB}
	${InfiniTAM_ENGINE_LIB}
	)

if(UNIX) 
	set(InfiniTAM_LIBRARIES_DEBUG ${InfiniTAM_LIBRARIES})
else(UNIX)
	set(InfiniTAM_LIBRARIES_DEBUG
		${InfiniTAM_MAIN_LIB_DEBUG}
		${InfiniTAM_UTIL_LIB_DEBUG}
		${InfiniTAM_ORUTIL_LIB_DEBUG}
		${InfiniTAM_ENGINE_LIB_DEBUG}
		)
endif(UNIX)

set(INFINITAM_INCLUDE_DIR ${InfiniTAM_INCLUDE_DIR})
set(INFINITAM_INCLUDE_DIRS ${InfiniTAM_INCLUDE_DIR})
set(InfiniTAM_INCLUDE_DIRS ${InfiniTAM_INCLUDE_DIR})

set(INFINITAM_LIBRARIES ${InfiniTAM_LIBRARIES})
set(INFINITAM_LIBRARY ${InfiniTAM_LIBRARIES})
set(InfiniTAM_LIBRARY ${InfiniTAM_LIBRARIES})

set(INFINITAM_LIBRARIES_DEBUG ${InfiniTAM_LIBRARIES_DEBUG})
set(INFINITAM_LIBRARY_DEBUG ${InfiniTAM_LIBRARIES_DEBUG})
set(InfiniTAM_LIBRARY_DEBUG ${InfiniTAM_LIBRARIES_DEBUG})

