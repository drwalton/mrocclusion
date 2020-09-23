#ifndef MROCCLUSION_MARKERTRACKER_HPP_INCLUDED
#define MROCCLUSION_MARKERTRACKER_HPP_INCLUDED

#include <memory>
#include <mrocclusion/PinholeCameraModel.hpp>

//!\brief Marker-based AR tracker.
class MarkerTracker final {
public:
	explicit MarkerTracker(const PinholeCameraModel &model,
		float patternWidth = 0.15f, 
		float borderWidth = 0.25f);

	~MarkerTracker() throw();

	void processImage(const unsigned char *image);

	bool trackingGood() const;

	float confidence() const;
	
	//!\brief Return transform between camera and marker (rigid transform)
	mat4 getModelViewMat() const;
private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

#endif //MROCCLUSION_ARCAMERA_HPP_INCLUDED
