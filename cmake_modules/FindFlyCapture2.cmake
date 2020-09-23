find_path(FLYCAPTURE2_INCLUDE_DIRS
	NAMES Camera.h
	PATHS
		"G:/Program Files/Point Grey Research/FlyCapture2/include"
		"C:/Program Files/Point Grey Research/FlyCapture2/include"
)

find_library(FLYCAPTURE2_LIBRARIES
	NAMES
		FlyCapture2_v140
	PATHS
		"G:/Program Files/Point Grey Research/FlyCapture2/lib64/vs2015"
		"C:/Program Files/Point Grey Research/FlyCapture2/lib64/vs2015"
)

if(NOT FLYCAPTURE2_LIBRARIES)
	set(FLYCAPTURE2_FOUND FALSE)
else()
	set(FLYCAPTURE2_FOUND TRUE)
endif()

