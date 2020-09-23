find_path(EFUSION_INCLUDE_DIRS
	NAMES
		ElasticFusion.h
	PATHS
		~/lib/ElasticFusion/Core/src
		${CGAL_DIR_WIN32}/include
		/usr/local/include
		/opt/local/include
)

find_path(PANGOLIN_INCLUDE_DIRS
	NAMES
		pangolin/pangolin.h
	PATHS
		~/lib/Pangolin/include
		${CGAL_DIR_WIN32}/include
		/usr/local/include
		/opt/local/include
)

find_library(EFUSION_LIBRARY SHARED
	NAMES
		efusion
	PATHS
		~/lib/ElasticFusion/Core/src/build
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
)

find_library(PANGOLIN_LIBRARY SHARED
	NAMES
		pangolin
	PATHS
		~/lib/Pangolin/build/src
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
)
