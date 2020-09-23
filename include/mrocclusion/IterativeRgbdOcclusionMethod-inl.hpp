
//Status of pixels surrounding a processed pixels.
enum PixelStatus : char {
	MISSING,  //Pixel value is missing, should be ignored.
	FIXED,    //Pixel is known fg/bg pixel, matte value should not be changed.
	UNKNOWN
}; //Pixel value is unknown, need to find matte value.

template<typename T> T polyError(T e) {
	T em1 = e - T(1.0);
	return T(1.0) - (em1*em1);
}

template<typename T> T logError(T e) {
	//return T(1.0) + ceres::log(T(1.0/M_E) + e);
	return 4.0 * (e - (e * e));
}

template<typename T> T lowHighError(T e) {
	return e - e*e;
}

inline uchar absdiff(uchar a, uchar b) {
	return a < b ? b - a : a - b;
}

inline float colorDifference(const uchar *c1, const uchar *c2) {
	float diff = 0.f;
	diff += absdiff(c1[0], c2[0]);
	diff += absdiff(c1[1], c2[1]);
	diff += absdiff(c1[2], c2[2]);
	return diff;
}

//Gradient cost function object.
struct GradientCostFunctor
{
	double weight;
	PixelStatus s[4];
	double fixedValues[4];

	GradientCostFunctor(
		double weight, PixelStatus *pixelStatus, float *fixedValue)
		:weight(weight)
	{
		for (size_t i = 0; i < 4; ++i) {
			s[i] = pixelStatus[i];
			fixedValues[i] = fixedValue[i];
		}
	}

	template <typename T> bool operator() (
		const T *const valueHere,
		const T *const valueAbove,
		const T *const valueLeft,
		const T *const valueBelow,
		const T *const valueRight,
		T *residual) const
	{
		T vals[4] = { *valueAbove, *valueLeft, *valueBelow, *valueRight };
		bool missing[4] = { false, false, false, false };
		for (size_t i = 0; i < 4; ++i) {
			if (s[i] == UNKNOWN) {

			}
			else if (s[i] == FIXED) {
				vals[i] = T(fixedValues[i]);
			}
			else /* s[i] == MISSING */ {
				missing[i] = true;
			}
		}

		T sum(0.0);
		for (size_t i = 0; i < 4; ++i) {
			if (!missing[i]) {
				T diff = vals[i] - *valueHere;
				sum += diff * diff;
			}
		}
		residual[0] = weight * logError(sum * 0.5);
		return true;
	}
};

//Colour error function object.
struct ColorSimilarityCostFunctor
{
	double weight;
	PixelStatus s[4];
	double colorDiffs[4];

	ColorSimilarityCostFunctor(
		double weight,
		const uchar *colorHere,
		const uchar *colorAbove,
		const uchar *colorLeft,
		const uchar *colorBelow,
		const uchar *colorRight,
		const PixelStatus *status)
		:weight(weight)
	{
		for (size_t i = 0; i < 4; ++i) {
			s[i] = status[i];
		}
		colorDiffs[0] = colorDifference(colorHere, colorAbove);
		colorDiffs[1] = colorDifference(colorHere, colorLeft);
		colorDiffs[2] = colorDifference(colorHere, colorBelow);
		colorDiffs[3] = colorDifference(colorHere, colorRight);
	}

	template <typename T> bool operator() (
		const T *const valueHere,
		const T *const valueAbove,
		const T *const valueLeft,
		const T *const valueBelow,
		const T *const valueRight,
		T *residual) const
	{
		T vals[4] = { *valueAbove, *valueLeft, *valueBelow, *valueRight };
		*residual = T(0.0);
		for (size_t i = 0; i < 4; ++i) {
			T matteDiff = (*valueHere - vals[i]);
			matteDiff *= matteDiff;
			*residual = (1 - colorDiffs[i]) * matteDiff;
		}
		*residual *= T(weight);
		return true;
	}
};
