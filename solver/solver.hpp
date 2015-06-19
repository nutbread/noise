#ifndef ___H_SOLVER
#define ___H_SOLVER

#include <cstdint>



// Types
class GlobalData;
class ThreadData;

class GlobalData {
public:
	typedef double Double;

	CRITICAL_SECTION lock;

	int threadCount;

	bool displayEvery;

	uint64_t dimensions;
	uint64_t points;
	uint64_t pointsLarge;

	uint64_t gradientCount;
	uint64_t iterationCount;
	uint64_t iterations;

	Double* T;
	Double* Tinv;
	Double* simplexPoints;

	int updates;
	Double minimum;
	Double maximum;

	GlobalData();
	~GlobalData();

	bool
	updateExtrema(
		const ThreadData* tData,
		Double minimum,
		Double maximum,
		bool updateCount
	);

};

class ThreadData {
public:
	typedef double Double;

	HANDLE threadHandle;
	GlobalData* globalData;
	int threadIndex;
	Double pointsDouble;
	uint64_t gradientCount;
	uint64_t iterationStart;
	uint64_t iterationCount;
	uint64_t iterations;
	Double* coordinates;
	Double* coordinatesTransformed;
	Double* T;
	Double* Tinv;
	Double* simplexPoints;
	Double* gradients;
	Double minimum;
	Double maximum;

	ThreadData();
	~ThreadData();

};



// Threading
int
getThreadCount();

bool
spawnThread(
	LPTHREAD_START_ROUTINE function,
	ThreadData* threadInfo
);

void
joinThreads(
	int threadCount,
	ThreadData* threadInfos
);

void
completeThread(
	ThreadData* threadInfo
);



// Primary looping functions
DWORD WINAPI
threadFunction(
	void* data
);

void gradientLoop(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
);

void
iterateFunctionPow3(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
);

void
iterateFunctionPow4(
	uint64_t dimensions,
	uint64_t sum,
	const uint64_t* indices,
	ThreadData* tData
);



// Main
int
main(
	int argc,
	char** argv
);



#endif // ___H_SOLVER


