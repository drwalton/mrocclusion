set(SDL_DIR "C:/local/SDL/SDL2_ttf-2.0.14/")

find_path(SDL_ttf_INCLUDE_DIRS
	NAMES
		SDL_ttf.h
	PATHS
		/usr/include/SDL2
		/usr/local/include/SDL2
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2_ttf-2.0.14/include
		F:/local/SDL/SDL2_ttf-2.0.14/include
		"${SDL_DIR}/include"
		/opt/local/include/SDL2
		/usr/local/include/SDL2
)

find_library(SDL_ttf_LIBRARIES
	NAMES
		SDL2_ttf
	PATHS
		/usr/lib
		/usr/local/lib
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2_ttf-2.0.14/lib/x64
		F:/local/SDL/SDL2_ttf-2.0.14/lib/x64
		"${SDL_DIR}/lib/x64"
		/Library/Frameworks
)

message("SDL2 Library: ${SDL_LIBRARIES}")

