find_path(CERES_INCLUDE_DIRS
	NAMES
		ceres/ceres.h
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres-windows/ceres-solver/include
		"G:/local/ceres-windows/ceres-solver/include"
		"F:/local/ceres-windows/ceres-solver/include"
)

find_path(GLOG_INCLUDE_DIR
	NAMES
		glog/logging.h
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres-windows/glog/src/windows
		"G:/local/ceres-windows/glog/src/windows"
		"F:/local/ceres-windows/glog/src/windows"
)

find_path(CERES_CONFIG_DIR
	NAMES
		ceres/internal/config.h
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres-windows/win/include
		"G:/local/ceres-windows/win/include"
		"F:/local/ceres-windows/win/include"
)

set(CERES_INCLUDE_DIRS
	${CERES_INCLUDE_DIRS} ${GLOG_INCLUDE_DIR} ${CERES_CONFIG_DIR})

find_library(CERES_LIBRARIES_RELEASE
	NAMES
		ceres
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres-windows/x64/Release/
		"F:/local/ceres-windows/x64/Release/"
		"G:/local/ceres-windows/x64/Release/"
)

find_library(CERES_LIBRARIES_DEBUG
	NAMES
		ceres
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres/x64/Debug/
		"F:/local/ceres-windows/x64/Debug/"
		"G:/local/ceres-windows/x64/Debug/"
)

find_library(GLOG_LIBRARY_RELEASE
	NAMES
		libglog_static
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres/x64/Release/
		"F:/local/ceres-windows/x64/Release/"
		"G:/local/ceres-windows/x64/Release/"
)
find_library(GLOG_LIBRARY_DEBUG
	NAMES
		libglog_static
	PATHS
		$ENV{WIN_LOCAL_DIR}/ceres/x64/Debug/
		"F:/local/ceres-windows/x64/Debug/"
		"G:/local/ceres-windows/x64/Debug/"
)

set(CERES_LIBRARIES
	optimized
	${GLOG_LIBRARY_RELEASE}
	debug
	${GLOG_LIBRARY_DEBUG}
	optimized
	${CERES_LIBRARIES_RELEASE}
	debug
	${CERES_LIBRARIES_DEBUG}
)

set(CERES_LIBRARIES_RELEASE
	${GLOG_LIBRARY_RELEASE}
	${CERES_LIBRARIES_RELEASE}
)
set(CERES_LIBRARIES_DEBUG
	${GLOG_LIBRARY_DEBUG}
	${CERES_LIBRARIES_DEBUG}
)

if(CERES_LIBRARIES_DEBUG)
	set(CERES_FOUND on)
endif(CERES_LIBRARIES_DEBUG)

message("Found CERES: LIB: ${CERES_LIBRARIES} INCLUDE: ${CERES_INCLUDE_DIRS}")
