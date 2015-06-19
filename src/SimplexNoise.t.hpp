/*
	Templated SimplexNoise
*/
#include <cassert>
#include <limits>
#include <type_traits>
#include <cmath>
#include <iostream>
using namespace std;



// SimplexNoiseGenericHelper<Float>
template <typename Float>
Float
SimplexNoiseGenericHelper<Float> :: skewFactor(
	int dimensions
) {
	return static_cast<Float>((::sqrt(dimensions + 1.0) - 1.0) / dimensions);
}

template <typename Float>
Float
SimplexNoiseGenericHelper<Float> :: deskewFactor(
	int dimensions
) {
	return static_cast<Float>((dimensions + 1.0 - ::sqrt(dimensions + 1.0)) / (dimensions * (dimensions + 1)));
}

template <typename Float>
void
SimplexNoiseGenericHelper<Float> :: skewPoint(
	int dimensions,
	Float skewFactor,
	Float* position
) {
	assert(dimensions > 0);
	assert(position != nullptr);

	Float skew = position[0];
	for (int i = 1; i < dimensions; ++i) {
		skew += position[i];
	}
	skew *= skewFactor;

	for (int i = 0; i < dimensions; ++i) {
		position[i] += skew;
	}
}

template <typename Float>
void
SimplexNoiseGenericHelper<Float> :: deskewPoint(
	int dimensions,
	Float deskewFactor,
	HashInt positionSkewSum,
	const HashInt* positionSkew,
	Float* position
) {
	assert(dimensions > 0);
	assert(positionSkew != nullptr);
	assert(position != nullptr);
	Float positionSkewSumDebug = positionSkew[0];
	for (int i = 1; i < dimensions; ++i) {
		positionSkewSumDebug += positionSkew[i];
	}
	assert(positionSkewSumDebug == positionSkewSum);

	Float skew = positionSkewSum * deskewFactor;

	for (int i = 0; i < dimensions; ++i) {
		position[i] -= positionSkew[i];
		position[i] += skew;
	}
}

template <typename Float>
Float
SimplexNoiseGenericHelper<Float> :: dot(
	int dimensions,
	const Float* point1,
	const Float* point2
) {
	assert(dimensions > 0);
	assert(point1 != nullptr);
	assert(point2 != nullptr);

	Float d = point1[0] * point2[0];
	for (int i = 1; i < dimensions; ++i) {
		d += point1[i] * point2[i];
	}
	return d;
}

template <typename Float>
void
SimplexNoiseGenericHelper<Float> :: gradient(
	int dimensions,
	const unsigned char* permutationArray,
	const HashInt* position,
	const HashInt* offset,
	Float* result
) {
	assert(dimensions > 0);
	assert(permutationArray != nullptr);
	assert(position != nullptr);
	assert(result != nullptr);

	if (dimensions == 2) {
		// Special case since this method doesn't work well for 2d
		if (offset == nullptr) {
			SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(permutationArray, position[0], position[1]), result[0], result[1]);
		}
		else {
			SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(permutationArray, position[0] + offset[0], position[1] + offset[1]), result[0], result[1]);
		}

		return;
	}

	uint32_t bits = dimensions - 1;
	uint32_t bytes = ((bits - 1) / (sizeof(uint32_t) * 8)) + 1;
	uint32_t* mask = new uint32_t[bytes];
	uint32_t zeroPos = 0;

	int i, k;
	uint32_t j, b;
	HashInt h;

	for (j = 0; j < bytes; ++j) {
		mask[j] = 0;
	}

	// Form the hash for the gradient
	for (i = 0; i < dimensions; ++i) {
		h = position[i];
		if (offset != nullptr) h += offset[i];

		for (j = 0; j < bytes; ++j) {
			b = 0;
			for (k = 0; k < sizeof(uint32_t) * 8; k += sizeof(unsigned char) * 8) {
				b |= permutationArray[h] << k;
			}
			mask[j] ^= b;
		}

		for (k = 0; k < sizeof(uint32_t) * 8; k += sizeof(unsigned char) * 8) {
			zeroPos |= permutationArray[h] << k;
		}
	}

	// Set the gradient
	zeroPos %= dimensions;
	for (j = 0; j < zeroPos; ++j) {
		*result = (mask[j / (sizeof(uint32_t) * 8)] & (1 << (j % (sizeof(uint32_t) * 8)))) == 0 ? -1 : 1;
		++result;
	}
	*result = 0;
	++result;
	++j;
	for (; j < dimensions; ++j) {
		*result = (mask[j / (sizeof(uint32_t) * 8)] & (1 << (j % (sizeof(uint32_t) * 8)))) == 0 ? -1 : 1;
		++result;
	}
}



// SimplexNoiseHelper<Float, Dimensions>
template <typename Float, int Dimensions>
constexpr Float
SimplexNoiseHelper<Float, Dimensions> :: skewFactor() {
	return static_cast<Float>((::sqrt(Dimensions + 1.0) - 1.0) / Dimensions);
}

template <typename Float, int Dimensions>
constexpr Float
SimplexNoiseHelper<Float, Dimensions> :: deskewFactor() {
	return static_cast<Float>((Dimensions + 1.0 - ::sqrt(Dimensions + 1.0)) / (Dimensions * (Dimensions + 1)));
}



