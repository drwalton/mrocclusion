#include "mrocclusion/GuidedFilterGpu.hpp"
#include "mrocclusion/Directories.hpp"
#include <mrocclusion/Texture.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/GLBuffer.hpp>
#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>


const size_t partialSumBlockSize = 128;

struct GuidedFilterGpu::Impl {
	size_t r; 
	float eps;
	size_t width, height, bufferSize, nBlocksHorz, nBlocksVert, nBlockSumsHorz, nBlockSumsVert;
	Texture inputColor;
	ShaderProgram multiplyShader,
		copyShader,
		addInPlaceShader,
		divideInPlaceShader, 
		subtractProductShader,
		mulAdd6Shader, 
		sum4Shader, 
		sum1Sub3Shader,
		satHorzSumBlocks, satHorzSumSums, satHorzAddSums,
		satVertSumBlocks, satVertSumSums, satVertAddSums,
		boxFilterFromSat, normalBoxFilterFromSat,
		colorFromRgb8Tex, inputFromR8Tex, outputToR8Tex,
		squaredMeanToVarShader, squaredMeanToCovarShader,
		productMinusProductShader,
		thresholdCostsShader;
	ShaderStorageBuffer gray, output, colorR, colorG, colorB, partialSumBuffer, satBuffer;
	ShaderStorageBuffer meanIr, meanIg, meanIb;
	ShaderStorageBuffer invrr, invrg, invrb, invgg, invgb, invbb;
	ShaderStorageBuffer tmp, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
	ShaderStorageBuffer ar, ag, ab, b;

	void testShowImage(const std::string &title, ShaderStorageBuffer *image, bool normalize = false)
	{
		cv::Mat im(cv::Size(width, height), CV_32FC1);
		image->getData((float*)im.data, bufferSize);
		if (normalize) {
			double min, max;
			cv::minMaxLoc(im, &min, &max);
			cv::imshow(title, im / max);
		} else {
			cv::imshow(title, im);
		}
	}

