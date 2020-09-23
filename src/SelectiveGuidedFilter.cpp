#include "mrocclusion/SelectiveGuidedFilter.hpp"

//!\brief Clone the source to the destination, converting to a floating-point
//!       format if necessary.
void makeFloatOrCopy(const cv::Mat &src, cv::Mat *dst)
{
	if (src.type() == CV_32FC1 || src.type() == CV_32FC3) {
		*dst = src.clone();
	} else {
		src.convertTo(*dst, CV_32F);
		*dst /= 255.f;
	}
}

void selectiveBoxFilter(const cv::Mat & inputImage, size_t rad, const cv::Mat & mask, cv::Mat * outputImage)
{
	cv::Mat maskedInput = inputImage;
	maskedInput.setTo(0, ~mask);
	cv::blur(maskedInput, *outputImage, cv::Size(rad*2+1, rad*2+1));
	inputImage.copyTo(*outputImage, ~mask);
	/*

	//Basic on2 for now.
	if (outputImage->size() != inputImage.size() ||
		outputImage->type() != inputImage.type()) {
		*outputImage = cv::Mat(inputImage.size(), inputImage.type());
	}
	for (size_t r = 0; r < inputImage.rows; ++r) {
		for (size_t c = 0; c < inputImage.cols; ++c) {
			size_t startR = r < rad ? 0 : r - rad;
			size_t endR = r > inputImage.rows-rad ? inputImage.rows-1 : r + rad;
			size_t startC = c < rad ? 0 : c - rad;
			size_t endC = c > inputImage.cols-rad ? inputImage.cols-1 : c + rad;
			size_t denom = 0;

			if (inputImage.type() == CV_32FC1) {
				float num = 0;
				for (size_t r2 = startR; r2 < endR; ++r2) {
					for (size_t c2 = startC; c2 < endC; ++c2) {
						if (mask.at<unsigned char>(r2, c2)) {
							denom++;
							num += inputImage.at<float>(r2, c2);
						}
					}
				}
				if (denom == 0) {
					outputImage->at<float>(r, c) = inputImage.at<float>(r,c);
				} else {
					outputImage->at<float>(r, c) = num / (float)(denom);
				}
			} else if (inputImage.type() == CV_32FC3) {
				cv::Vec3f num = 0;
				for (size_t r2 = startR; r2 < endR; ++r2) {
					for (size_t c2 = startC; c2 < endC; ++c2) {
						if (mask.at<unsigned char>(r, c)) {
							denom++;
							num += inputImage.at<cv::Vec3f>(r, c);
						}
					}
				}
				if (denom == 0) {
					outputImage->at<cv::Vec3f>(r, c) = inputImage.at<cv::Vec3f>(r,c);
				} else {
					outputImage->at<cv::Vec3f>(r, c) = num / (float)(denom);
				}
			} else {
				throw std::runtime_error("selectiveboxfilter not compatible with this mat type.");
			}
		}
	}
	*/
}

void selectiveGuidedFilterMonoMono(const cv::Mat & p, const cv::Mat & I, const cv::Mat & mask, cv::Mat * out,
	size_t r, float eps)
{
	cv::Mat pf, If;
	if (p.type() == CV_32FC1) {
		pf = p.clone();
	} else {
		p.convertTo(pf, CV_32F);
	}
	if (I.type() == CV_32FC1) {
		If = I.clone();
	} else {
		I.convertTo(If, CV_32F);
	}
	
	cv::Mat meanI, meanp;
	selectiveBoxFilter(If, r, mask, &meanI);
	selectiveBoxFilter(pf, r, mask, &meanp);

	cv::Mat corrI, corrIp;
	selectiveBoxFilter(If.mul(If), r, mask, &corrI);
	selectiveBoxFilter(If.mul(pf), r, mask, &corrIp);

	cv::Mat varI, covIp;
	varI = corrI - meanI.mul(meanI);
	covIp = corrIp - meanI.mul(meanp);

	cv::Mat a, b;
	a = covIp / (varI + eps);
	b = meanp - a.mul(meanI);

	cv::Mat meana, meanb;
	cv::boxFilter(a, meana, -1, cv::Size(r*2, r*2));
	cv::boxFilter(b, meanb, -1, cv::Size(r*2, r*2));

	cv::Mat outf = meana.mul(If) + meanb;
	outf.convertTo(*out, CV_8UC1);
}

