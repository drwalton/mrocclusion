set(SDL_LIB_DIR_SEARCH
	"C:/local/SDL/SDL2-2.0.4/lib/x64"
	"F:/local/SDL/SDL2-2.0.4/lib/x64"
	"G:/local/SDL/SDL2-2.0.4/lib/x64"
)
set(SDL_INCLUDE_DIR_SEARCH
	"C:/local/SDL/SDL2-2.0.4/include"
	"F:/local/SDL/SDL2-2.0.4/include"
	"G:/local/SDL/SDL2-2.0.4/include"
)

find_path(SDL_INCLUDE_DIRS
	NAMES
		SDL.h
	PATHS
		/usr/include/SDL2
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/include
		${SDL_INCLUDE_DIR_SEARCH}
		/opt/local/include/SDL2
		/usr/local/include/SDL2
)

set(SDL2_DIR "$(SDL_INCLUDE_DIRS)/..")

find_library(SDL_LIBRARY NAMES
		SDL2
	PATHS
		/usr/lib
		/usr/local/lib
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/lib/x64
		${SDL_LIB_DIR_SEARCH}
		/Library/Frameworks
)

find_library(SDL_MAIN_LIBRARY NAMES
		SDL2Main
		SDL2main
	PATHS
		$ENV{WIN_LOCAL_DIR}/SDL/SDL2-2.0.4/lib/x64
		${SDL_LIB_DIR_SEARCH}
		/Library/Frameworks
)

set(SDL_LIBRARIES
	${SDL_LIBRARY}
	${SDL_MAIN_LIBRARY}
)

message("SDL2 Library: ${SDL_LIBRARIES}")
