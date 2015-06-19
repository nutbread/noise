#include <iostream>
#include <fstream>
#include <cstdlib>

#include "SimplexNoise.hpp"

using namespace std;



int main(int argc, char** argv) {
	// Usage
	if (argc <= 7) {
		cout << "Usage:" << endl;
		cout << "  " << argv[0] << " filename width height scale dimensions octaves derivative" << endl;
		return -1;
	}


	// Arguments
	const char* filename = argv[1];
	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	double scale = atof(argv[4]);
	int dimensions = atoi(argv[5]);
	int octaves = atoi(argv[6]);
	int derivative = atoi(argv[7]);

	if (dimensions < 1) dimensions = 1;
	else if (dimensions > 4) dimensions = 4;
	if (width < 1) width = 1;
	if (height < 1) height = 1;
	if (derivative < 0) derivative = 0;
	else if (derivative > dimensions) derivative = dimensions;
	if (octaves < 0) octaves = 0;

	fstream out(filename, fstream::out | fstream::binary);
	if (!out.is_open()) {
		cerr << "Failed to open output file" << endl;
		return -2;
	}

	double* values = new double[width * height];
	double* v = values;
	double noise[5];


	// Create noise
	bool first = true;
	double n;
	double min = 0.0, max = 0.0;
	SimplexNoise<double> generator;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			switch (dimensions) {
				case 1:
					if (octaves == 0) {
						noise[0] = generator.noise1(x * scale, &noise[1]);
					}
					else {
						noise[0] = generator.noise1_octaves(octaves, x * scale, &noise[1]);
					}
				break;
				case 2:
					if (octaves == 0) {
						noise[0] = generator.noise2(x * scale, y * scale, &noise[1], &noise[2]);
					}
					else {
						noise[0] = generator.noise2_octaves(octaves, x * scale, y * scale, &noise[1], &noise[2]);
					}
				break;
				case 3:
					if (octaves == 0) {
						noise[0] = generator.noise3(x * scale, y * scale, 0.0, &noise[1], &noise[2], &noise[3]);
					}
					else {
						noise[0] = generator.noise3_octaves(octaves, x * scale, y * scale, 0.0, &noise[1], &noise[2], &noise[3]);
					}
				break;
				case 4:
					if (octaves == 0) {
						noise[0] = generator.noise4(x * scale, y * scale, 0.0, 0.0, &noise[1], &noise[2], &noise[3], &noise[4]);
					}
					else {
						noise[0] = generator.noise4_octaves(octaves, x * scale, y * scale, 0.0, 0.0, &noise[1], &noise[2], &noise[3], &noise[4]);
					}
				break;
			}

			n = noise[derivative];
			*(v++) = n;

			if (first) {
				min = n;
				max = n;
				first = false;
			}
			else {
				if (n < min) min = n;
				if (n > max) max = n;
			}
		}
	}


	// Range output
	cout.precision(16);
	cout << "Min = " << min << endl;
	cout << "Max = " << max << endl;
	if (-min > max) max = -min;
	max *= 2.0;


	// Header
	out << "P5" << "\n";
	out << width << " " << height << "\n";
	out << "255" << "\n";

	// Body
	v = values;
	int px;
	for (int i = 0, j = width * height; i < j; ++i) {
		n = 0.5 + *(v++) / max;
		px = static_cast<int>(n * 255 + 0.5);
		out << static_cast<char>(px);
	}


	// Done
	return 0;
}


