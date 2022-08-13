# MR Occlusion

This contains code for comparing various methods for handling MR occlusion, in addition to an implementation of our Cost Volume Filtering occlusion-handling approach.

For more details please see [our paper](https://discovery.ucl.ac.uk/id/eprint/1575582/7/Walton_a11-walton.pdf).

## Requirements
This code has the following external dependencies:
* Assimp http://assimp.org/
* ARToolkitPlus https://github.com/paroj/artoolkitplus
* Eigen http://eigen.tuxfamily.org/index.php?title=Main_Page 
* Boost https://www.boost.org/ 
* SDL2 https://www.libsdl.org/ 
* SDL2_ttf https://www.libsdl.org/projects/SDL_ttf/ 
* OpenCV (including the opencv_contrib extra modules)
    * This has been tested with OpenCV built from the latest github code: https://github.com/opencv/opencv and https://github.com/opencv/opencv_contrib.
* ZLib http://zlib.net/ 
* InfiniTAM https://github.com/victorprad/InfiniTAM
* Ceres https://github.com/tbennun/ceres-windows/ (for a version you can build on windows)

## Live demos
The OcclusionInfinitamLive and OcclusionMethodsLive both use an ARToolkitPlus marker to determine where to render the virtual scene, so you will need to print one out in order to use these demos. The marker images are available [in the ARToolkitPlus repository](https://github.com/paroj/artoolkitplus/tree/master/id-markers).
