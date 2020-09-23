find_path(MPFR_INCLUDE_DIRS
	NAMES
		mpfr.h
	PATHS
		/usr/local/include
		F:/local/CGAL/CGAL-4.9/auxiliary/gmp/include
)

find_library(MPFR_LIBRARIES
	NAMES
		mpfr
		libmpfr-4.lib
	PATHS
		/usr/local/lib
		F:/local/CGAL/CGAL-4.9/auxiliary/gmp/lib/
)

set(MPFR_LIBRARY ${MPFR_LIBRARIES})

