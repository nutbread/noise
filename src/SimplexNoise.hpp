/*
	Templated SimplexNoise
*/
#ifndef ___H_SIMPLEX_NOISE
#define ___H_SIMPLEX_NOISE



#include <cstdint>
#include <cassert>



#if defined(NDEBUG) && defined(SIMPLEX_NOISE_FAST_MATH)
	// Check compiler
	#ifdef _MSC_VER
		// Visual C++ (not tested)
		#define FAST_MATH(...) \
			_Pragma("float_control(except, off)") \
			_Pragma("fenv_access(off)") \
			_Pragma("float_control(precise, off)") \
			_Pragma("fp_contract(on)") \
			__VA_ARGS__; \
			_Pragma("float_control(except, on)") \
			_Pragma("fenv_access(on)") \
			_Pragma("float_control(precise, on)") \
			_Pragma("fp_contract(off)")
	#else
		// Default: assume g++
		#define FAST_MATH(...) \
			__VA_ARGS__ \
			__attribute__((noinline, optimize("-ffast-math")));
	#endif
#else
	// No fast math
	#define FAST_MATH(...) \
		__VA_ARGS__;
#endif



// Class definitions
class SimplexNoiseBase;

template <typename Float>
class SimplexNoise;

template <typename Float, int Dimensions>
class SimplexNoiseHelper;



// Base class
class SimplexNoiseBase {
protected:
	static const unsigned char perm[];

};



// Primary class
template <typename Float>
class SimplexNoise : public SimplexNoiseBase {
public: // Public types
	typedef int32_t HashInt;
	typedef uint32_t OctavesInt;

public: // Public static methods
	template <typename Integer>
	static Integer fast_floor(
		Float x
	);

	template <typename Integer>
	static Integer fast_ceiling(
		Float x
	);

private: // Private instance members
	const unsigned char* perm;

public: // Public instance methods
	SimplexNoise();
	SimplexNoise(
		const unsigned char* perm
	);
	~SimplexNoise();

	// Noise functions
	FAST_MATH(Float noise1(Float x));
	FAST_MATH(Float noise1(Float x, Float* dx));
	FAST_MATH(Float noise1_octaves(OctavesInt octaves, Float x));
	FAST_MATH(Float noise1_octaves(OctavesInt octaves, Float x, Float* dx));

	FAST_MATH(Float noise2(Float x, Float y));
	FAST_MATH(Float noise2(Float x, Float y, Float* dx, Float* dy));
	FAST_MATH(Float noise2_octaves(OctavesInt octaves, Float x, Float y));
	FAST_MATH(Float noise2_octaves(OctavesInt octaves, Float x, Float y, Float* dx, Float* dy));

	FAST_MATH(Float noise3(Float x, Float y, Float z));
	FAST_MATH(Float noise3(Float x, Float y, Float z, Float* dx, Float* dy, Float* dz));
	FAST_MATH(Float noise3_octaves(OctavesInt octaves, Float x, Float y, Float z));
	FAST_MATH(Float noise3_octaves(OctavesInt octaves, Float x, Float y, Float z, Float* dx, Float* dy, Float* dz));

	FAST_MATH(Float noise4(Float x, Float y, Float z, Float w));
	FAST_MATH(Float noise4(Float x, Float y, Float z, Float w, Float* dx, Float* dy, Float* dz, Float* dw));
	FAST_MATH(Float noise4_octaves(OctavesInt octaves, Float x, Float y, Float z, Float w));
	FAST_MATH(Float noise4_octaves(OctavesInt octaves, Float x, Float y, Float z, Float w, Float* dx, Float* dy, Float* dz, Float* dw));

	FAST_MATH(Float noise_n(Float maxRadius, Float power, int dimensions, const Float* position));

};



// Helper classes
template <typename Float>
class SimplexNoiseGenericHelper final {
public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	static Float
	skewFactor(
		int dimensions
	);

	static Float
	deskewFactor(
		int dimensions
	);

