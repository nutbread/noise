#include <iostream>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <cstdlib>
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include "hyperloop.hpp"
#include "Matrix.hpp"
#include "solver.hpp"

using namespace std;



// Threading
int
getThreadCount() {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	int cpuCount = sysinfo.dwNumberOfProcessors;
	if (cpuCount < 1) cpuCount = 1;

	return cpuCount;
}

bool
spawnThread(
	LPTHREAD_START_ROUTINE function,
	ThreadData* threadInfo
) {
	assert(threadInfo != nullptr);

	// Attr
	SECURITY_ATTRIBUTES threadAttr;
	threadAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	threadAttr.lpSecurityDescriptor = nullptr;
	threadAttr.bInheritHandle = FALSE;

	// Create thread
	threadInfo->threadHandle = CreateThread(
		&threadAttr, // lpThreadAttributes,
		0, // dwStackSize,
		function, // lpStartAddress,
		static_cast<void*>(threadInfo), // lpParameter,
		0, // dwCreationFlags,
		nullptr // lpThreadId
	);

	// Return the handle
	return (threadInfo->threadHandle != nullptr);
}

void
joinThreads(
	int threadCount,
	ThreadData* threadInfos
) {
	if (threadCount == 0) return;

	HANDLE* handles = new HANDLE[threadCount];
	for (int i = 0; i < threadCount; ++i) {
		handles[i] = threadInfos[i].threadHandle;
	}

	WaitForMultipleObjects(threadCount, handles, TRUE, INFINITE);

	delete [] handles;
}

void
completeThread(
	ThreadData* threadInfo
) {
	assert(threadInfo != nullptr);

	if (threadInfo->threadHandle != nullptr) {
		CloseHandle(threadInfo->threadHandle);
		threadInfo->threadHandle = nullptr;
	}
}



// Class methods
GlobalData :: GlobalData() :
	lock(),
	threadCount(1),
	displayEvery(true),
	dimensions(3),
	points(1),
	pointsLarge(1),
	gradientCount(1),
	iterationCount(1),
	iterations(0),
	T(nullptr),
	Tinv(nullptr),
	simplexPoints(nullptr),
	updates(0),
	minimum(0.0),
	maximum(0.0)
{
	InitializeCriticalSection(&this->lock);
}

GlobalData :: ~GlobalData() {
	DeleteCriticalSection(&this->lock);
}

bool
GlobalData :: updateExtrema(
	const ThreadData* tData,
	Double minimum,
	Double maximum,
	bool updateCount
) {
	bool updated = false;

	EnterCriticalSection(&this->lock);

	// Update
	if (this->iterations == 0) {
		this->minimum = minimum;
		this->maximum = maximum;
		updated = true;
	}
	else {
		if (minimum < this->minimum) {
			this->minimum = minimum;
			updated = true;
		}
		if (maximum > this->maximum) {
			this->maximum = maximum;
			updated = true;
		}
	}
	if (updateCount) ++this->iterations;

	// Output
	if (updated || tData->globalData->displayEvery) {
		if (updateCount) {
			cout << "progress=" << this->iterations << "/" << this->iterationCount;
		}
		else {
			cout << "progress=retry";
		}
		cout << " (thread=" << (tData->threadIndex + 1) << "/" << this->threadCount << "; progress=" << (tData->iterations) << "/" << tData->iterationCount << ")" <<
			" min=" << this->minimum << "; max=" << this->maximum << "; largest=" << (-this->minimum > this->maximum ? -this->minimum : this->maximum);
		if (updated) {
			cout << " [updated]";
			++this->updates;
		}
		cout << endl;
	}

	stringstream ss;
	ss << "solver (" << this->iterations << "/" << this->iterationCount << "; " << this->updates << ")";
	string s = ss.str();
	SetConsoleTitle(s.c_str());

	LeaveCriticalSection(&this->lock);

	return updated;
}



ThreadData :: ThreadData() :
	threadHandle(nullptr),
	globalData(nullptr),
	threadIndex(0),
	pointsDouble(0.0),
	gradientCount(0),
	iterationStart(0),
	iterationCount(0),
	iterations(0),
	coordinates(nullptr),
	coordinatesTransformed(nullptr),
	T(nullptr),
	Tinv(nullptr),
	simplexPoints(nullptr),
	gradients(nullptr),
	minimum(0.0),
	maximum(0.0)
{
}
ThreadData :: ~ThreadData() {
	delete [] this->coordinates;
}



