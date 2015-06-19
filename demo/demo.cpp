#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "SimplexNoise.hpp"

using namespace std;



inline int minInt(int v1, int v2) {
	return (v1 < v2) ? v1 : v2;
}



int main(int argc, char** argv) {
	// Usage
	if (argc <= 11) {
		cerr << "Invalid arguments" << endl;
		cerr << "  demo filename mode norm_mode width height scale max_radius power dimensions octaves [x y z w ...]" << endl;
		// mode=binary|ascii
		// norm_mode=centered|full
		return -1;
	}


	// Arguments
	int argid = 1;
	const char* filename = argv[argid++];
	bool outputBinary = (stricmp(argv[argid++], "ascii") != 0);
	bool normalizeCentered = (stricmp(argv[argid++], "centered") == 0);
	int width = atoi(argv[argid++]);
	int height = atoi(argv[argid++]);
	double scale = atof(argv[argid++]);
	double maxRadius = atof(argv[argid++]);
	double power = atof(argv[argid++]);
	int dimensions = atoi(argv[argid++]);
	int octaves = atoi(argv[argid++]);

	if (width < 1) width = 1;
	if (height < 1) height = 1;
	if (dimensions < 2) dimensions = 2;
	if (octaves < 1) octaves = 1;


	// Setup
	ostream* infoStream = &cerr;
	ostream* f = nullptr;
	fstream* ff = nullptr;
	if (strcmp(filename, "-") == 0) {
		f = &cout;
		infoStream = &cerr;
#ifdef _WIN32
		// put into binary mode on windows
		_setmode(_fileno(stdout), _O_BINARY);
#endif
	}
	else {
		ff = new fstream(filename, fstream::out | fstream::binary);
		if (!ff->is_open()) {
			cerr << "Failed to open output file" << endl;
			delete ff;
			return -2;
		}
		f = ff;
	}

	double* values = new double[width * height];
	double* v = values;

	bool first = true;
	double min = 0.0, max = 0.0;

	double* p = new double[dimensions];
	double* pDefault = new double[dimensions];
	for (int i = 0; i < dimensions; ++i) {
		p[i] = 0.0;
	}
	for (int i = 0, j = minInt(argc - argid + i, dimensions); i < j; ++i) {
		p[i] = atof(argv[argid++]);
	}
	for (int i = 0; i < dimensions; ++i) {
		pDefault[i] = p[i];
	}

	SimplexNoise<double> d;


	// Status
	infoStream->precision(16);
	*infoStream << "Generating noise:" << endl;
	*infoStream << "    filename: " << filename << (ff == nullptr ? " (stdout)" : "") << endl;
	*infoStream << "        mode: " << (outputBinary ? "binary" : "ascii") << endl;
	*infoStream << "   norm_mode: " << (normalizeCentered ? "centered" : "full") << endl;
	*infoStream << "       width: " << width << endl;
	*infoStream << "      height: " << height << endl;
	*infoStream << "       scale: " << scale << endl;
	*infoStream << "  max_radius: " << maxRadius << endl;
	*infoStream << "       power: " << power << endl;
	*infoStream << "  dimensions: " << dimensions << endl;
	*infoStream << "     octaves: " << octaves << endl << endl;


	// Generate noise
	double n, nScale;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			p[0] = pDefault[0] + x * scale;
			p[1] = pDefault[1] + y * scale;
			n = d.noise_n(maxRadius, power, dimensions, p);
			if (octaves > 1) {
				nScale = 0.5;
				for (int i = 1; i < octaves; ++i) {
					for (int j = 0; j < dimensions; ++j) p[j] *= 2.0;
					n += d.noise_n(maxRadius, power, dimensions, p) * nScale;
					nScale /= 2.0;
				}
				for (int j = 0; j < dimensions; ++j) {
					p[j] = pDefault[j];
				}
			}

			if (first) {
				min = max = n;
				first = false;
			}
			else {
				if (n < min) min = n;
				if (n > max) max = n;
			}

			*v = n;
			++v;
		}
	}


	// Output min/max
	*infoStream << "Mininum value: " << min << endl;
	*infoStream << "Maximum value: " << max << endl;

	double range = max - min;
	if (normalizeCentered) {
		range = (-min > max) ? -min : max;
		min = -range;
		range *= 2.0;
	}


	// Write file
	*f << (outputBinary ? "P5" : "P2") << "\n";
	*f << width << " " << height << "\n";
	*f << "255" << "\n";

	v = values;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			double n = (*v - min);
			if (range == 0) n = 0.5;
			else n /= range;
			int px = static_cast<int>(n * 255 + 0.5);

			if (outputBinary) {
				*f << static_cast<char>(px);
			}
			else {
				*f << px << "\n";
			}

			++v;
		}
	}


	// Cleanup
	if (ff != nullptr) {
		ff->close();
		delete ff;
	}
	delete [] p;
	delete [] values;


	// Done
	return 0;
}


