find_path(GMP_INCLUDE_DIR
	NAMES
		gmp.h
	PATHS
		C:/local/gmp/include
		F:/local/CGAL/CGAL-4.9/auxiliary/gmp/include/
)

find_library(GMP_LIBRARY
	NAMES
		gmp.lib
		libgmp-10.lib
	PATHS
		C:/local/gmp/lib
		F:/local/CGAL/CGAL-4.9/auxiliary/gmp/lib/
)

find_library(GMP_LIBRARY_DEBUG
	NAMES
		gmpDebug.lib
		libgmp-10.lib
	PATHS
		C:/local/gmp/lib
		F:/local/CGAL/CGAL-4.9/auxiliary/gmp/lib/
)