	Impl(size_t width, size_t height)
		:inputColor(GL_TEXTURE_2D, GL_RGB8,
			width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, nullptr),
		width(width), height(height),
		bufferSize(width*height*sizeof(float)),
		nBlocksHorz(ceil(float(width) / float(partialSumBlockSize))),
		nBlocksVert(ceil(float(height) / float(partialSumBlockSize))),
		nBlockSumsHorz(ceil(float(nBlocksHorz) / float(partialSumBlockSize))),
		nBlockSumsVert(ceil(float(nBlocksVert) / float(partialSumBlockSize))),
		copyShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/Copy.comp" }),
		multiplyShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/ElementWiseMultiply.comp" }),
		divideInPlaceShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/DivideInPlace.comp" }),
		addInPlaceShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/AddInPlace.comp" }),
		mulAdd6Shader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/MulAdd6.comp" }),
		sum4Shader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/Sum4.comp" }),
		sum1Sub3Shader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/Sum1Sub3.comp" }),
		satHorzSumBlocks({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatHorzSumBlocks.comp" }),
		satHorzSumSums({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatHorzSumSums.comp" }),
		satHorzAddSums({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatHorzAddSums.comp" }),
		satVertSumBlocks({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatVertSumBlocks.comp" }),
		satVertSumSums({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatVertSumSums.comp" }),
		satVertAddSums({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SatVertAddSums.comp" }),
		boxFilterFromSat({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/BoxFilterFromSat.comp" }),
		normalBoxFilterFromSat({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/NormalBoxFilterFromSat.comp" }),
		colorFromRgb8Tex({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/ColorFromRgb8Tex.comp" }),
		inputFromR8Tex({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/InputFromR8Tex.comp" }),
		outputToR8Tex({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/OutputToR8Tex.comp" }),
		squaredMeanToVarShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SquaredMeanToVar.comp" }),
		squaredMeanToCovarShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SquaredMeanToCovar.comp" }),
		subtractProductShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/SubtractProduct.comp" }),
		productMinusProductShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/ProductMinusProduct.comp" }),
		thresholdCostsShader({ MROCCLUSION_SHADER_DIR + 
				"/guidedfilter/ThresholdCosts.comp" }),
		gray(bufferSize),
		output(bufferSize),
		colorR(bufferSize),
		colorG(bufferSize),
		colorB(bufferSize),
		partialSumBuffer(bufferSize),
		satBuffer(bufferSize),
		meanIr(bufferSize),
		meanIg(bufferSize),
		meanIb(bufferSize),
		invrr(bufferSize),
		invrg(bufferSize),
		invrb(bufferSize),
		invgg(bufferSize),
		invgb(bufferSize),
		invbb(bufferSize),
		tmp(bufferSize),
		tmp0(bufferSize),
		tmp1(bufferSize),
		tmp2(bufferSize),
		tmp3(bufferSize),
		tmp4(bufferSize),
		tmp5(bufferSize),
		ar(bufferSize),
		ag(bufferSize),
		ab(bufferSize),
		b(bufferSize)
		
	{}

	//Splits colorTex into r,g,b channels, converts to float and writes the result to
	// the colorR, colorG, colorB buffers.
	void initColor(Texture *colorTex)
	{
		colorFromRgb8Tex.use();
		glBindImageTexture(0, colorTex->tex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8UI);
		colorR.bindBase(1);
		colorG.bindBase(2);
		colorB.bindBase(3);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	void initColor(const float *r, const float *g, const float *b)
	{
		colorR.update(r, bufferSize);
		colorG.update(g, bufferSize);
		colorB.update(b, bufferSize);
	}

	void setupGuidedFilter(const float *r, const float *g, const float *b, size_t radius, float eps)
	{
		initColor(r, g, b);
		setupGuidedFilter(radius, eps);
	}

	void setupGuidedFilter(Texture *colorTex, size_t radius, float eps)
	{
		//split I into colorR, colorG, colorB.
		initColor(colorTex);
		setupGuidedFilter(radius, eps);
	}
	void setupGuidedFilter(size_t radius, float eps)
	{
		this->r = radius;
		this->eps = eps;
		if (radius == 0) {
			return;
		}

		//Generate means.
		normalBoxFilter(&colorR, &meanIr, radius);
		normalBoxFilter(&colorG, &meanIg, radius);
		normalBoxFilter(&colorB, &meanIb, radius);

		// ==== Find variances ====
		ShaderStorageBuffer
			*varIrr = &tmp0,
			*varIrg = &tmp1,
			*varIrb = &tmp2,
			*varIgg = &tmp3,
			*varIgb = &tmp4,
			*varIbb = &tmp5;
		//varIrr
		multiply(&colorR, &colorR, &tmp);
		normalBoxFilter(&tmp, varIrr, radius);
		squaredMeanToVar(varIrr, &meanIr, eps);


		//varIrg
		multiply(&colorR, &colorG, &tmp);
		normalBoxFilter(&tmp, varIrg, radius);
		squaredMeanToCovar(varIrg, &meanIr, &meanIg);


		//varIrb
		multiply(&colorR, &colorB, &tmp);
		normalBoxFilter(&tmp, varIrb, radius);
		squaredMeanToCovar(varIrb, &meanIr, &meanIb);

		//varIgg
		multiply(&colorG, &colorG, &tmp);
		normalBoxFilter(&tmp, varIgg, radius);
		squaredMeanToVar(varIgg, &meanIg, eps);

		//varIgb
		multiply(&colorG, &colorB, &tmp);
		normalBoxFilter(&tmp, varIgb, radius);
		squaredMeanToCovar(varIgb, &meanIg, &meanIb);

		//varIbb
		multiply(&colorB, &colorB, &tmp);
		normalBoxFilter(&tmp, varIbb, radius);
		squaredMeanToVar(varIbb, &meanIb, eps);


		// === Find inv variances ===
		productMinusProduct(&invrr, varIgg, varIbb, varIgb, varIgb);
		productMinusProduct(&invrg, varIgb, varIrb, varIrg, varIbb);
		productMinusProduct(&invrb, varIrg, varIgb, varIgg, varIrb);
		productMinusProduct(&invgg, varIrr, varIbb, varIrb, varIrb);
		productMinusProduct(&invgb, varIrb, varIrg, varIrr, varIgb);
		productMinusProduct(&invbb, varIrr, varIgg, varIrg, varIrg);

		//Find covdet
		ShaderStorageBuffer *covdet = &tmp;
		mulAdd6(covdet, &invrr, varIrr, &invrg, varIrg, &invrb, varIrb);

		//Divide all inv variances by covdet.
		divideInPlace(&invrr, covdet);
		divideInPlace(&invrg, covdet);
		divideInPlace(&invrb, covdet);
		divideInPlace(&invgg, covdet);
		divideInPlace(&invgb, covdet);
		divideInPlace(&invbb, covdet);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void applyFilter(ShaderStorageBuffer *input,
			ShaderStorageBuffer *output)
	{
		if (r == 0) {
			copyBuffer(input, output);
			return;
		}

		ShaderStorageBuffer
			*covIpr = &tmp0,
			*covIpg = &tmp1,
			*covIpb = &tmp2;
		//meanIpr
		multiply(&colorR, input, &tmp);
		normalBoxFilter(&tmp, covIpr, r);
		//meanIpg
		multiply(&colorG, input, &tmp);
		normalBoxFilter(&tmp, covIpg, r);
		//meanIpb
		multiply(&colorB, input, &tmp);
		normalBoxFilter(&tmp, covIpb, r);

		//meanP
		ShaderStorageBuffer *meanP = &tmp;
		normalBoxFilter(input, meanP, r);

		//covIpr
		subtractProduct(covIpr, &meanIr, meanP);

		//covIpg
		subtractProduct(covIpg, &meanIg, meanP);

		//covIpb
		subtractProduct(covIpb, &meanIb, meanP);

		//ar
		mulAdd6(&ar,
			&invrr, covIpr,
			&invrg, covIpg,
			&invrb, covIpb);

		//ag
		mulAdd6(&ag,
			&invrg, covIpr,
			&invgg, covIpg,
			&invgb, covIpb);

		//ab
		mulAdd6(&ab,
			&invrb, covIpr,
			&invgb, covIpg,
			&invbb, covIpb);

		//b
		sum1Sub3(&b, meanP,
			&ar, &meanIr,
			&ag, &meanIg,
			&ab, &meanIb);

		//Final result
		normalBoxFilter(&ar, &tmp0, r);
		normalBoxFilter(&ag, &tmp1, r);
		normalBoxFilter(&ab, &tmp2, r);
		normalBoxFilter(&b, &tmp, r);
		sum4(output, &tmp, 
			&tmp0, &colorR, 
			&tmp1, &colorG,
			&tmp2, &colorB);
	}

	void filterUChar(Texture *inputTex, Texture *outputTex)
	{
		inputFromR8Tex.use();
		glBindImageTexture(0, inputTex->tex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
		gray.bindBase(1);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		applyFilter(&gray, &output);

		bufferToR8Tex(&output, outputTex);
	}

	void bufferToR8Tex(ShaderStorageBuffer *buffer, Texture *texture)
	{
		outputToR8Tex.use();
		glBindImageTexture(0, texture->tex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
		buffer->bindBase(1);
		glDispatchCompute(width, height, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	//!\brief Computes inout - (mean*mean) + eps and stores result in inout.
	void squaredMeanToVar(
		ShaderStorageBuffer *inout,
		ShaderStorageBuffer *mean,
		float eps)
	{
		squaredMeanToVarShader.use();
		inout->bindBase(0);
		mean->bindBase(1);
		squaredMeanToVarShader.setUniform("eps", eps);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Copies input into output.
	void copyBuffer(
		ShaderStorageBuffer *input,
		ShaderStorageBuffer *output)
	{
		copyShader.use();
		input->bindBase(0);
		output->bindBase(1);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes inout - (meana*meanb) and stores result in inout.
	void squaredMeanToCovar(
		ShaderStorageBuffer *inout,
		ShaderStorageBuffer *meana,
		ShaderStorageBuffer *meanb)
	{
		squaredMeanToCovarShader.use();
		inout->bindBase(0);
		meana->bindBase(1);
		meanb->bindBase(2);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes inout - b*c and stores result in inout.
	void subtractProduct(
		ShaderStorageBuffer *inout,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *c)
	{
		subtractProductShader.use();
		inout->bindBase(0);
		b->bindBase(1);
		c->bindBase(2);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes b*c - d*e and stores result in out.
	void productMinusProduct(
		ShaderStorageBuffer *out,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *c,
		ShaderStorageBuffer *d,
		ShaderStorageBuffer *e)
	{
		productMinusProductShader.use();
		out->bindBase(0);
		b->bindBase(1);
		c->bindBase(2);
		d->bindBase(3);
		e->bindBase(4);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes a*b and stores result in out.
	void multiply(ShaderStorageBuffer *a,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *out)
	{
		multiplyShader.use();
		a->bindBase(0);
		b->bindBase(1);
		out->bindBase(2);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes inout + b and stores result in inout.
	void addInPlace(ShaderStorageBuffer *inout,
		ShaderStorageBuffer *b)
	{
		addInPlaceShader.use();
		inout->bindBase(0);
		b->bindBase(1);
		glDispatchCompute(width*height, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes b*c + d*e + f*g and stores result in out.
	void mulAdd6(ShaderStorageBuffer *out,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *c,
		ShaderStorageBuffer *d,
		ShaderStorageBuffer *e,
		ShaderStorageBuffer *f,
		ShaderStorageBuffer *g)
	{
		mulAdd6Shader.use();
		out->bindBase(0);
		b->bindBase(1);
		c->bindBase(2);
		d->bindBase(3);
		e->bindBase(4);
		f->bindBase(5);
		g->bindBase(6);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes b + c*d + e*f + g*h and stores result in out.
	void sum4(ShaderStorageBuffer *out,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *c,
		ShaderStorageBuffer *d,
		ShaderStorageBuffer *e,
		ShaderStorageBuffer *f,
		ShaderStorageBuffer *g,
		ShaderStorageBuffer *h)
	{
		sum4Shader.use();
		out->bindBase(0);
		b->bindBase(1);
		c->bindBase(2);
		d->bindBase(3);
		e->bindBase(4);
		f->bindBase(5);
		g->bindBase(6);
		h->bindBase(7);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes b - (c1*c2 + d1*d2 + e1*e2) and stores result in out.
	void sum1Sub3(ShaderStorageBuffer *out,
		ShaderStorageBuffer *b,
		ShaderStorageBuffer *c1,
		ShaderStorageBuffer *c2,
		ShaderStorageBuffer *d1,
		ShaderStorageBuffer *d2,
		ShaderStorageBuffer *e1,
		ShaderStorageBuffer *e2)
	{
		sum1Sub3Shader.use();
		out->bindBase(0);
		b->bindBase(1);
		c1->bindBase(2);
		c2->bindBase(3);
		d1->bindBase(4);
		d2->bindBase(5);
		e1->bindBase(6);
		e2->bindBase(7);
		glDispatchCompute(width*height/4, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Computes inout / b and stores the result in inout.
	//!\note At locations where b == 0, inout is left as-is.
	void divideInPlace(ShaderStorageBuffer *inout,
			ShaderStorageBuffer *b)
	{
		divideInPlaceShader.use();
		inout->bindBase(0);
		b->bindBase(1);
		glDispatchCompute(width*height/4, 1, 1);
	}

	//!\brief Snaps input to 0 or 1 at each pixel.
	void thresholdCosts(ShaderStorageBuffer *inout, Texture *classTex)
	{
		//cv::Mat thresholdCosts(cv::Size(width, height), CV_32FC1);
		//inout->getData(thresholdCosts.data, bufferSize);
		thresholdCostsShader.setUniform("width", GLint(width));
		thresholdCostsShader.use();
		inout->bindBase(0);
		glBindImageTexture(1, classTex->tex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
		//cv::imshow("Filtered costs(GPU)", thresholdCosts);

		glDispatchCompute(width, height, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		//inout->getData(thresholdCosts.data, bufferSize);
		//cv::imshow("ThresholdedCosts (GPU)", thresholdCosts);
	}

	//!\brief Generate a summed-area table from an input buffer.
	void makeSat(ShaderStorageBuffer *inputBuffer, ShaderStorageBuffer *satBuffer)
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satHorzSumBlocks.use();
		inputBuffer->bindBase(0);
		satBuffer->bindBase(1);
		satHorzSumBlocks.setUniform("width", GLuint(width));
		glDispatchCompute(nBlocksHorz, height, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satHorzSumSums.use();
		satBuffer->bindBase(0);
		partialSumBuffer.bindBase(1);
		satHorzSumSums.setUniform("width", GLuint(width));
		satHorzSumSums.setUniform("nBlocksHorz", GLuint(nBlocksHorz));
		glDispatchCompute(nBlockSumsHorz, height, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satHorzAddSums.use();
		satBuffer->bindBase(0);
		partialSumBuffer.bindBase(1);
		satHorzAddSums.setUniform("width", GLuint(width));
		//satHorzAddSums.setUniform("nBlocksHorz", GLuint(nBlocksHorz));
		glDispatchCompute(nBlocksHorz-1, height, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satVertSumBlocks.use();
		satBuffer->bindBase(0);
		satVertSumBlocks.setUniform("width", GLuint(width));
		satVertSumBlocks.setUniform("height", GLuint(height));
		glDispatchCompute(width, nBlocksVert, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satVertSumSums.use();
		satBuffer->bindBase(0);
		partialSumBuffer.bindBase(1);
		satVertSumSums.setUniform("width", GLuint(width));
		satVertSumSums.setUniform("height", GLuint(height));
		satVertSumSums.setUniform("nBlocksVert", GLuint(nBlocksVert));
		glDispatchCompute(width, nBlockSumsVert, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		satVertAddSums.use();
		satBuffer->bindBase(0);
		partialSumBuffer.bindBase(1);
		satVertAddSums.setUniform("width", GLuint(width));
		satVertAddSums.setUniform("height", GLuint(height));
		//satVertAddSums.setUniform("nBlocksVert", GLuint(nBlocksVert));
		glDispatchCompute(width, nBlocksVert-1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Apply a (NOT NORMALIZED) box filter to an input buffer.
	void boxFilter(
		ShaderStorageBuffer *inputBuffer, 
		ShaderStorageBuffer *outputBuffer, 
		size_t radius)
	{
		makeSat(inputBuffer, &satBuffer);

		boxFilterFromSat.use();
		satBuffer.bindBase(0);
		outputBuffer->bindBase(1);
		boxFilterFromSat.setUniform("radius", GLint(radius));
		boxFilterFromSat.setUniform("width", GLint(width));
		boxFilterFromSat.setUniform("height", GLint(height));
		glDispatchCompute(width, height, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	//!\brief Apply a (NORMALIZED) box filter to an input buffer.
	void normalBoxFilter(
		ShaderStorageBuffer *inputBuffer, 
		ShaderStorageBuffer *outputBuffer, 
		size_t radius)
	{
		makeSat(inputBuffer, &satBuffer);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		normalBoxFilterFromSat.use();
		satBuffer.bindBase(0);
		outputBuffer->bindBase(1);
		normalBoxFilterFromSat.setUniform("radius", GLint(radius));
		normalBoxFilterFromSat.setUniform("width", GLint(width));
		normalBoxFilterFromSat.setUniform("height", GLint(height));
		glDispatchCompute(width, height, 1);
	}
};

GuidedFilterGpu::GuidedFilterGpu(
	size_t width, size_t height)
	:pimpl_(new Impl(width, height)),
	visualise(true)
{
}

GuidedFilterGpu::~GuidedFilterGpu()
{
}

void GuidedFilterGpu::setup(Texture * I, size_t r, float eps)
{
	pimpl_->setupGuidedFilter(I, r, eps);
}

void GuidedFilterGpu::setup(const float * Ir, const float * Ig, const float * Ib, size_t r, float eps)
{
	pimpl_->setupGuidedFilter(Ir, Ig, Ib, r, eps);
}

void GuidedFilterGpu::costsToMatte(
	Texture * guidance, const float * initialCosts, 
	Texture *classTex,
	Texture * outputMatte, size_t costFilterR, 
	float costFilterEps, size_t finalFilterR, float finalFilterEps)
{
	pimpl_->initColor(guidance);
	pimpl_->setupGuidedFilter(costFilterR, costFilterEps);
	pimpl_->gray.update(initialCosts, pimpl_->bufferSize);
	pimpl_->applyFilter(&(pimpl_->gray), &(pimpl_->output));

	if (visualise) {
		cv::Mat filteredCosts(cv::Size(pimpl_->width, pimpl_->height), CV_32FC1);
		pimpl_->output.getData(filteredCosts.data, pimpl_->bufferSize);
		cv::imshow("Filtered costs", filteredCosts);
	}

	pimpl_->thresholdCosts(&(pimpl_->output), classTex);

	if (visualise) {
		cv::Mat filteredCosts(cv::Size(pimpl_->width, pimpl_->height), CV_32FC1);
		pimpl_->output.getData(filteredCosts.data, pimpl_->bufferSize);
		cv::imshow("Thresholded costs", filteredCosts);
	}

	pimpl_->setupGuidedFilter(finalFilterR, finalFilterEps);
	pimpl_->applyFilter(&(pimpl_->output), &(pimpl_->gray));
	pimpl_->bufferToR8Tex(&(pimpl_->gray), outputMatte);
}

void GuidedFilterGpu::filter(const float * input, float * output)
{
	pimpl_->gray.update(input, pimpl_->bufferSize);
	pimpl_->applyFilter(&(pimpl_->gray), &(pimpl_->output));
	pimpl_->output.getData(output, pimpl_->bufferSize);
}

void GuidedFilterGpu::filterUChar(Texture* input, Texture* output)
{
	pimpl_->filterUChar(input, output);
}

void GuidedFilterGpu::testSat(const float * input, float * output)
{
	pimpl_->colorR.update(input, pimpl_->width*pimpl_->height*sizeof(float));
	auto t = std::chrono::system_clock::now();
	size_t nIters = 10000;
	for (size_t i = 0; i < nIters; ++i) {
		pimpl_->makeSat(&(pimpl_->colorR), &(pimpl_->colorG));
	}
	auto d = std::chrono::system_clock::now() - t;
	std::cout << "Time taken for " << nIters << " iterations: " << 
		std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
	pimpl_->colorG.getData(output, pimpl_->width*pimpl_->height * sizeof(float));
}

void GuidedFilterGpu::testBoxFilter(const float * input, float * output)
{
	pimpl_->colorR.update(input, pimpl_->width*pimpl_->height*sizeof(float));
	auto t = std::chrono::system_clock::now();
	size_t nIters = 10000;
	for (size_t i = 0; i < nIters; ++i) {
		pimpl_->boxFilter(&(pimpl_->colorR), &(pimpl_->colorG), 5);
	}
	auto d = std::chrono::system_clock::now() - t;
	std::cout << "Time taken for " << nIters << " iterations: " << 
		std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
	pimpl_->colorG.getData(output, pimpl_->width*pimpl_->height * sizeof(float));
}

void GuidedFilterGpu::testNormalBoxFilter(const float * input, float * output)
{
	pimpl_->colorR.update(input, pimpl_->width*pimpl_->height*sizeof(float));
	auto t = std::chrono::system_clock::now();
	size_t nIters = 100;
	for (size_t i = 0; i < nIters; ++i) {
		pimpl_->normalBoxFilter(&(pimpl_->colorR), &(pimpl_->colorG), 5);
	}
	auto d = std::chrono::system_clock::now() - t;
	std::cout << "Time taken for " << nIters << " iterations: " << 
		std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
	pimpl_->colorG.getData(output, pimpl_->width*pimpl_->height * sizeof(float));
}

void GuidedFilterGpu::testGuidedFilter(const float *Ir, const float *Ig, const float *Ib, const float *p, float *out, size_t r, float eps)
{
	pimpl_->initColor(Ir, Ig, Ib);
	pimpl_->gray.update(p, pimpl_->bufferSize);
	size_t nIters = 100;
	auto t = std::chrono::system_clock::now();
	for (size_t i = 0; i < nIters; ++i) {
		pimpl_->setupGuidedFilter(r, eps);
		pimpl_->applyFilter(&(pimpl_->gray), &(pimpl_->output));
		pimpl_->applyFilter(&(pimpl_->gray), &(pimpl_->output));
		glFinish();
	}
	auto d = std::chrono::system_clock::now() - t;
	std::cout << "Time taken for " << nIters << " iterations: " << 
		std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
	pimpl_->output.getData(out, pimpl_->bufferSize);
}