// Primary looping functions
DWORD WINAPI
threadFunction(
	void* data
) {
	ThreadData* tData = static_cast<ThreadData*>(data);
	GlobalData* gd = tData->globalData;

	// Vars
	tData->gradients = new double[(gd->dimensions + 1) * gd->dimensions];

	hyperloop::triangle_eq::iterate_range<uint64_t, ThreadData*, gradientLoop>(
		gd->dimensions + 1,
		tData->gradientCount,
		tData->iterationStart,
		tData->iterationCount,
		tData
	);

	delete [] tData->gradients;

	return 0;
}

void gradientLoop(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
) {
	// Vars
	GlobalData* gd = tData->globalData;

	// Set the gradient
	dimensions = gd->dimensions;
	double* g = tData->gradients;
	for (uint64_t i = 0; i <= dimensions; ++i) {
		uint64_t zeroPos = (indices[i] >> (dimensions - 1));
		for (uint64_t j = 0, k = 0; j < dimensions; ++j) {
			if (j == zeroPos) {
				*(g++) = 0.0;
			}
			else {
				*(g++) = ((indices[i] & (1 << (k++))) == 0 ? -1.0 : 1.0);
			}
		}
	}

	// Loop
	tData->minimum = 0.0;
	tData->maximum = 0.0;

	if (gd->dimensions == 2) {
		hyperloop::triangle_sum::iterate<uint64_t, ThreadData*, iterateFunctionPow4>(
			gd->dimensions,
			gd->points,
			tData
		);
	}
	else {
		hyperloop::triangle_sum::iterate<uint64_t, ThreadData*, iterateFunctionPow3>(
			gd->dimensions,
			gd->points,
			tData
		);
	}

	// Next
	++tData->iterations;
	bool updated = gd->updateExtrema(
		tData,
		tData->minimum,
		tData->maximum,
		true
	);

	if (updated) {
		// Set point count
		tData->pointsDouble = gd->pointsLarge;

		// If update was successful, re-run using a higher point density
		tData->minimum = 0.0;
		tData->maximum = 0.0;

		if (gd->dimensions == 2) {
			hyperloop::triangle_sum::iterate<uint64_t, ThreadData*, iterateFunctionPow4>(
				gd->dimensions,
				gd->pointsLarge,
				tData
			);
		}
		else {
			hyperloop::triangle_sum::iterate<uint64_t, ThreadData*, iterateFunctionPow3>(
				gd->dimensions,
				gd->pointsLarge,
				tData
			);
		}

		// Update again
		gd->updateExtrema(
			tData,
			tData->minimum,
			tData->maximum,
			false
		);

		// Reset point count
		tData->pointsDouble = gd->points;
	}
}

void
iterateFunctionPow3(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
) {
	uint64_t testSum = 0;
	for (uint64_t i = 0; i <= dimensions; ++i) {
		tData->coordinates[i] = indices[i] / tData->pointsDouble;
		testSum += indices[i];
	}
	assert(testSum == sum);

	barycentricToCartesian(
		dimensions,
		tData->T,
		tData->coordinates,
		tData->coordinatesTransformed
	);

	// Check that it's within range of all endpoints
	//uint64_t dimensionsPlus1 = dimensions + 1;
	double distSq;
	double d, dot;
	double value = 0.0;
	double* g = tData->gradients;
	for (uint64_t i = 0; i <= dimensions; ++i) {
		distSq = 0.0;
		dot = 0.0;
		for (uint64_t j = 0; j < dimensions; ++j) {
			d = (tData->coordinatesTransformed[j] - tData->simplexPoints[i * dimensions + j]);
			distSq += d * d;
			dot += d * (*(g++));
		}

		// Distance too large
		if (distSq >= 0.5) continue;

		distSq = 0.5 - distSq;
		distSq = distSq * distSq * distSq;
		distSq *= dot;

		value += distSq;
	}

	// Update min/max values
	if (value < tData->minimum) tData->minimum = value;
	else if (value > tData->maximum) tData->maximum = value;
}