void selectiveGuidedFilter(
	const cv::Mat & inputImage, const cv::Mat & guidanceImage, 
	const cv::Mat & mask, cv::Mat * outputImage,
	size_t radius, float eps)
{
	if (inputImage.channels() == 1 && guidanceImage.channels() == 1) {
		selectiveGuidedFilterMonoMono(inputImage, guidanceImage, mask, outputImage,
			radius, eps);
	} else {
		throw std::runtime_error("No implementation for this type of matrix.");
	}
}


void selectiveGuidedFilter(
	const cv::Mat &p, const cv::Mat &I,
	const cv::Mat &processMask, const cv::Mat &useMask,
	cv::Mat *outputImage,
	size_t r, float eps)
{
	cv::Mat pf, If;
	makeFloatOrCopy(p, &pf);
	makeFloatOrCopy(I, &If);


	cv::Mat I_r, I_g, I_b;
	std::vector<cv::Mat> I_c;
	cv::split(If, I_c);
	I_r = I_c[0];
	I_g = I_c[1];
	I_b = I_c[2];
	
	cv::Mat meanI_r, meanI_g, meanI_b;

	selectiveBoxFilter(I_r, r, processMask, useMask, &meanI_r);
	selectiveBoxFilter(I_g, r, processMask, useMask, &meanI_g);
	selectiveBoxFilter(I_b, r, processMask, useMask, &meanI_b);

	cv::Mat varI_rr, varI_rg, varI_rb, varI_gg, varI_gb, varI_bb;
	selectiveBoxFilter(I_r.mul(I_r), r, processMask, useMask, &varI_rr);
	varI_rr = varI_rr - meanI_r.mul(meanI_r) + eps;
	selectiveBoxFilter(I_r.mul(I_g), r, processMask, useMask, &varI_rg);
	varI_rg = varI_rg - meanI_r.mul(meanI_g);
	selectiveBoxFilter(I_r.mul(I_b), r, processMask, useMask, &varI_rb);
	varI_rb = varI_rb - meanI_r.mul(meanI_b);
	selectiveBoxFilter(I_g.mul(I_g), r, processMask, useMask, &varI_gg);
	varI_gg = varI_gg - meanI_g.mul(meanI_g) + eps;
	selectiveBoxFilter(I_g.mul(I_b), r, processMask, useMask, &varI_gb);
	varI_gb = varI_gb - meanI_g.mul(meanI_b);
	selectiveBoxFilter(I_b.mul(I_b), r, processMask, useMask, &varI_bb);
	varI_bb = varI_bb - meanI_b.mul(meanI_b) + eps;

	/*
	double min, max;
	cv::minMaxLoc(varI_rr, &min, &max);
	std::cout << "Selective guided filter input min, max " << min << ", " << max << std::endl;
	*/

	cv::Mat invrr = varI_gg.mul(varI_bb) - varI_gb.mul(varI_gb);
	cv::Mat invrg = varI_gb.mul(varI_rb) - varI_rg.mul(varI_bb);
    cv::Mat invrb = varI_rg.mul(varI_gb) - varI_gg.mul(varI_rb);
    cv::Mat invgg = varI_rr.mul(varI_bb) - varI_rb.mul(varI_rb);
    cv::Mat invgb = varI_rb.mul(varI_rg) - varI_rr.mul(varI_gb);
    cv::Mat invbb = varI_rr.mul(varI_gg) - varI_rg.mul(varI_rg);

	cv::Mat covDet = invrr.mul(varI_rr) + invrg.mul(varI_rg) + invrb.mul(varI_rb);

    invrr /= covDet;
    invrg /= covDet;
    invrb /= covDet;
    invgg /= covDet;
    invgb /= covDet;
    invbb /= covDet;

	cv::Mat meanp;
	cv::Mat useMaskP = useMask & (~processMask);
	selectiveBoxFilter(pf, r, processMask, useMaskP, &meanp);

	cv::Mat mean_Ip_r;
	selectiveBoxFilter(I_r.mul(pf), r, processMask, useMaskP, &mean_Ip_r);
	cv::Mat mean_Ip_g;
	selectiveBoxFilter(I_g.mul(pf), r, processMask, useMaskP, &mean_Ip_g);
	cv::Mat mean_Ip_b;
	selectiveBoxFilter(I_b.mul(pf), r, processMask, useMaskP, &mean_Ip_b);

    cv::Mat cov_Ip_r = mean_Ip_r - meanI_r.mul(meanp);
    cv::Mat cov_Ip_g = mean_Ip_g - meanI_g.mul(meanp);
    cv::Mat cov_Ip_b = mean_Ip_b - meanI_b.mul(meanp);

    cv::Mat a_r = invrr.mul(cov_Ip_r) + invrg.mul(cov_Ip_g) + invrb.mul(cov_Ip_b);
    cv::Mat a_g = invrg.mul(cov_Ip_r) + invgg.mul(cov_Ip_g) + invgb.mul(cov_Ip_b);
    cv::Mat a_b = invrb.mul(cov_Ip_r) + invgb.mul(cov_Ip_g) + invbb.mul(cov_Ip_b);

    cv::Mat b = meanp - a_r.mul(meanI_r) - a_g.mul(meanI_g) - a_b.mul(meanI_b); // Eqn. (15) in the paper;
	
	cv::Mat meana_r, meana_g, meana_b, meanb;
	selectiveBoxFilter(a_r, r, processMask, useMask, &meana_r);
	selectiveBoxFilter(a_g, r, processMask, useMask, &meana_g);
	selectiveBoxFilter(a_b, r, processMask, useMask, &meana_b);
	selectiveBoxFilter(b, r, processMask, useMask, &meanb);

	cv::Mat outf =
		meana_r.mul(I_r) +
		meana_g.mul(I_g) +
		meana_b.mul(I_b) +
		meanb;

	//Copy to output matrix. Convert to input type if necessary.
	if(p.type() == CV_8UC1 || p.type() == CV_8UC3) {
		outf *= 255.f;
		outf.convertTo(*outputImage, CV_8U);
	} else if(p.type() == CV_32FC1 || p.type() == CV_32FC3) {
		*outputImage = outf.clone();
	}
}

