#ifndef MROCCLUSION_GUIDEDFILTERGPU_HPP_INCLUDED
#define MROCCLUSION_GUIDEDFILTERGPU_HPP_INCLUDED

#include <memory>

class Texture;

class GuidedFilterGpu
{
public:
    GuidedFilterGpu(size_t width, size_t height);
    ~GuidedFilterGpu();

	//!\brief Set up guided filter, ready to apply to an image.
	//!\param I Guidance image. Supplied as an RGBA8 format texture. Alpha channel is not used.
	//!\note Fast version (GPU-GPU).
	void setup(Texture *I, size_t r, float eps);

	//!\brief Set up guided filter, ready to apply to an image.
	//!\param Ir Red channel of guidance image, in floating point format. Should be [0,1].
	//!\param Ig Green channel of guidance image, in floating point format. Should be [0,1].
	//!\param Ib Blue channel of guidance image, in floating point format. Should be [0,1].
	//!\note Slow version (requires CPU-GPU copy).
	void setup(const float *Ir, const float *Ig, const float *Ib, size_t r, float eps);

	void costsToMatte(Texture *guidance,
		const float *initialCosts,
		Texture *classTex,
		Texture *outputMatte,
		size_t costFilterR, float costFilterEps,
		size_t finalFilterR, float finalFilterEps);

	//!\brief Apply filter to an image.
	//!\note Slow version (requires input->GPU and output->CPU).
	void filter(const float *input, float *output);

	//!\brief Apply filter to an R8 texture. The result is converted to R8.
	//!\param input Input image. Pointer to an R8 texture.
	//!\param output Output image. Pointer to an R8 texture.
	//!\note Fast version  (no CPU-GPU copying).
	void filterUChar(Texture* input, Texture* output);

	// Test functions: these time components of the filter over multiple iterations.
	void testSat(const float *input, float *output);
	void testBoxFilter(const float *input, float *output);
	void testNormalBoxFilter(const float *input, float *output);
	void testGuidedFilter(
		const float *Ir, const float *Ig, const float *Ib, 
		const float *p, float *out, size_t r, float eps);

	bool visualise;
private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

#endif
