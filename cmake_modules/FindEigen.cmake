find_path(EIGEN_INCLUDE_DIRS
		Eigen/Core
	HINTS
		"$ENV{EIGEN_ROOT}"
	PATHS
		"$ENV{EIGEN_ROOT}"
		/opt/local/include/eigen3
		/usr/local/include/eigen3
		F:/local/eigen/eigen-3.2.9/
	PATH_SUFFIXES
		eigen3
		include/eigen3
		include
)

message("EIGEN ROOT ENV: $ENV{EIGEN_ROOT}")