// SimplexNoiseHelper<Float, 1>
template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 1> :: skewFactor() {
	return static_cast<Float>(1.4142135623730951 - 1.0); // (sqrt(1 + 1) - 1) / 1
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 1> :: deskewFactor() {
	return static_cast<Float>(1.0 - 1.4142135623730951 / 2.0); // (1 + 1 - sqrt(1 + 1)) / (1 * (1 + 1))
}

template <typename Float>
void
SimplexNoiseHelper<Float, 1> :: gradient(
	HashInt hash,
	Float& result
) {
	result = 1 + (hash & 0x7); // Gradient value 1.0, 2.0, ..., 8.0
	if (hash & 0x8) result = -result; // Set a random sign for the gradient
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 1> :: gradientMaximum() {
	return static_cast<Float>(8.0);
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 1> :: unnormalizedMaximum() {
	// Max value is: 8*(3/4)^4 = 2.53125
	return (gradientMaximum() * static_cast<Float>(81.0) / static_cast<Float>(256.0));
}



// SimplexNoiseHelper<Float, 2>
template <typename Float>
const signed char
SimplexNoiseHelper<Float, 2> :: gradientTable[][2] = {
	{-1,-1}, {-1,1}, {1,-1}, {1,1},
	{0,-1}, {0,1}, {-1,0}, {1,0},
};

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 2> :: skewFactor() {
	return static_cast<Float>((1.7320508075688772 - 1.0) / 2.0); // (sqrt(2 + 1) - 1) / 2
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 2> :: deskewFactor() {
	return static_cast<Float>((3.0 - 1.7320508075688772) / 6.0); // (2 + 1 - sqrt(2 + 1)) / (2 * (2 + 1))
}

template <typename Float>
void
SimplexNoiseHelper<Float, 2> :: skewPoint(
	Float& x,
	Float& y
) {
	Float skew = (x + y) * skewFactor();
	x += skew;
	y += skew;
}

template <typename Float>
void
SimplexNoiseHelper<Float, 2> :: deskewPoint(
	HashInt skewSum,
	HashInt xskew,
	HashInt yskew,
	Float& x,
	Float& y
) {
	assert(xskew + yskew == skewSum);

	Float skew = skewSum * deskewFactor();

	x -= xskew;
	x += skew;

	y -= yskew;
	y += skew;
}

template <typename Float>
typename SimplexNoiseHelper<Float, 2>::HashInt
SimplexNoiseHelper<Float, 2> :: gradientHash(
	const unsigned char* permutationArray,
	HashInt x,
	HashInt y
) {
	assert(permutationArray != nullptr);
	assert(x >= 0);
	assert(x <= 256);
	assert(y >= 0);
	assert(y <= 256);

	return permutationArray[x + permutationArray[y]];
}

template <typename Float>
void
SimplexNoiseHelper<Float, 2> :: gradient(
	HashInt hash,
	Float& result1,
	Float& result2
) {
	hash &= 0x7;
	result1 = gradientTable[hash][0];
	result2 = gradientTable[hash][1];
	/*
	hash &= 0x7;
	++hash;
	result1 = (((hash / 3) + 1) % 3) - 1;
	result2 = (((hash % 3) + 1) % 3) - 1;
	*/
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 2> :: unnormalizedMaximum() {
	// This is an approximated value achieved by brute force testing
	return 0.01425556220221299;
}



// SimplexNoiseHelper<Float, 3>
template <typename Float>
const signed char
SimplexNoiseHelper<Float, 3> :: gradientTable[][3] = {
	{0,-1,-1}, {0,-1,1}, {0,1,-1}, {0,1,1},
	{-1,0,-1}, {-1,0,1}, {1,0,-1}, {1,0,1},
	{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0},
};

template <typename Float>
const unsigned char
SimplexNoiseHelper<Float, 3> :: simplexOrders[][2][3] = {
	{ {0,0,1}, {0,1,1} }, // 0
	{ {0,0,1}, {1,0,1} }, // 1
	{ {0,0,0}, {0,0,0} }, // 2 (invalid)
	{ {1,0,0}, {1,0,1} }, // 3
	{ {0,1,0}, {0,1,1} }, // 4
	{ {0,0,0}, {0,0,0} }, // 5 (invalid)
	{ {0,1,0}, {1,1,0} }, // 6
	{ {1,0,0}, {1,1,0} }, // 7
};

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 3> :: skewFactor() {
	return static_cast<Float>(1.0 / 3.0); // (sqrt(3 + 1) - 1) / 3
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 3> :: deskewFactor() {
	return static_cast<Float>(1.0 / 6.0); // (3 + 1 - sqrt(3 + 1)) / (3 * (3 + 1))
}

template <typename Float>
void
SimplexNoiseHelper<Float, 3> :: skewPoint(
	Float& x,
	Float& y,
	Float& z
) {
	Float skew = (x + y + z) * skewFactor();
	x += skew;
	y += skew;
	z += skew;
}

template <typename Float>
void
SimplexNoiseHelper<Float, 3> :: deskewPoint(
	HashInt skewSum,
	HashInt xskew,
	HashInt yskew,
	HashInt zskew,
	Float& x,
	Float& y,
	Float& z
) {
	assert(xskew + yskew + zskew == skewSum);

	Float skew = skewSum * deskewFactor();

	x -= xskew;
	x += skew;

	y -= yskew;
	y += skew;

	z -= zskew;
	z += skew;
}

template <typename Float>
typename SimplexNoiseHelper<Float, 3>::HashInt
SimplexNoiseHelper<Float, 3> :: gradientHash(
	const unsigned char* permutationArray,
	HashInt x,
	HashInt y,
	HashInt z
) {
	assert(permutationArray != nullptr);
	assert(x >= 0);
	assert(x <= 256);
	assert(y >= 0);
	assert(y <= 256);
	assert(z >= 0);
	assert(z <= 256);

	return permutationArray[x + permutationArray[y + permutationArray[z]]];
}

template <typename Float>
void
SimplexNoiseHelper<Float, 3> :: gradient(
	HashInt hash,
	Float& result1,
	Float& result2,
	Float& result3
) {
	hash &= 0xB;
	result1 = gradientTable[hash][0];
	result2 = gradientTable[hash][1];
	result3 = gradientTable[hash][2];
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 3> :: unnormalizedMaximum() {
	// This is an approximated value achieved by brute force testing
	return 0.03199015150473494;
}



// SimplexNoiseHelper<Float, 4>
template <typename Float>
const signed char
SimplexNoiseHelper<Float, 4> :: gradientTable[][4] = {
	{0,-1,-1,-1}, {0,-1,-1,1}, {0,-1,1,-1}, {0,-1,1,1}, {0,1,-1,-1}, {0,1,-1,1}, {0,1,1,-1}, {0,1,1,1},
	{-1,0,-1,-1}, {-1,0,-1,1}, {-1,0,1,-1}, {-1,0,1,1}, {1,0,-1,-1}, {1,0,-1,1}, {1,0,1,-1}, {1,0,1,1},
	{-1,-1,0,-1}, {-1,-1,0,1}, {-1,1,0,-1}, {-1,1,0,1}, {1,-1,0,-1}, {1,-1,0,1}, {1,1,0,-1}, {1,1,0,1},
	{-1,-1,-1,0}, {-1,-1,1,0}, {-1,1,-1,0}, {-1,1,1,0}, {1,-1,-1,0}, {1,-1,1,0}, {1,1,-1,0}, {1,1,1,0},
};

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 4> :: skewFactor() {
	return static_cast<Float>((2.23606797749979 - 1.0) / 4.0); // (sqrt(4 + 1) - 1) / 4
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 4> :: deskewFactor() {
	return static_cast<Float>((5.0 - 2.23606797749979) / 20.0); // (4 + 1 - sqrt(4 + 1)) / (4 * (4 + 1))
}

template <typename Float>
void
SimplexNoiseHelper<Float, 4> :: skewPoint(
	Float* xyzw
) {
	assert(xyzw != nullptr);

	Float skew = (xyzw[0] + xyzw[1] + xyzw[2] + xyzw[3]) * skewFactor();
	xyzw[0] += skew;
	xyzw[1] += skew;
	xyzw[2] += skew;
	xyzw[3] += skew;
}

template <typename Float>
void
SimplexNoiseHelper<Float, 4> :: deskewPoint(
	HashInt xyzwSkewSum,
	const HashInt* xyzwSkew,
	Float* xyzw
) {
	assert(xyzwSkew != nullptr);
	assert(xyzw != nullptr);
	assert(xyzwSkew[0] + xyzwSkew[1] + xyzwSkew[2] + xyzwSkew[3] == xyzwSkewSum);

	Float skew = xyzwSkewSum * deskewFactor();

	xyzw[0] -= xyzwSkew[0];
	xyzw[0] += skew;

	xyzw[1] -= xyzwSkew[1];
	xyzw[1] += skew;

	xyzw[2] -= xyzwSkew[2];
	xyzw[2] += skew;

	xyzw[3] -= xyzwSkew[3];
	xyzw[3] += skew;
}

template <typename Float>
inline Float
SimplexNoiseHelper<Float, 4> :: dot(
	const Float* point1,
	const Float* point2
) {
	assert(point1 != nullptr);
	assert(point2 != nullptr);
	return (
		point1[0] * point2[0] +
		point1[1] * point2[1] +
		point1[2] * point2[2] +
		point1[3] * point2[3]
	);
}

template <typename Float>
typename SimplexNoiseHelper<Float, 4>::HashInt
SimplexNoiseHelper<Float, 4> :: gradientHash(
	const unsigned char* permutationArray,
	const HashInt* xyzw
) {
	assert(permutationArray != nullptr);
	assert(xyzw != nullptr);
	assert(xyzw[0] >= 0);
	assert(xyzw[0] < 256);
	assert(xyzw[1] >= 0);
	assert(xyzw[1] < 256);
	assert(xyzw[2] >= 0);
	assert(xyzw[2] < 256);
	assert(xyzw[3] >= 0);
	assert(xyzw[3] < 256);

	return
		permutationArray[xyzw[0] +
		permutationArray[xyzw[1] +
		permutationArray[xyzw[2] +
		permutationArray[xyzw[3]
		]]]];
}

template <typename Float>
typename SimplexNoiseHelper<Float, 4>::HashInt
SimplexNoiseHelper<Float, 4> :: gradientHash(
	const unsigned char* permutationArray,
	const HashInt* xyzw,
	const HashInt* offset
) {
	assert(permutationArray != nullptr);
	assert(xyzw != nullptr);
	assert(offset != nullptr);
	assert(xyzw[0] + offset[0] >= 0);
	assert(xyzw[0] + offset[0] <= 256);
	assert(xyzw[1] + offset[0] >= 0);
	assert(xyzw[1] + offset[0] <= 256);
	assert(xyzw[2] + offset[0] >= 0);
	assert(xyzw[2] + offset[0] <= 256);
	assert(xyzw[3] + offset[0] >= 0);
	assert(xyzw[3] + offset[0] <= 256);

	return
		permutationArray[xyzw[0] + offset[0] +
		permutationArray[xyzw[1] + offset[1] +
		permutationArray[xyzw[2] + offset[2] +
		permutationArray[xyzw[3] + offset[3]
		]]]];
}

template <typename Float>
void
SimplexNoiseHelper<Float, 4> :: gradient(
	HashInt hash,
	Float* result
) {
	assert(result != nullptr);

	const signed char* gt = gradientTable[hash & 0x1F];
	result[0] = gt[0];
	result[1] = gt[1];
	result[2] = gt[2];
	result[3] = gt[3];
}

template <typename Float>
constexpr Float
SimplexNoiseHelper<Float, 4> :: unnormalizedMaximum() {
	// This is an approximated value achieved by brute force testing
	return 0.03718810613189993;
}



// SimplexNoise<Float>
template <typename Float>
SimplexNoise<Float> :: SimplexNoise() :
	perm(SimplexNoiseBase::perm)
{
}

template <typename Float>
SimplexNoise<Float> :: SimplexNoise(
	const unsigned char* perm
) :
	perm(perm)
{
	assert(perm != nullptr);
}

template <typename Float>
SimplexNoise<Float> :: ~SimplexNoise() {
}



// fast rounding functions
template <typename Float>
template <typename Integer>
Integer
SimplexNoise<Float> :: fast_floor(
	Float x
) {
	typedef typename std::make_signed<Integer>::type signed_int;
	typedef typename std::make_unsigned<Integer>::type unsigned_int;

	//assert(x >= static_cast<Float>(std::numeric_limits<signed_int>::min()));
	//assert(x <= static_cast<Float>(std::numeric_limits<signed_int>::max()));

	return static_cast<signed_int>(static_cast<unsigned_int>(x - static_cast<Float>(std::numeric_limits<signed_int>::min()))) + std::numeric_limits<signed_int>::min();
}

template <typename Float>
template <typename Integer>
Integer
SimplexNoise<Float> :: fast_ceiling(
	Float x
) {
	// ceiling(x) = -floor(-x)
	// return -fast_floor(-x);

	typedef typename std::make_signed<Integer>::type signed_int;
	typedef typename std::make_unsigned<Integer>::type unsigned_int;

	//assert(x >= static_cast<Float>(std::numeric_limits<signed_int>::min()));
	//assert(x <= static_cast<Float>(std::numeric_limits<signed_int>::max()));

	return std::numeric_limits<signed_int>::max() - static_cast<signed_int>(static_cast<unsigned_int>(static_cast<Float>(std::numeric_limits<signed_int>::max()) - x));
}



// 1d noise
template <typename Float>
Float
SimplexNoise<Float> :: noise1(
	Float x
) {
	HashInt i0 = SimplexNoise<Float>::fast_floor<HashInt>(x);

	Float x0, x1;
	Float t0, t1;
	Float g;

	SimplexNoiseHelper<Float, 1>::gradient(this->perm[i0 & 0xFF], g);
	x0 = x - i0;
	t0 = 1 - x0 * x0;
	t0 *= t0;
	t0 *= t0;
	t0 *= g * x0;

	SimplexNoiseHelper<Float, 1>::gradient(this->perm[(i0 + 1) & 0xFF], g);
	x1 = x0 - 1;
	t1 = 1 - x1 * x1;
	t1 *= t1;
	t1 *= t1;
	t1 *= g * x1;

	return (t0 + t1) / SimplexNoiseHelper<Float, 1>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise1(
	Float x,
	Float* dx
) {
	assert(dx != nullptr);

	HashInt i0 = SimplexNoise<Float>::fast_floor<HashInt>(x);

	Float x0, x1;
	Float x0_2, x1_2;
	Float t0, t1;
	Float t0_2, t1_2;
	Float g0, g1;

	SimplexNoiseHelper<Float, 1>::gradient(this->perm[i0 & 0xFF], g0);
	x0 = x - i0;
	x0_2 = x0 * x0;
	t0 = 1 - x0_2;
	t0_2 = t0 * t0;
	t0 = t0_2 * t0_2;
	t0 *= g0 * x0;

	SimplexNoiseHelper<Float, 1>::gradient(this->perm[(i0 + 1) & 0xFF], g1);
	x1 = x0 - 1;
	x1_2 = x1 * x1;
	t1 = 1 - x1_2;
	t1_2 = t1 * t1;
	t1 = t1_2 * t1_2;
	t1 *= g1 * x1;

	// fn'(x) = (g0 * (x0**2 - 1)**3 * (9*x0**2 - 1) + g1 * (x1**2 - 1)**3 * (9*x1**2 - 1)) / maximum
	*dx = g0 * t0_2 * (x0_2 - 1) * (9 * x0_2 - 1);
	*dx += g1 * t1_2 * (x1_2 - 1) * (9 * x1_2 - 1);
	*dx /= SimplexNoiseHelper<Float, 1>::unnormalizedMaximum();

	return (t0 + t1) / SimplexNoiseHelper<Float, 1>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise1_octaves(
	OctavesInt octaves,
	Float x
) {
	if (octaves <= 0) return 0;

	Float n = noise1(x);
	Float scale = 1;
	Float total = 1;

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise1(x / scale) * scale;
	}

	return n / total;
}

template <typename Float>
Float
SimplexNoise<Float> :: noise1_octaves(
	OctavesInt octaves,
	Float x,
	Float* dx
) {
	assert(dx != nullptr);

	if (octaves <= 0) {
		*dx = 0;
		return 0;
	}

	Float n = noise1(x, dx);
	Float scale = 1;
	Float total = 1;
	Float dx2;

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise1(x / scale, &dx2) * scale;
		*dx += dx2 * scale;
	}

	*dx /= total;
	return n / total;
}



// 2d noise
template <typename Float>
Float
SimplexNoise<Float> :: noise2(
	Float x,
	Float y
) {
	// fn(x,y) = sum((max(0, 0.5 - dot(xy[i], xy[i])) ^ 4) * dot(gradient[i], xy[i]), i, 0, 2)

	// Skew point and find origin
	Float sx = x;
	Float sy = y;
	SimplexNoiseHelper<Float, 2>::skewPoint(sx, sy);
	HashInt si = SimplexNoise<Float>::fast_floor<HashInt>(sx);
	HashInt sj = SimplexNoise<Float>::fast_floor<HashInt>(sy);

	// The x,y relative distances from the deskewed origin
	Float rx0 = x;
	Float ry0 = y;
	SimplexNoiseHelper<Float, 2>::deskewPoint((si + sj), si, sj, rx0, ry0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	si &= 0xFF;
	sj &= 0xFF;

	// Simplex second point order
	// simplex0, simplex1 = (rx0 > ry0) ? (1, 0) : (0, 1);
	bool simplex0 = (rx0 >= ry0);
	bool simplex1 = !simplex0;
	Float rx1, ry1;
	Float gx, gy;
	Float c, c0;

	// Contribution 1
	c0 = static_cast<Float>(0.5) - rx0 * rx0 - ry0 * ry0;
	if (c0 <= 0) {
		c0 = 0;
	}
	else {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si, sj), gx, gy);
		c0 *= c0;
		c0 *= c0;
		c0 *= (gx * rx0 + gy * ry0);
	}

	// Contribution 2
	rx1 = rx0;
	ry1 = ry0;
	SimplexNoiseHelper<Float, 2>::deskewPoint(1, simplex0, simplex1, rx1, ry1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si + simplex0, sj + simplex1), gx, gy);
		c *= c;
		c *= c;
		c *= (gx * rx1 + gy * ry1);

		c0 += c;
	}

	// Contribution 3
	rx1 = rx0;
	ry1 = ry0;
	SimplexNoiseHelper<Float, 2>::deskewPoint(2, 1, 1, rx1, ry1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si + 1, sj + 1), gx, gy);
		c *= c;
		c *= c;
		c *= (gx * rx1 + gy * ry1);

		c0 += c;
	}

	// Done
	return c0 / SimplexNoiseHelper<Float, 2>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise2(
	Float x,
	Float y,
	Float* dx,
	Float* dy
) {
	assert(dx != nullptr);
	assert(dy != nullptr);

	// fn(x,y) = sum((max(0, 0.5 - dot(xy[i], xy[i])) ^ 4) * dot(gradient[i], xy[i]), i, 0, 2)

	// Skew point and find origin
	Float sx = x;
	Float sy = y;
	SimplexNoiseHelper<Float, 2>::skewPoint(sx, sy);
	HashInt si = SimplexNoise<Float>::fast_floor<HashInt>(sx);
	HashInt sj = SimplexNoise<Float>::fast_floor<HashInt>(sy);

	// The x,y relative distances from the deskewed origin
	Float rx0 = x;
	Float ry0 = y;
	SimplexNoiseHelper<Float, 2>::deskewPoint((si + sj), si, sj, rx0, ry0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	si &= 0xFF;
	sj &= 0xFF;

	// Simplex second point order
	// simplex0, simplex1 = (rx0 > ry0) ? (1, 0) : (0, 1);
	bool simplex0 = (rx0 >= ry0);
	bool simplex1 = !simplex0;
	Float dx2, dy2;
	Float rx1, ry1;
	Float gx, gy;
	Float c, c0, c1, dot;

	// Contribution 1
	c0 = static_cast<Float>(0.5) - rx0 * rx0 - ry0 * ry0;
	if (c0 <= 0) {
		c0 = 0;
		*dx = 0;
		*dy = 0;
		dx2 = 0;
		dy2 = 0;
	}
	else {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si, sj), gx, gy);

		dot = (gx * rx0 + gy * ry0);
		c1 = c0;

		c0 *= c0;
		*dx = rx0 * dot * c0 * c1;
		*dy = ry0 * dot * c0 * c1;

		c0 *= c0;
		dx2 = gx * c0;
		dy2 = gy * c0;

		c0 *= dot;
	}

	// Contribution 2
	rx1 = rx0;
	ry1 = ry0;
	SimplexNoiseHelper<Float, 2>::deskewPoint(1, simplex0, simplex1, rx1, ry1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si + simplex0, sj + simplex1), gx, gy);

		dot = (gx * rx1 + gy * ry1);
		c1 = c;

		c *= c;
		*dx += rx1 * dot * c * c1;
		*dy += ry1 * dot * c * c1;

		c *= c;
		dx2 += gx * c;
		dy2 += gy * c;

		c *= dot;

		c0 += c;
	}

	// Contribution 3
	rx1 = rx0;
	ry1 = ry0;
	SimplexNoiseHelper<Float, 2>::deskewPoint(2, 1, 1, rx1, ry1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 2>::gradient(SimplexNoiseHelper<Float, 2>::gradientHash(this->perm, si + 1, sj + 1), gx, gy);

		dot = (gx * rx1 + gy * ry1);
		c1 = c;

		c *= c;
		*dx += rx1 * dot * c * c1;
		*dy += ry1 * dot * c * c1;

		c *= c;
		dx2 += gx * c;
		dy2 += gy * c;

		c *= dot;

		c0 += c;
	}

	// Complete derivative
	*dx *= -8;
	*dy *= -8;
	*dx += dx2;
	*dy += dy2;
	*dx /= SimplexNoiseHelper<Float, 2>::unnormalizedMaximum();
	*dy /= SimplexNoiseHelper<Float, 2>::unnormalizedMaximum();

	// Done
	return c0 / SimplexNoiseHelper<Float, 2>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise2_octaves(
	OctavesInt octaves,
	Float x,
	Float y
) {
	if (octaves <= 0) return 0;

	Float n = noise2(x, y);
	Float scale = 1;
	Float total = 1;

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise2(x / scale, y / scale) * scale;
	}

	return n / total;
}

template <typename Float>
Float
SimplexNoise<Float> :: noise2_octaves(
	OctavesInt octaves,
	Float x,
	Float y,
	Float* dx,
	Float* dy
) {
	assert(dx != nullptr);
	assert(dy != nullptr);

	if (octaves <= 0) {
		*dx = 0;
		*dy = 0;
		return 0;
	}

	Float n = noise2(x, y, dx, dy);
	Float scale = 1;
	Float total = 1;
	Float d2[2];

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise2(x / scale, y / scale, &d2[0], &d2[1]) * scale;
		*dx += d2[0] * scale;
		*dy += d2[1] * scale;
	}

	*dx /= total;
	*dy /= total;
	return n / total;
}