void
iterateFunctionPow4(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
) {
	uint64_t testSum = 0;
	for (uint64_t i = 0; i <= dimensions; ++i) {
		tData->coordinates[i] = indices[i] / tData->pointsDouble;
		testSum += indices[i];
	}
	assert(testSum == sum);

	barycentricToCartesian(
		dimensions,
		tData->T,
		tData->coordinates,
		tData->coordinatesTransformed
	);

	// Check that it's within range of all endpoints
	//uint64_t dimensionsPlus1 = dimensions + 1;
	double distSq;
	double d, dot;
	double value = 0.0;
	double* g = tData->gradients;
	for (uint64_t i = 0; i <= dimensions; ++i) {
		distSq = 0.0;
		dot = 0.0;
		for (uint64_t j = 0; j < dimensions; ++j) {
			d = (tData->coordinatesTransformed[j] - tData->simplexPoints[i * dimensions + j]);
			distSq += d * d;
			dot += d * (*(g++));
		}

		// Distance too large
		if (distSq >= 0.5) continue;

		distSq = 0.5 - distSq;
		distSq *= distSq;
		distSq *= distSq;
		distSq *= dot;

		value += distSq;
	}

	// Update min/max values
	if (value < tData->minimum) tData->minimum = value;
	else if (value > tData->maximum) tData->maximum = value;
}



// Main
int
main(
	int argc,
	char** argv
) {
	cout.precision(16);

	if (argc <= 5) {
		cerr << "Usage:" << endl;
		cerr << "  " << argv[0] << " show_every threads dimensions sample_points sample_points_large" << endl;
		cerr << endl;
		cerr << "  show_every: true | false" << endl;
		cerr << "  threads: number of threads (0 for auto)" << endl;
		cerr << "  dimensions: number of dimensions (2 or greater)" << endl;
		cerr << "  sample_points: scale of points to sample" << endl;
		cerr << "  sample_points_large: scale of points to sample (high density)" << endl;

		return -1;
	}


	// Setup global data
	GlobalData gd;

	gd.displayEvery = (strcmp(argv[1], "true") == 0);

	gd.threadCount = atoi(argv[2]);
	if (gd.threadCount <= 0) gd.threadCount = getThreadCount();

	gd.dimensions = atoi(argv[3]);
	if (gd.dimensions < 2) gd.dimensions = 2;

	gd.points = atoi(argv[4]);
	if (gd.points < 1) gd.points = 1;

	gd.pointsLarge = atoi(argv[5]);
	if (gd.pointsLarge < 1) gd.pointsLarge = 1;


	gd.gradientCount = (1 << (gd.dimensions - 1)) * gd.dimensions;
	if (gd.dimensions == 2) {
		// If 2d, custom gradient gen
		gd.gradientCount += (1 << gd.dimensions);
	}
	gd.iterationCount = hyperloop::triangle_eq::iterations<uint64_t>(gd.dimensions + 1, gd.gradientCount);
	gd.iterations = 0;

	gd.T = generateTMatrix(gd.dimensions);
	gd.Tinv = matrixInverse(gd.dimensions, gd.T); // Not really needed
	gd.simplexPoints = generateSimplexPointArray(gd.dimensions);



	// Setup thread datas
	ThreadData* threadDatas = new ThreadData[gd.threadCount];
	for (int i = 0; i < gd.threadCount; ++i) {
		threadDatas[i].globalData = &gd;
		threadDatas[i].threadIndex = i;
		threadDatas[i].pointsDouble = gd.points;
		threadDatas[i].gradientCount = gd.gradientCount;
		threadDatas[i].iterationStart = (gd.iterationCount * i) / gd.threadCount;
		threadDatas[i].iterationCount = (gd.iterationCount * (i + 1)) / gd.threadCount - threadDatas[i].iterationStart;
		threadDatas[i].coordinates = new ThreadData::Double[(gd.dimensions + 1) * 2];
		threadDatas[i].coordinatesTransformed = &threadDatas[i].coordinates[gd.dimensions + 1];
		threadDatas[i].T = gd.T;
		threadDatas[i].Tinv = gd.Tinv;
		threadDatas[i].simplexPoints = gd.simplexPoints;
	}



	// Spawn threads
	int threadCount = gd.threadCount - 1;

	bool okay = true;
	for (int i = 0; i < threadCount; ++i) {
		okay = spawnThread(threadFunction, &threadDatas[i]) && okay;
	}

	// Last thread
	threadFunction(static_cast<void*>(&threadDatas[threadCount]));

	// Join
	joinThreads(threadCount, threadDatas);

	// Clean
	for (int i = 0; i < threadCount; ++i) {
		completeThread(&threadDatas[i]);
	}



	// Clean
	delete [] threadDatas;
	delete [] gd.Tinv;
	delete [] gd.T;
	delete [] gd.simplexPoints;



	// Done
	return okay ? 0 : -2;
}




