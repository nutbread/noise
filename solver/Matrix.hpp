#ifndef ___H_MATRIX
#define ___H_MATRIX

#include <cstdint>



// Points and skewing
constexpr double
getDeskewFactor(
	uint64_t dimensions
);

void
deskewPoint(
	uint64_t dimensions,
	double* point
);

double*
barycentricToCartesian(
	uint64_t dimensions,
	const double* T,
	const double* point
);

void
barycentricToCartesian(
	uint64_t dimensions,
	const double* T,
	const double* point,
	double* dest
);

double*
generateSimplexPointArray(
	uint64_t dimensions
);



// Matrix functions
double*
generateTMatrix(
	uint64_t dimensions
);

double*
matrixInverse(
	uint64_t dimensions,
	const double* matrix
);

double*
matrixMinor(
	uint64_t dimensions,
	uint64_t x,
	uint64_t y,
	const double* matrix
);

double
matrixDeterminant(
	uint64_t dimensions,
	const double* matrix
);

void
printMatrix(
	uint64_t dimensions,
	const double* matrix
);



#endif // ___H_MATRIX


