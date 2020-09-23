set(CGAL_DIR_WIN32 C:/local/CGAL/CGAL-4.8.1/)

find_path(CGAL_INCLUDE_DIRS
	NAMES
		CGAL/CORE/CORE.h
	PATHS
		${CGAL_DIR_WIN32}/include
		/usr/local/include
		/opt/local/include
		F:/local/CGAL/CGAL-4.9/include
)

find_library(CGAL_LIBRARY
	NAMES
		CGAL-vc140-mt-4.9.lib
		CGAL
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

find_library(CGAL_CORE_LIBRARY
	NAMES
		CGAL_Core-vc140-mt-4.9.lib
		CGAL_Core
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

if(NOT APPLE)
find_library(CGAL_ImageIO_LIBRARY
	NAMES
		CGAL_ImageIO-vc140-mt-4.9.lib
		CGAL_ImageIO
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)
endif(NOT APPLE)

find_library(CGAL_LIBRARY_DEBUG
	NAMES
		CGAL-vc140-mt-gd-4.9.lib
		CGAL
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

find_library(CGAL_CORE_LIBRARY_DEBUG
	NAMES
		CGAL_Core-vc140-mt-gd-4.9.lib
		CGAL_Core
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

if(NOT APPLE)
find_library(CGAL_ImageIO_LIBRARY_DEBUG
	NAMES
		CGAL_ImageIO-vc140-mt-gd-4.9.lib
		CGAL_ImageIO
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

find_library(GMP_LIBRARY
	NAMES
		libgmp-10.lib
		gmp
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)

find_library(MPFR_LIBRARY
	NAMES
		libmpfr-4.lib
		mpfr
	PATHS
		${CGAL_DIR_WIN32}/lib
		/usr/local/lib
		/opt/local/lib
		F:/local/CGAL/CGAL-4.9/build/lib/
)
endif(NOT APPLE)

if(NOT APPLE)
set(CGAL_LIBRARIES
	${CGAL_LIBRARY}
	${CGAL_CORE_LIBRARY}
	${CGAL_ImageIO_LIBRARY}
	${GMP_LIBRARY}
	${MPFR_LIBRARY}
)
set(CGAL_LIBRARIES_DEBUG
	${CGAL_LIBRARY_DEBUG}
	${CGAL_CORE_LIBRARY_DEBUG}
	${CGAL_ImageIO_LIBRARY_DEBUG}
	${GMP_LIBRARY}
	${MPFR_LIBRARY}
)
else(NOT APPLE)
set(CGAL_LIBRARIES
	${CGAL_LIBRARY}
	${CGAL_CORE_LIBRARY}
)

set(CGAL_LIBRARIES_RELEASE ${CGAL_LIBRARIES})
set(CGAL_LIBRARIES_DEBUG
	${CGAL_LIBRARY_DEBUG}
	${CGAL_CORE_LIBRARY_DEBUG}
)
endif(NOT APPLE)
