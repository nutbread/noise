#include <iostream>
#include <cassert>
#include <cmath>
#include "Matrix.hpp"

using namespace std;



constexpr double
getDeskewFactor(
	uint64_t dimensions
) {
	return (dimensions + 1 - sqrt(dimensions + 1)) / (dimensions * (dimensions + 1));
}

void
deskewPoint(
	uint64_t dimensions,
	double* point
) {
	double sum = 0.0;

	for (uint64_t i = 0; i < dimensions; ++i) {
		sum += point[i];
	}

	sum *= getDeskewFactor(dimensions);

	for (uint64_t i = 0; i < dimensions; ++i) {
		point[i] -= sum;
	}
}

double*
barycentricToCartesian(
	uint64_t dimensions,
	const double* T,
	const double* point
) {
	double* p = new double[dimensions];

	for (uint64_t i = 0; i < dimensions; ++i) {
		p[i] = 0.0;
		for (uint64_t j = 0; j < dimensions; ++j) {
			p[i] += point[j] * T[i + j * dimensions];
		}
	}

	return p;
}

void
barycentricToCartesian(
	uint64_t dimensions,
	const double* T,
	const double* point,
	double* dest
) {
	for (uint64_t i = 0; i < dimensions; ++i) {
		dest[i] = 0.0;
		for (uint64_t j = 0; j < dimensions; ++j) {
			dest[i] += point[j] * T[i + j * dimensions];
		}
	}
}

double*
generateSimplexPointArray(
	uint64_t dimensions
) {
	double* pointArray = new double[dimensions * (dimensions + 1)];
	double* point;

	for (uint64_t i = 0; i < dimensions; ++i) {
		point = &pointArray[dimensions * i];
		for (uint64_t j = 0; j < dimensions; ++j) {
			point[j] = (j <= i) ? 1.0 : 0.0;
		}
		deskewPoint(dimensions, point);
	}

	point = &pointArray[dimensions * dimensions];
	for (uint64_t j = 0; j < dimensions; ++j) {
		point[j] = 0.0;
	}
	deskewPoint(dimensions, point); // not necessary

	return pointArray;
}



double*
generateTMatrix(
	uint64_t dimensions
) {
	double* tMatrix = new double[dimensions * dimensions];

	for (uint64_t i = 0; i < dimensions; ++i) {
		double* point = &tMatrix[dimensions * i];
		for (uint64_t j = 0; j < dimensions; ++j) {
			point[j] = (j <= i) ? 1.0 : 0.0;
		}
		deskewPoint(dimensions, point);
	}

	return tMatrix;
}

double*
matrixInverse(
	uint64_t dimensions,
	const double* matrix
) {
	// Naive implementation, since it's not really time critical
	assert(dimensions > 0);

	// Matrix of minors
	double* m = new double[dimensions * dimensions];

	if (dimensions == 1) {
		assert(*matrix != 0.0);
		*m = 1.0 / *matrix;
		return m;
	}

	for (uint64_t y = 0; y < dimensions; ++y) {
		for (uint64_t x = 0; x < dimensions; ++x) {
			double* minor = matrixMinor(dimensions, x, y, matrix);
			m[x + y * dimensions] = matrixDeterminant(dimensions - 1, minor);
			delete [] minor;
		}
	}

	// Cofactors
	for (uint64_t y = 0; y < dimensions; ++y) {
		for (uint64_t x = 0; x < dimensions; ++x) {
			uint64_t i = x + y * dimensions;
			if (((x + y) % 2) == 1) m[i] = -m[i];
		}
	}

	// Adjugate
	for (uint64_t y = 0; y < dimensions; ++y) {
		for (uint64_t x = y + 1; x < dimensions; ++x) {
			uint64_t p1 = x + y * dimensions;
			uint64_t p2 = y + x * dimensions;
			double v1 = m[p1];
			m[p1] = m[p2];
			m[p2] = v1;
		}
	}

	// Determinant
	double d = matrixDeterminant(dimensions, m);
	assert(d != 0.0);

	for (uint64_t i = 0, j = dimensions * dimensions; i < j; ++i) {
		m[i] /= d;
	}

	// Done
	return m;
}

double*
matrixMinor(
	uint64_t dimensions,
	uint64_t x,
	uint64_t y,
	const double* matrix
) {
	assert(dimensions > 1);
	assert(x >= 0);
	assert(x < dimensions);
	assert(y >= 0);
	assert(y < dimensions);

	uint64_t d = dimensions - 1;
	double* m = new double[d * d];

	for (uint64_t y1 = 0, y2 = 0; y1 < dimensions; ++y1) {
		if (y1 == y) continue;
		for (uint64_t x1 = 0, x2 = 0; x1 < dimensions; ++x1) {
			if (x1 == x) continue;

			m[x2 + y2 * d] = matrix[x1 + y1 * dimensions];

			++x2;
		}
		++y2;
	}

	return m;
}

double
matrixDeterminant(
	uint64_t dimensions,
	const double* matrix
) {
	assert(dimensions > 0);

	// Base case
	if (dimensions == 1) return *matrix;

	double determinant = 0.0;

	for (uint64_t i = 0; i < dimensions; ++i) {
		double* minor = matrixMinor(dimensions, i, 0, matrix);
		double d = matrixDeterminant(dimensions - 1, minor) * matrix[i];
		delete [] minor;

		if ((i % 2) == 0) {
			determinant += d;
		}
		else {
			determinant -= d;
		}
	}

	return determinant;
}

void
printMatrix(
	uint64_t dimensions,
	const double* matrix
) {
	cout << "Matrix" << endl;

	for (uint64_t y = 0; y < dimensions; ++y) {
		cout << "  ";
		for (uint64_t x = 0; x < dimensions; ++x) {
			if (x > 0) cout << ",";
			cout << matrix[x + y * dimensions];
		}
		cout << endl;
	}

	cout << "  det = " << matrixDeterminant(dimensions, matrix) << endl;

}


