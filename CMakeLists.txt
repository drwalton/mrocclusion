cmake_minimum_required(VERSION 3.6)

project(mrocclusion LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
#Disables Boost's auto-linking, which often causes problems for me on Windows.
add_definitions( -DBOOST_ALL_NO_LIB )

add_definitions(/DGOOGLE_GLOG_DLL_DECL=)

#Needs to be defined for some libraries to link correctly (e.g.
# program_options).
add_definitions( -DBOOST_ALL_DYN_LINK  )
endif(WIN32)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

#Turn on modern C++ in Clang.
if(APPLE)
	set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif(APPLE)

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -pthread -fPIC")
endif(NOT WIN32)

find_package(CGAL REQUIRED)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(CUDA REQUIRED)
find_package(Eigen REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenNI2 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(Boost REQUIRED)
find_package(Boost COMPONENTS Filesystem System Program_Options thread REQUIRED)
find_package(Assimp REQUIRED)
find_package(ZLIB REQUIRED)
find_package(InfiniTAM REQUIRED)
find_package(ARToolKitPlus REQUIRED)
find_package(freeglut REQUIRED)

find_package(Ceres REQUIRED)
find_package(CXSparse REQUIRED)

include_directories(include)
include_directories(.)
add_definitions(-DTW_STATIC)
include_directories(3rdParty/AntTweakBar_sdl2/include)
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(${ARTOOLKITPLUS_INCLUDE_DIRS})
include_directories(${CGAL_INCLUDE_DIRS})
include_directories(${gmp_INCLUDE_DIRS})
include_directories(${EIGEN_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${SDL_ttf_INCLUDE_DIRS})
include_directories(${MPFR_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${OPENNI2_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${InfiniTAM_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${GLOG_INCLUDE_DIR})
include_directories(${CERES_INCLUDE_DIRS})

#Autogenerate directories file.
file(WRITE include/mrocclusion/Directories.hpp "\
\#ifndef MROCCLUSION_DIRECTORIES_HPP_INCLUDED\n\
\#define MROCCLUSION_DIRECTORIES_HPP_INCLUDED\n\
\n
\#include <string>\n
\n
const std::string MROCCLUSION_CONFIG_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/config/\";\n\
const std::string MROCCLUSION_CALIB_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/calib/\";\n\
const std::string MROCCLUSION_SHADER_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\";\n\
const std::string MROCCLUSION_MODEL_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/models/\";\n\
const std::string MROCCLUSION_FONT_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/fonts/\";\n\
const std::string MROCCLUSION_RESULTS_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/results/\";\n\
const std::string MROCCLUSION_RECORDING_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/recordings/\";\n\
\n
\#endif\n\
")

add_subdirectory(3rdParty)
add_subdirectory(src)


set(MROCCLUSION_EXEC_LIBRARIES
	occlusion
	AntTweakBar
	inpaint
	poly2tri
	${ZLIB_LIBRARIES}
	optimized
		${CGAL_LIBRARIES}
	debug
		${CGAL_LIBRARIES_DEBUG}
	${OPENCV_LIBRARIES}
	optimized
		${InfiniTAM_LIBRARIES}
	debug
		${InfiniTAM_LIBRARIES_DEBUG}
	${FREEGLUT_LIBRARIES}
	optimized ${ARTOOLKITPLUS_LIBRARIES}
	debug ${ARTOOLKITPLUS_LIBRARIES_DEBUG}
	${SDL_LIBRARIES}
	${OPENGL_LIBRARIES}
	${OPENNI2_LIBRARIES}
	${GLEW_LIBRARY}
	${SDL_ttf_LIBRARIES}
	${Boost_LIBRARIES}
	${ASSIMP_LIBRARIES}
	${CUDA_CUDA_LIBRARY}
	${CUDA_CUDART_LIBRARY}
	${CUDA_cublas_LIBRARY}
	${CUDA_cufft_LIBRARY}
	${MPFR_LIBRARIES}
	${GMP_LIBRARY}
	${MROCCLUSION_EXEC_LIBRARIES}
	${CERES_LIBRARIES}
	${CXSPARSE_LIBRARY}
)

link_directories(${CUDA_TOOLKIT_ROOT_DIR}/lib/x64/)

message("Ceres optimized: ${CERES_LIBRARIES_RELEASE}")
message("Ceres debug: ${CERES_LIBRARIES_DEBUG}")
message("Final Library List: ${MROCCLUSION_EXEC_LIBRARIES}")

add_subdirectory(test)
add_subdirectory(tools)