void selectiveBoxFilter(
	const cv::Mat &inputImage, size_t rad,
	const cv::Mat &processMask, const cv::Mat &useMask,
	cv::Mat *outputImage)
{
	//Basic on2 for now.
	if (outputImage->size() != inputImage.size() ||
		outputImage->type() != inputImage.type()) {
		*outputImage = cv::Mat(inputImage.size(), inputImage.type());
	}
	for (size_t r = 0; r < inputImage.rows; ++r) {
		for (size_t c = 0; c < inputImage.cols; ++c) {
			size_t startR = r < rad ? 0 : r - rad;
			size_t endR = r > inputImage.rows-rad ? inputImage.rows-1 : r + rad;
			size_t startC = c < rad ? 0 : c - rad;
			size_t endC = c > inputImage.cols-rad ? inputImage.cols-1 : c + rad;
			size_t denom = 0;

			if(!processMask.at<unsigned char>(r,c)) {
				//This pixel shouldn't be processed.
				if (inputImage.type() == CV_32FC1) {
					outputImage->at<float>(r,c) = inputImage.at<float>(r,c);
				} else if(inputImage.type() == CV_32FC3) {
					outputImage->at<cv::Vec3f>(r,c) = inputImage.at<cv::Vec3f>(r,c);
				}
				//Move on to next pixel.
				continue;
			}

			if (inputImage.type() == CV_32FC1) {
				float num = 0;
				for (size_t r2 = startR; r2 < endR; ++r2) {
					for (size_t c2 = startC; c2 < endC; ++c2) {
						if (useMask.at<unsigned char>(r2, c2)) {
							denom++;
							num += inputImage.at<float>(r2, c2);
						}
					}
				}
				if (denom == 0) {
					outputImage->at<float>(r, c) = inputImage.at<float>(r,c);
				} else {
					outputImage->at<float>(r, c) = num / (float)(denom);
				}
			} else if (inputImage.type() == CV_32FC3) {
				cv::Vec3f num = 0;
				for (size_t r2 = startR; r2 < endR; ++r2) {
					for (size_t c2 = startC; c2 < endC; ++c2) {
						if (useMask.at<unsigned char>(r2, c2)) {
							denom++;
							num += inputImage.at<cv::Vec3f>(r, c);
						}
					}
				}
				if (denom == 0) {
					//Failure case - shouldn't get here unless issue with masks.
					outputImage->at<cv::Vec3f>(r, c) = inputImage.at<cv::Vec3f>(r,c);
				} else {
					outputImage->at<cv::Vec3f>(r, c) = num / (float)(denom);
				}
			} else {
				throw std::runtime_error("selectiveboxfilter not compatible with this mat type.");
			}
		}
	}
}

