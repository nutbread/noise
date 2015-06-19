#ifndef ___H_HYPERLOOP
#define ___H_HYPERLOOP



namespace hyperloop {


namespace triangle {


template <typename Integer, typename DataType>
class FunctionHelper final {
private:
	FunctionHelper();
	~FunctionHelper();

public:
	typedef void (*IterationFunction)(
		Integer dimensions,
		Integer maximum,
		const Integer* indices,
		DataType data
	);

};


/**
	Performs a triangular n-dimensional loop across n variables, with
	each successive variable starting with the index of the previous plus 1.
		for (i = 0; i < n; ++i) {
		for (j = i+1; j < n; ++j) {
		for (k = j+1; k < n; ++k) {
		...

	@param Integer
		The integer type to use for the loop variables
	@param DataType
		A type of data to be passed into the iteration function
	@param ItFn
		The iteration function used for execution during the loop

	@param dimensions
		Number of dimensions.
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@param data
		Custom data to pass into the function
*/
template <typename Integer=int, typename DataType=void*, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn=nullptr>
void
iterate(
	Integer dimensions,
	Integer maximum,
	DataType data
);

/**
	Get how many iterations a hyperloop will perform

	@param Integer
		The integer type to use for the loop variables

	@param dimensions
		Number of dimensions.
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@return
		The total number of iterations the loop has, or 0 if an overflow occurs
*/
template <typename Integer=int>
Integer
iterations(
	Integer dimensions,
	Integer maximum
);


}


namespace triangle_eq {


template <typename Integer, typename DataType>
class FunctionHelper final {
private:
	FunctionHelper();
	~FunctionHelper();

public:
	typedef void (*IterationFunction)(
		Integer dimensions,
		Integer maximum,
		const Integer* indices,
		DataType data
	);

};


/**
	Performs a triangular n-dimensional loop across n variables, with
	each successive variable starting with the index of the previous.
	The form is a generalized form of:
		for (i = 0; i < n; ++i) {
		for (j = i; j < n; ++j) {
		for (k = j; k < n; ++k) {
		...

	@param Integer
		The integer type to use for the loop variables
	@param DataType
		A type of data to be passed into the iteration function
	@param ItFn
		The iteration function used for execution during the loop

	@param dimensions
		Number of dimensions.
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@param data
		Custom data to pass into the function
*/
template <typename Integer=int, typename DataType=void*, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn=nullptr>
void
iterate(
	Integer dimensions,
	Integer maximum,
	DataType data
);

/**
	Performs a segment of an n-dimensional loop across n variables

	@param Integer
		The integer type to use for the loop variables
	@param DataType
		A type of data to be passed into the iteration function
	@param ItFn
		The iteration function used for execution during the loop

	@param dimensions
		Number of dimensions.
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@param iterationStart
		The first index of the iteration to start on
		Must be in the range [ 0 , iterations(dimensions, maximum) )
	@param iterationCount
		The number of iterations to perform
		Must be in the range [ 0 , iterations(dimensions, maximum) - iterationStart ]
	@param data
		Custom data to pass into the function
*/
template <typename Integer=int, typename DataType=void*, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn=nullptr>
void
iterate_range(
	Integer dimensions,
	Integer maximum,
	Integer iteration_start,
	Integer iteration_count,
	DataType data
);

/**
	Get how many iterations a hyperloop will perform

	@param Integer
		The integer type to use for the loop variables

	@param dimensions
		Number of dimensions.
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@return
		The total number of iterations the loop has, or 0 if an overflow occurs
*/
template <typename Integer=int>
Integer
iterations(
	Integer dimensions,
	Integer maximum
);

/**
	Get a specific permutation of a hyperloop after a specific amount of iterations.

	@param Integer
		The integer type to use for the loop variables

	@param dimensions
		The dimensionality of the loop
		Must be greater than 0
	@param maximum
		The maximum value each loop index should iterate to
	@param index
		The number of the permutation.
		Must be in the range [ 0 , iterations(...) )
	@param result
		The destination array to put the values into.
		The length needs to be at least [dimensions + 1]
*/
template <typename Integer=int>
void
permutation(
	Integer dimensions,
	Integer maximum,
	Integer index,
	Integer* result
);


}


namespace triangle_sum {


template <typename Integer, typename DataType>
class FunctionHelper final {
private:
	FunctionHelper();
	~FunctionHelper();

public:
	typedef void (*IterationFunction)(
		Integer dimensions,
		Integer sum,
		const Integer* indices,
		DataType data
	);

};


/**
	Performs an n-dimensional loop across n variables

	@param Integer
		The integer type to use for the loop variables
	@param DataType
		A type of data to be passed into the iteration function
	@param ItFn
		The iteration function used for execution during the loop

	@param dimensions
		One less than the number of dimensions.
			ie: n = dimensions + 1
		The reason why it's one less is because the last variable is inductive:
			v[n] = sum - Summation(v[i], i=1 to n-1)
		It also better correlates to how many for-loops would be used in a
		static version of the code.
		Must be greater than 0
	@param sum
		The value all of the loop variables should add to
		Must be greater than 0
	@param data
		Custom data to pass into the function
*/
template <typename Integer=int, typename DataType=void*, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn=nullptr>
void
iterate(
	Integer dimensions,
	Integer sum,
	DataType data
);

/**
	Performs a segment of an n-dimensional loop across n variables

	@param Integer
		The integer type to use for the loop variables
	@param DataType
		A type of data to be passed into the iteration function
	@param ItFn
		The iteration function used for execution during the loop

	@param dimensions
		One less than the number of dimensions.
		Must be greater than 0
	@param sum
		The value all of the loop variables should add to
		Must be greater than 0
	@param iterationStart
		The first index of the iteration to start on
		Must be in the range [ 0 , iterations(dimensions, sum) )
	@param iterationCount
		The number of iterations to perform
		Must be in the range [ 0 , iterations(dimensions, sum) - iterationStart ]
	@param data
		Custom data to pass into the function
*/
template <typename Integer=int, typename DataType=void*, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn=nullptr>
void
iterate_range(
	Integer dimensions,
	Integer sum,
	Integer iteration_start,
	Integer iteration_count,
	DataType data
);

/**
	Get how many iterations a hyperloop will perform

	@param Integer
		The integer type to use for the loop variables

	@param dimensions
		The dimensionality of the loop
		Must be greater than 0
	@param sum
		The value all of the loop variables should add to
		Must be greater than or equal to 0
	@return
		The total number of iterations the loop has, or 0 if an overflow occurs
*/
template <typename Integer=int>
Integer
iterations(
	Integer dimensions,
	Integer sum
);

/**
	Get a specific permutation of a hyperloop after a specific amount of iterations.

	@param Integer
		The integer type to use for the loop variables

	@param dimensions
		The dimensionality of the loop
		Must be greater than 0
	@param sum
		The value all of the loop variables should add to
		Must be greater than 0
	@param index
		The number of the permutation.
		Must be in the range [ 0 , iterations(...) )
	@param result
		The destination array to put the values into.
		The length needs to be at least [dimensions + 1]
*/
template <typename Integer=int>
void
permutation(
	Integer dimensions,
	Integer sum,
	Integer index,
	Integer* result
);


}


}



#include "hyperloop.t.hpp"



#endif // ___H_HYPERLOOP