// 3d noise
template <typename Float>
Float
SimplexNoise<Float> :: noise3(
	Float x,
	Float y,
	Float z
) {
	// fn(x,y,z) = sum((max(0, 0.5 - dot(xyz[i], xyz[i])) ^ 3) * dot(gradient[i], xyz[i]), i, 0, 3)

	// Skew point and find origin
	Float sx = x;
	Float sy = y;
	Float sz = z;
	SimplexNoiseHelper<Float, 3>::skewPoint(sx, sy, sz);
	HashInt si = SimplexNoise<Float>::fast_floor<HashInt>(sx);
	HashInt sj = SimplexNoise<Float>::fast_floor<HashInt>(sy);
	HashInt sk = SimplexNoise<Float>::fast_floor<HashInt>(sz);

	// The x,y,z relative distances from the deskewed origin
	Float rx0 = x;
	Float ry0 = y;
	Float rz0 = z;
	SimplexNoiseHelper<Float, 3>::deskewPoint((si + sj + sk), si, sj, sk, rx0, ry0, rz0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	si &= 0xFF;
	sj &= 0xFF;
	sk &= 0xFF;

	// Simplex point order
	int key =
		(rx0 >= ry0 ? 0x1 : 0) |
		(rx0 >= rz0 ? 0x2 : 0) |
		(ry0 >= rz0 ? 0x4 : 0);
	assert(key != 0x2);
	assert(key != (0x1 | 0x4));
	const unsigned char (*simplexOrder)[3] = SimplexNoiseHelper<Float, 3>::simplexOrders[key];

	Float rx1, ry1, rz1;
	Float gx, gy, gz;
	Float c, c0;

	// Contibution 1
	c = static_cast<Float>(0.5) - rx0 * rx0 - ry0 * ry0 - rz0 * rz0;
	if (c <= 0) {
		c0 = 0;
	}
	else {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si, sj, sk), gx, gy, gz);
		c0 = c * c * c;
		c0 *= (gx * rx0 + gy * ry0 + gz * rz0);
	}

	// Contibution 2
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(1, simplexOrder[0][0], simplexOrder[0][1], simplexOrder[0][2], rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + simplexOrder[0][0], sj + simplexOrder[0][1], sk + simplexOrder[0][2]), gx, gy, gz);
		c = c * c * c;
		c *= (gx * rx1 + gy * ry1 + gz * rz1);

		c0 += c;
	}

	// Contibution 3
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(2, simplexOrder[1][0], simplexOrder[1][1], simplexOrder[1][2], rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + simplexOrder[1][0], sj + simplexOrder[1][1], sk + simplexOrder[1][2]), gx, gy, gz);
		c = c * c * c;
		c *= (gx * rx1 + gy * ry1 + gz * rz1);

		c0 += c;
	}

	// Contibution 4
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(3, 1, 1, 1, rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + 1, sj + 1, sk + 1), gx, gy, gz);
		c = c * c * c;
		c *= (gx * rx1 + gy * ry1 + gz * rz1);

		c0 += c;
	}

	// Done
	return c0 / SimplexNoiseHelper<Float, 3>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise3(
	Float x,
	Float y,
	Float z,
	Float* dx,
	Float* dy,
	Float* dz
) {
	assert(dx != nullptr);
	assert(dy != nullptr);
	assert(dz != nullptr);

	// fn(x,y,z) = sum((max(0, 0.5 - dot(xyz[i], xyz[i])) ^ 3) * dot(gradient[i], xyz[i]), i, 0, 3)

	// Skew point and find origin
	Float sx = x;
	Float sy = y;
	Float sz = z;
	SimplexNoiseHelper<Float, 3>::skewPoint(sx, sy, sz);
	HashInt si = SimplexNoise<Float>::fast_floor<HashInt>(sx);
	HashInt sj = SimplexNoise<Float>::fast_floor<HashInt>(sy);
	HashInt sk = SimplexNoise<Float>::fast_floor<HashInt>(sz);

	// The x,y,z relative distances from the deskewed origin
	Float rx0 = x;
	Float ry0 = y;
	Float rz0 = z;
	SimplexNoiseHelper<Float, 3>::deskewPoint((si + sj + sk), si, sj, sk, rx0, ry0, rz0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	si &= 0xFF;
	sj &= 0xFF;
	sk &= 0xFF;

	// Simplex point order
	int key =
		(rx0 >= ry0 ? 0x1 : 0) |
		(rx0 >= rz0 ? 0x2 : 0) |
		(ry0 >= rz0 ? 0x4 : 0);
	assert(key != 0x2);
	assert(key != (0x1 | 0x4));
	const unsigned char (*simplexOrder)[3] = SimplexNoiseHelper<Float, 3>::simplexOrders[key];

	Float dx2, dy2, dz2;
	Float rx1, ry1, rz1;
	Float gx, gy, gz;
	Float c, c0, c2, dot;

	// Contibution 1
	c0 = static_cast<Float>(0.5) - rx0 * rx0 - ry0 * ry0 - rz0 * rz0;
	if (c0 <= 0) {
		c0 = 0;
		*dx = 0;
		*dy = 0;
		*dz = 0;
		dx2 = 0;
		dy2 = 0;
		dz2 = 0;
	}
	else {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si, sj, sk), gx, gy, gz);

		dot = (gx * rx0 + gy * ry0 + gz * rz0);
		c2 = c0 * c0;
		c0 *= c2;

		*dx = rx0 * dot * c2;
		*dy = ry0 * dot * c2;
		*dz = rz0 * dot * c2;

		dx2 = c0 * gx;
		dy2 = c0 * gy;
		dz2 = c0 * gz;

		c0 *= dot;
	}

	// Contibution 2
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(1, simplexOrder[0][0], simplexOrder[0][1], simplexOrder[0][2], rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + simplexOrder[0][0], sj + simplexOrder[0][1], sk + simplexOrder[0][2]), gx, gy, gz);

		dot = (gx * rx1 + gy * ry1 + gz * rz1);
		c2 = c * c;
		c *= c2;

		*dx += rx1 * dot * c2;
		*dy += ry1 * dot * c2;
		*dz += rz1 * dot * c2;

		dx2 += c * gx;
		dy2 += c * gy;
		dz2 += c * gz;

		c *= dot;

		c0 += c;
	}

	// Contibution 3
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(2, simplexOrder[1][0], simplexOrder[1][1], simplexOrder[1][2], rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + simplexOrder[1][0], sj + simplexOrder[1][1], sk + simplexOrder[1][2]), gx, gy, gz);

		dot = (gx * rx1 + gy * ry1 + gz * rz1);
		c2 = c * c;
		c *= c2;

		*dx += rx1 * dot * c2;
		*dy += ry1 * dot * c2;
		*dz += rz1 * dot * c2;

		dx2 += c * gx;
		dy2 += c * gy;
		dz2 += c * gz;

		c *= dot;

		c0 += c;
	}

	// Contibution 4
	rx1 = rx0;
	ry1 = ry0;
	rz1 = rz0;
	SimplexNoiseHelper<Float, 3>::deskewPoint(3, 1, 1, 1, rx1, ry1, rz1);
	c = static_cast<Float>(0.5) - rx1 * rx1 - ry1 * ry1 - rz1 * rz1;
	if (c > 0) {
		SimplexNoiseHelper<Float, 3>::gradient(SimplexNoiseHelper<Float, 3>::gradientHash(this->perm, si + 1, sj + 1, sk + 1), gx, gy, gz);

		dot = (gx * rx1 + gy * ry1 + gz * rz1);
		c2 = c * c;
		c *= c2;

		*dx += rx1 * dot * c2;
		*dy += ry1 * dot * c2;
		*dz += rz1 * dot * c2;

		dx2 += c * gx;
		dy2 += c * gy;
		dz2 += c * gz;

		c *= dot;

		c0 += c;
	}

	// Complete derivative
	*dx *= -6;
	*dy *= -6;
	*dz *= -6;
	*dx += dx2;
	*dy += dy2;
	*dz += dz2;
	*dx /= SimplexNoiseHelper<Float, 3>::unnormalizedMaximum();
	*dy /= SimplexNoiseHelper<Float, 3>::unnormalizedMaximum();
	*dz /= SimplexNoiseHelper<Float, 3>::unnormalizedMaximum();

	// Done
	return c0 / SimplexNoiseHelper<Float, 3>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise3_octaves(
	OctavesInt octaves,
	Float x,
	Float y,
	Float z
) {
	if (octaves <= 0) return 0;

	Float n = noise3(x, y, z);
	Float scale = 1;
	Float total = 1;

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise3(x / scale, y / scale, z / scale) * scale;
	}

	return n / total;
}

