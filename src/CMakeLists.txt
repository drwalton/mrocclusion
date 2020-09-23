set(OCCLUSION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/mrocclusion)

add_library(occlusion
	${OCCLUSION_INCLUDE_DIR}/NonCopyable.hpp

	BlobRemoval.cpp
	${OCCLUSION_INCLUDE_DIR}/BlobRemoval.hpp

	Comparison.cpp
	${OCCLUSION_INCLUDE_DIR}/Comparison.hpp

	CrabbOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/CrabbOcclusionMethod.hpp

	RgbHistogram.cpp
	${OCCLUSION_INCLUDE_DIR}/RgbHistogram.hpp

	RgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/RgbdOcclusionMethod.hpp

	BaselineBilateralOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/BaselineBilateralOcclusionMethod.hpp

	BaselineGuidedOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/BaselineGuidedOcclusionMethod.hpp

	BaselineManifoldOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/BaselineManifoldOcclusionMethod.hpp

	AdaptiveBilateralRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/AdaptiveBilateralRgbdOcclusionMethod.hpp

	BilateralRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/BilateralRgbdOcclusionMethod.hpp

	ErrorMetrics.cpp
	${OCCLUSION_INCLUDE_DIR}/ErrorMetrics.hpp

	GrabcutRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/GrabcutRgbdOcclusionMethod.hpp

	GuidedRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/GuidedRgbdOcclusionMethod.hpp

	AdaptiveManifoldOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/AdaptiveManifoldOcclusionMethod.hpp

	IterativeRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/IterativeRgbdOcclusionMethod.hpp
	${OCCLUSION_INCLUDE_DIR}/IterativeRgbdOcclusionMethod-inl.hpp

	IterativeRgbdOcclusionMethodMOG.cpp
	${OCCLUSION_INCLUDE_DIR}/IterativeRgbdOcclusionMethodMOG.hpp

	IterativeRgbdOcclusionMethodHistogram.cpp
	${OCCLUSION_INCLUDE_DIR}/IterativeRgbdOcclusionMethodHistogram.hpp

	NaiveRgbdOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/NaiveRgbdOcclusionMethod.hpp

	CostVolumeOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/CostVolumeOcclusionMethod.hpp

	guidedfilter.cpp
	${OCCLUSION_INCLUDE_DIR}/guidedfilter.h

	GuidedFilterGpu.cpp
	${OCCLUSION_INCLUDE_DIR}/GuidedFilterGpu.hpp

	SelectiveGuidedFilter.cpp
	${OCCLUSION_INCLUDE_DIR}/SelectiveGuidedFilter.hpp

	SelectiveGuidedOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/SelectiveGuidedOcclusionMethod.hpp

	GpuCvfOcclusionMethod.cpp
	${OCCLUSION_INCLUDE_DIR}/GpuCvfOcclusionMethod.hpp

	AntTweakBar.cpp
	${OCCLUSION_INCLUDE_DIR}/AntTweakBar.hpp
	ArCamera.cpp
	${OCCLUSION_INCLUDE_DIR}/ArCamera.hpp
	ArScene.cpp
	${OCCLUSION_INCLUDE_DIR}/ArScene.hpp
	CameraModel.cpp
	${OCCLUSION_INCLUDE_DIR}/CameraModel.hpp
	DepthCam.cpp
	${OCCLUSION_INCLUDE_DIR}/DepthCam.hpp
	Entity.cpp
	${OCCLUSION_INCLUDE_DIR}/Entity.hpp
	Exception.cpp
	${OCCLUSION_INCLUDE_DIR}/Exception.hpp
	Files.cpp
	${OCCLUSION_INCLUDE_DIR}/Files.hpp
	FullScreenQuad.cpp
	${OCCLUSION_INCLUDE_DIR}/FullScreenQuad.hpp
	GLBuffer.cpp
	${OCCLUSION_INCLUDE_DIR}/GLBuffer.hpp
	GLWindow.cpp
	${OCCLUSION_INCLUDE_DIR}/GLWindow.hpp
	InftamRgbdSlam.cpp
	${OCCLUSION_INCLUDE_DIR}/InftamRgbdSlam.hpp
	KahanVal.cpp
	${OCCLUSION_INCLUDE_DIR}/KahanVal.hpp
	MarkerTracker.cpp
	${OCCLUSION_INCLUDE_DIR}/MarkerTracker.hpp
	Matrices.cpp
	${OCCLUSION_INCLUDE_DIR}/Matrices.hpp
	Mesh.cpp
	${OCCLUSION_INCLUDE_DIR}/Mesh.hpp
	ModelLoader.cpp
	${OCCLUSION_INCLUDE_DIR}/ModelLoader.hpp
	OpenNIDepthCam.cpp
	${OCCLUSION_INCLUDE_DIR}/OpenNIDepthCam.hpp
	PinholeCameraModel.cpp
	${OCCLUSION_INCLUDE_DIR}/PinholeCameraModel.hpp
	Renderable.cpp
	${OCCLUSION_INCLUDE_DIR}/Renderable.hpp
	RenderToTexture.cpp
	${OCCLUSION_INCLUDE_DIR}/RenderToTexture.hpp
	RgbdSlam.cpp
	${OCCLUSION_INCLUDE_DIR}/RgbdSlam.hpp
	ScaramuzzaOmniCameraModel.cpp
	${OCCLUSION_INCLUDE_DIR}/ScaramuzzaOmniCameraModel.hpp
	ShaderProgram.cpp
	${OCCLUSION_INCLUDE_DIR}/ShaderProgram.hpp
	SimpleOmniCameraModel.cpp
	${OCCLUSION_INCLUDE_DIR}/SimpleOmniCameraModel.hpp
	Texture.cpp
	${OCCLUSION_INCLUDE_DIR}/Texture.hpp
	TexturedPhongMesh.cpp
	${OCCLUSION_INCLUDE_DIR}/TexturedPhongMesh.hpp
	VectorTypes.cpp
	${OCCLUSION_INCLUDE_DIR}/VectorTypes.hpp
	VertexArrayObject.cpp
	${OCCLUSION_INCLUDE_DIR}/VertexArrayObject.hpp

	${OCCLUSION_INCLUDE_DIR}/Constants.hpp
)

set_target_properties(occlusion PROPERTIES FOLDER src)
target_link_libraries(occlusion AntTweakBar)