	static void
	skewPoint(
		int dimensions,
		Float skewFactor,
		Float* position
	);

	static void
	deskewPoint(
		int dimensions,
		Float deskewFactor,
		HashInt positionSkewSum,
		const HashInt* positionSkew,
		Float* position
	);

	static Float
	dot(
		int dimensions,
		const Float* point1,
		const Float* point2
	);

	static void
	gradient(
		int dimensions,
		const unsigned char* permutationArray,
		const HashInt* position,
		const HashInt* offset,
		Float* result
	);

};

template <typename Float, int Dimensions>
class SimplexNoiseHelper final {
public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	/*
		skew_factor = (sqrt(n + 1) - 1) / n;
		deskew_factor = (n + 1 - sqrt(n + 1)) / (n * (n + 1));
	*/

	static constexpr Float
	skewFactor();

	static constexpr Float
	deskewFactor();

};

template <typename Float>
class SimplexNoiseHelper<Float, 1> final {
public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	static constexpr Float
	skewFactor();

	static constexpr Float
	deskewFactor();

	static void
	gradient(
		HashInt hash,
		Float& result
	);

	static constexpr Float
	gradientMaximum();

	static constexpr Float
	unnormalizedMaximum();

};

template <typename Float>
class SimplexNoiseHelper<Float, 2> final {
private:
	static const signed char gradientTable[][2];

public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	static constexpr Float
	skewFactor();

	static constexpr Float
	deskewFactor();

	static void
	skewPoint(
		Float& x,
		Float& y
	);

	static void
	deskewPoint(
		HashInt skewSum,
		HashInt xskew,
		HashInt yskew,
		Float& x,
		Float& y
	);

	static HashInt
	gradientHash(
		const unsigned char* permutationArray,
		HashInt x,
		HashInt y
	);

	static void
	gradient(
		HashInt hash,
		Float& result1,
		Float& result2
	);

	static constexpr Float
	unnormalizedMaximum();

};

template <typename Float>
class SimplexNoiseHelper<Float, 3> final {
private:
	friend class SimplexNoise<Float>;
	static const signed char gradientTable[][3];
	static const unsigned char simplexOrders[][2][3];

public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	static constexpr Float
	skewFactor();

	static constexpr Float
	deskewFactor();

	static void
	skewPoint(
		Float& x,
		Float& y,
		Float& z
	);

	static void
	deskewPoint(
		HashInt skewSum,
		HashInt xskew,
		HashInt yskew,
		HashInt zskew,
		Float& x,
		Float& y,
		Float& z
	);

	static HashInt
	gradientHash(
		const unsigned char* permutationArray,
		HashInt x,
		HashInt y,
		HashInt z
	);

	static void
	gradient(
		HashInt hash,
		Float& result1,
		Float& result2,
		Float& result3
	);

	static constexpr Float
	unnormalizedMaximum();

};

template <typename Float>
class SimplexNoiseHelper<Float, 4> final {
private:
	static const signed char gradientTable[][4];

public:
	typedef typename SimplexNoise<Float>::HashInt HashInt;

	static constexpr Float
	skewFactor();

	static constexpr Float
	deskewFactor();

	static void
	skewPoint(
		Float* xyzw
	);

	static void
	deskewPoint(
		HashInt xyzwSkewSum,
		const HashInt* xyzwSkew,
		Float* xyzw
	);

	static inline Float
	dot(
		const Float* point1,
		const Float* point2
	);

	static HashInt
	gradientHash(
		const unsigned char* permutationArray,
		const HashInt* xyzw
	);

	static HashInt
	gradientHash(
		const unsigned char* permutationArray,
		const HashInt* xyzw,
		const HashInt* offset
	);

	static void
	gradient(
		HashInt hash,
		Float* result
	);

	static constexpr Float
	unnormalizedMaximum();

};



// Template implementations
#include "SimplexNoise.t.hpp"



#undef FAST_MATH



#endif // ___H_SIMPLEX_NOISE