template <typename Float>
Float
SimplexNoise<Float> :: noise3_octaves(
	OctavesInt octaves,
	Float x,
	Float y,
	Float z,
	Float* dx,
	Float* dy,
	Float* dz
) {
	assert(dx != nullptr);
	assert(dy != nullptr);
	assert(dz != nullptr);

	if (octaves <= 0) {
		*dx = 0;
		*dy = 0;
		*dz = 0;
		return 0;
	}

	Float n = noise3(x, y, z, dx, dy, dz);
	Float scale = 1;
	Float total = 1;
	Float d2[3];

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise3(x / scale, y / scale, z / scale, &d2[0], &d2[1], &d2[2]) * scale;
		*dx += d2[0] * scale;
		*dy += d2[1] * scale;
		*dz += d2[2] * scale;
	}

	*dx /= total;
	*dy /= total;
	*dz /= total;
	return n / total;
}



// 4d noise
template <typename Float>
Float
SimplexNoise<Float> :: noise4(
	Float x,
	Float y,
	Float z,
	Float w
) {
	// fn(x,y,z,w) = sum((max(0, 0.5 - dot(xyzw[i], xyzw[i])) ^ 3) * dot(gradient[i], xyzw[i]), i, 0, 4)

	// Helper struct for sorting
	struct Point {
		unsigned int index;
		Point* next;
	};

	// Vars
	Float point0[4];
	Float point1[4];
	Float gradient[4];
	HashInt pointFloor[4];
	HashInt simplexOrder[4];
	Point pointOrder[4];
	Float c, c0;
	Float currentValue;
	Point* first;
	Point* current;
	Point** pos;
	unsigned int i, j;

	// Skew point and find origin
	point0[0] = x;
	point0[1] = y;
	point0[2] = z;
	point0[3] = w;
	SimplexNoiseHelper<Float, 4>::skewPoint(point0);
	pointFloor[0] = SimplexNoise<Float>::fast_floor<HashInt>(point0[0]);
	pointFloor[1] = SimplexNoise<Float>::fast_floor<HashInt>(point0[1]);
	pointFloor[2] = SimplexNoise<Float>::fast_floor<HashInt>(point0[2]);
	pointFloor[3] = SimplexNoise<Float>::fast_floor<HashInt>(point0[3]);

	// The x,y,z,w relative distances from the deskewed origin
	point0[0] = x;
	point0[1] = y;
	point0[2] = z;
	point0[3] = w;
	SimplexNoiseHelper<Float, 4>::deskewPoint((pointFloor[0] + pointFloor[1] + pointFloor[2] + pointFloor[3]), pointFloor, point0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	pointFloor[0] &= 0xFF;
	pointFloor[1] &= 0xFF;
	pointFloor[2] &= 0xFF;
	pointFloor[3] &= 0xFF;

	// Find simplex order using insertion sort
	first = &(pointOrder[0]);
	pointOrder[0].index = 0;
	for (i = 1; i < 4; ++i) {
		current = &(pointOrder[i]);
		current->index = i;
		currentValue = point0[i];
		pos = &first;
		for (j = 0; ; ) {
			if (currentValue <= point0[(*pos)->index]) {
				// insert at this position
				current->next = *pos;
				*pos = current;
				break;
			}
			if (++j >= i) {
				// currentValue is the new minimum
				(*pos)->next = current;
				break;
			}
			pos = &(*pos)->next;
		}
	}

	// First case; this is a simplified version of the (i=0) case of the loop below
	c = static_cast<Float>(0.5) - SimplexNoiseHelper<Float, 4>::dot(point0, point0);
	if (c <= 0) {
		c0 = 0;
	}
	else {
		SimplexNoiseHelper<Float, 4>::gradient(SimplexNoiseHelper<Float, 4>::gradientHash(this->perm, pointFloor), gradient);
		c0 = c * c * c;
		c0 *= SimplexNoiseHelper<Float, 4>::dot(gradient, point0);
	}

	// Loop over remaining points
	for (i = 1; i <= 4; ++i) {
		// Simplex orders
		current = first;
		for (j = 4; ; ) {
			simplexOrder[current->index] = (j <= i);
			if (--j == 0) break;
			current = current->next;
		}

		// Relative distances for the remaining simplex points
		point1[0] = point0[0];
		point1[1] = point0[1];
		point1[2] = point0[2];
		point1[3] = point0[3];
		SimplexNoiseHelper<Float, 4>::deskewPoint(i, simplexOrder, point1);

		c = static_cast<Float>(0.5) - SimplexNoiseHelper<Float, 4>::dot(point1, point1);
		if (c > 0) {
			SimplexNoiseHelper<Float, 4>::gradient(SimplexNoiseHelper<Float, 4>::gradientHash(this->perm, pointFloor, simplexOrder), gradient);
			c = c * c * c;
			c *= SimplexNoiseHelper<Float, 4>::dot(gradient, point1);

			// Add to sum
			c0 += c;
		}
	}

	// Done
	return c0 / SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise4(
	Float x,
	Float y,
	Float z,
	Float w,
	Float* dx,
	Float* dy,
	Float* dz,
	Float* dw
) {
	assert(dx != nullptr);
	assert(dy != nullptr);
	assert(dz != nullptr);
	assert(dw != nullptr);

	// fn(x,y,z,w) = sum((max(0, 0.5 - dot(xyzw[i], xyzw[i])) ^ 3) * dot(gradient[i], xyzw[i]), i, 0, 4)

	// Helper struct for sorting
	struct Point {
		unsigned int index;
		Point* next;
	};

	// Vars
	Float point0[4];
	Float point1[4];
	Float gradient[4];
	HashInt pointFloor[4];
	HashInt simplexOrder[4];
	Point pointOrder[4];
	Float derivative2[4];
	Float c, c0, c2, dot;
	Float currentValue;
	Point* first;
	Point* current;
	Point** pos;
	unsigned int i, j;

	// Skew point and find origin
	point0[0] = x;
	point0[1] = y;
	point0[2] = z;
	point0[3] = w;
	SimplexNoiseHelper<Float, 4>::skewPoint(point0);
	pointFloor[0] = SimplexNoise<Float>::fast_floor<HashInt>(point0[0]);
	pointFloor[1] = SimplexNoise<Float>::fast_floor<HashInt>(point0[1]);
	pointFloor[2] = SimplexNoise<Float>::fast_floor<HashInt>(point0[2]);
	pointFloor[3] = SimplexNoise<Float>::fast_floor<HashInt>(point0[3]);

	// The x,y,z,w relative distances from the deskewed origin
	point0[0] = x;
	point0[1] = y;
	point0[2] = z;
	point0[3] = w;
	SimplexNoiseHelper<Float, 4>::deskewPoint((pointFloor[0] + pointFloor[1] + pointFloor[2] + pointFloor[3]), pointFloor, point0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	pointFloor[0] &= 0xFF;
	pointFloor[1] &= 0xFF;
	pointFloor[2] &= 0xFF;
	pointFloor[3] &= 0xFF;

	// Find simplex order using insertion sort
	first = &(pointOrder[0]);
	pointOrder[0].index = 0;
	for (i = 1; i < 4; ++i) {
		current = &(pointOrder[i]);
		current->index = i;
		currentValue = point0[i];
		pos = &first;
		for (j = 0; ; ) {
			if (currentValue <= point0[(*pos)->index]) {
				// insert at this position
				current->next = *pos;
				*pos = current;
				break;
			}
			if (++j >= i) {
				// currentValue is the new minimum
				(*pos)->next = current;
				break;
			}
			pos = &(*pos)->next;
		}
	}

	// First case; this is a simplified version of the (i=0) case of the loop below
	c0 = static_cast<Float>(0.5) - SimplexNoiseHelper<Float, 4>::dot(point0, point0);
	if (c0 <= 0) {
		c0 = 0;
		*dx = 0;
		*dy = 0;
		*dz = 0;
		*dw = 0;
		derivative2[0] = 0;
		derivative2[1] = 0;
		derivative2[2] = 0;
		derivative2[3] = 0;
	}
	else {
		SimplexNoiseHelper<Float, 4>::gradient(SimplexNoiseHelper<Float, 4>::gradientHash(this->perm, pointFloor), gradient);

		dot = SimplexNoiseHelper<Float, 4>::dot(gradient, point0);
		c2 = c0 * c0;
		c0 *= c2;

		*dx = point0[0] * dot * c2;
		*dy = point0[1] * dot * c2;
		*dz = point0[2] * dot * c2;
		*dw = point0[3] * dot * c2;

		derivative2[0] = c0 * gradient[0];
		derivative2[1] = c0 * gradient[1];
		derivative2[2] = c0 * gradient[2];
		derivative2[3] = c0 * gradient[3];

		c0 *= dot;
	}

	// Loop over remaining points
	for (i = 1; i <= 4; ++i) {
		// Simplex orders
		current = first;
		for (j = 4; ; ) {
			simplexOrder[current->index] = (j <= i);
			if (--j == 0) break;
			current = current->next;
		}

		// Relative distances for the remaining simplex points
		point1[0] = point0[0];
		point1[1] = point0[1];
		point1[2] = point0[2];
		point1[3] = point0[3];
		SimplexNoiseHelper<Float, 4>::deskewPoint(i, simplexOrder, point1);

		c = static_cast<Float>(0.5) - SimplexNoiseHelper<Float, 4>::dot(point1, point1);
		if (c > 0) {
			SimplexNoiseHelper<Float, 4>::gradient(SimplexNoiseHelper<Float, 4>::gradientHash(this->perm, pointFloor, simplexOrder), gradient);

			dot = SimplexNoiseHelper<Float, 4>::dot(gradient, point1);
			c2 = c * c;
			c *= c2;

			*dx += point1[0] * dot * c2;
			*dy += point1[1] * dot * c2;
			*dz += point1[2] * dot * c2;
			*dw += point1[3] * dot * c2;

			derivative2[0] += c * gradient[0];
			derivative2[1] += c * gradient[1];
			derivative2[2] += c * gradient[2];
			derivative2[3] += c * gradient[3];

			c *= dot;

			c0 += c;
		}
	}

	// Complete derivative
	*dx *= -6;
	*dy *= -6;
	*dz *= -6;
	*dw *= -6;
	*dx += derivative2[0];
	*dy += derivative2[1];
	*dz += derivative2[2];
	*dw += derivative2[3];
	*dx /= SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();
	*dy /= SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();
	*dz /= SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();
	*dw /= SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();

	// Done
	return c0 / SimplexNoiseHelper<Float, 4>::unnormalizedMaximum();
}

template <typename Float>
Float
SimplexNoise<Float> :: noise4_octaves(
	OctavesInt octaves,
	Float x,
	Float y,
	Float z,
	Float w
) {
	if (octaves <= 0) return 0;

	Float n = noise4(x, y, z, w);
	Float scale = 1;
	Float total = 1;

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise4(x / scale, y / scale, z / scale, w / scale) * scale;
	}

	return n / total;
}

template <typename Float>
Float
SimplexNoise<Float> :: noise4_octaves(
	OctavesInt octaves,
	Float x,
	Float y,
	Float z,
	Float w,
	Float* dx,
	Float* dy,
	Float* dz,
	Float* dw
) {
	assert(dx != nullptr);
	assert(dy != nullptr);
	assert(dz != nullptr);

	if (octaves <= 0) {
		*dx = 0;
		*dy = 0;
		*dz = 0;
		*dw = 0;
		return 0;
	}

	Float n = noise4(x, y, z, w, dx, dy, dz, dw);
	Float scale = 1;
	Float total = 1;
	Float d2[4];

	while (--octaves != 0) {
		scale /= 2;
		total += scale;
		n += noise4(x / scale, y / scale, z / scale, w / scale, &d2[0], &d2[1], &d2[2], &d2[3]) * scale;
		*dx += d2[0] * scale;
		*dy += d2[1] * scale;
		*dz += d2[2] * scale;
		*dz += d2[3] * scale;
	}

	*dx /= total;
	*dy /= total;
	*dz /= total;
	*dw /= total;
	return n / total;
}


// n-d noise
template <typename Float>
Float
SimplexNoise<Float> :: noise_n(
	Float maxRadius,
	Float power,
	int dimensions,
	const Float* position
) {
	assert(power > 0);
	assert(dimensions > 1);
	assert(position != nullptr);

	// Helper struct for sorting
	struct Point {
		unsigned int index;
		Point* next;
	};

	// Vars
	Float* point0 = new Float[dimensions];
	Float* point1 = new Float[dimensions];
	Float* gradient = new Float[dimensions];
	HashInt* pointFloor = new HashInt[dimensions];
	HashInt* simplexOrder = new HashInt[dimensions];
	Point* pointOrder = new Point[dimensions];
	Float c0, c1;
	Float currentValue;
	Point* first;
	Point* current;
	Point** pos;
	HashInt pointFloorSum;
	unsigned int i, j;
	int k;
	Float skewFactor = SimplexNoiseGenericHelper<Float>::skewFactor(dimensions);
	Float deskewFactor = SimplexNoiseGenericHelper<Float>::deskewFactor(dimensions);

	// Skew point and find origin
	for (k = 0; k < dimensions; ++k) point0[k] = position[k];
	SimplexNoiseGenericHelper<Float>::skewPoint(dimensions, skewFactor, point0);
	for (k = 0; k < dimensions; ++k) pointFloor[k] = SimplexNoise<Float>::fast_floor<HashInt>(point0[k]);

	// The x,y,z,w relative distances from the deskewed origin
	for (k = 0; k < dimensions; ++k) point0[k] = position[k];
	pointFloorSum = pointFloor[0];
	for (k = 1; k < dimensions; ++k) pointFloorSum += pointFloor[k];
	SimplexNoiseGenericHelper<Float>::deskewPoint(dimensions, deskewFactor, pointFloorSum, pointFloor, point0);

	// Truncate the integer indices to [0,255]; this makes gradient hashing use less computations
	for (k = 0; k < dimensions; ++k) pointFloor[k] &= 0xFF;

	// Find simplex order using insertion sort
	first = &(pointOrder[0]);
	pointOrder[0].index = 0;
	for (i = 1; i < dimensions; ++i) {
		current = &(pointOrder[i]);
		current->index = i;
		currentValue = point0[i];
		pos = &first;
		for (j = 0; ; ) {
			if (currentValue <= point0[(*pos)->index]) {
				// insert at this position
				current->next = *pos;
				*pos = current;
				break;
			}
			if (++j >= i) {
				// currentValue is the new minimum
				(*pos)->next = current;
				break;
			}
			pos = &(*pos)->next;
		}
	}

	// First case; this is a simplified version of the (i=0) case of the loop below
	c0 = static_cast<Float>(maxRadius) - SimplexNoiseGenericHelper<Float>::dot(dimensions, point0, point0);
	if (c0 < 0) {
		c0 = 0;
	}
	else {
		SimplexNoiseGenericHelper<Float>::gradient(dimensions, this->perm, pointFloor, nullptr, gradient);
		c0 = ::pow(c0, power);
		c0 *= SimplexNoiseGenericHelper<Float>::dot(dimensions, gradient, point0);
	}

	// Loop over remaining points
	for (i = 1; i <= dimensions; ++i) {
		// Simplex orders
		current = first;
		for (j = dimensions; ; ) {
			simplexOrder[current->index] = (j <= i);
			if (--j == 0) break;
			current = current->next;
		}

		// Relative distances for the remaining simplex points
		for (k = 0; k < dimensions; ++k) point1[k] = point0[k];
		SimplexNoiseGenericHelper<Float>::deskewPoint(dimensions, deskewFactor, i, simplexOrder, point1);

		c1 = static_cast<Float>(maxRadius) - SimplexNoiseGenericHelper<Float>::dot(dimensions, point1, point1);
		if (c1 > 0) {
			SimplexNoiseGenericHelper<Float>::gradient(dimensions, this->perm, pointFloor, simplexOrder, gradient);
			c1 = ::pow(c1, power);
			c1 *= SimplexNoiseGenericHelper<Float>::dot(dimensions, gradient, point1);

			// Add to sum
			c0 += c1;
		}
	}

	// Clean
	delete [] point0;
	delete [] point1;
	delete [] gradient;
	delete [] pointFloor;
	delete [] simplexOrder;
	delete [] pointOrder;

	// Done
	return c0;
}


