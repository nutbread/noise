#include <cassert>
#include <limits>



namespace hyperloop {


namespace triangle {


template <typename Integer, typename DataType, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn>
void
iterate(
	Integer dimensions,
	Integer maximum,
	DataType data
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);

	// Init
	Integer* indices = new Integer[dimensions];
	Integer i, v;
	Integer d = dimensions - 1;
	Integer max2 = maximum - d;

	for (i = 0; i < dimensions; ++i) {
		indices[i] = i;
	}

	i = d;

	while (true) {
		// Function
		if (ItFn != nullptr) {
			ItFn(dimensions, maximum, indices, data);
		}

		// Index updates
		if (++indices[i] >= maximum) {
			do {
				if (i == 0) return; // done
				--i;
			}
			while (++indices[i] >= max2 + i); // >= (maximum - (d - i))

			v = indices[i];
			while (i < d) {
				indices[++i] = ++v;
			}
		}
	}
}

template <typename Integer>
Integer
iterations(
	Integer dimensions,
	Integer maximum
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);

	Integer product = maximum;
	Integer divisor = dimensions;

	while (--dimensions != 0) {
		product *= --maximum;
		divisor *= dimensions;
	}

	return product / divisor;
}


}


namespace triangle_eq {


template <typename Integer, typename DataType, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn>
void
iterate(
	Integer dimensions,
	Integer maximum,
	DataType data
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);

	// Fast exit
	if (maximum == 0) return;

	// Init
	Integer* indices = new Integer[dimensions];
	Integer i, v;
	Integer d = dimensions - 1;

	for (i = 0; i < dimensions; ++i) {
		indices[i] = 0;
	}

	i = d;

	while (true) {
		// Function
		if (ItFn != nullptr) {
			ItFn(dimensions, maximum, indices, data);
		}

		// Index updates
		if (++indices[i] >= maximum) {
			do {
				if (i == 0) return; // done
			}
			while (++indices[--i] >= maximum);

			v = indices[i];
			while (i < d) {
				indices[++i] = v;
			}
		}
	}
}

template <typename Integer, typename DataType, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn>
void
iterate_range(
	Integer dimensions,
	Integer maximum,
	Integer iteration_start,
	Integer iteration_count,
	DataType data
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);
	assert(iteration_start >= 0);
	assert(iteration_start < iterations<Integer>(dimensions, maximum));
	assert(iteration_count >= 0);
	assert(iteration_count <= iterations<Integer>(dimensions, maximum) - iteration_start);

	// Fast exit
	if (maximum == 0 || iteration_count == 0) return;

	// Init
	Integer* indices = new Integer[dimensions];
	Integer d = dimensions - 1;
	Integer i = d;
	Integer v;

	permutation<Integer>(
		dimensions,
		maximum,
		iteration_start,
		indices
	);

	while (true) {
		// Function
		if (ItFn != nullptr) {
			ItFn(dimensions, maximum, indices, data);
		}

		// Done
		if (--iteration_count == 0) return;

		// Index updates
		if (++indices[i] >= maximum) {
			// Done check not necessary
			while (++indices[--i] >= maximum);

			v = indices[i];
			while (i < d) {
				indices[++i] = v;
			}
		}
	}
}

template <typename Integer>
Integer
iterations(
	Integer dimensions,
	Integer maximum
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);

	Integer product = maximum;
	Integer divisor = dimensions;

	while (--dimensions != 0) {
		product *= ++maximum;
		divisor *= dimensions;
	}

	return product / divisor;
}

template <typename Integer>
void
permutation(
	Integer dimensions,
	Integer maximum,
	Integer index,
	Integer* result
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(maximum >= 0);
	assert(index >= 0);
	assert(index < iterations<Integer>(dimensions, maximum));
	assert(result != nullptr);

	Integer i = 0;
	Integer its;
	Integer itSum = 0;

	if (--dimensions == 0) {
		*result = index;
		return;
	}

	while (true) {
		its = itSum + iterations<Integer>(dimensions, maximum);
		if (index < its) {
			*(result++) = i;
			if (--dimensions == 0) {
				*result = index - itSum + i;
				return;
			}
			index -= itSum;
			itSum = 0;
		}
		else {
			itSum = its;
			++i;
			--maximum;
		}
	}
}


}


namespace triangle_sum {


template <typename Integer, typename DataType, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn>
void
iterate(
	Integer dimensions,
	Integer sum,
	DataType data
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(sum > 0);

	// Init
	Integer* sums = new Integer[dimensions * 2 + 1]; // sums uses Integer[dimensions], init to: { sum , sum , ... , sum }
	Integer* indices = &sums[dimensions]; // indices uses Integer[dimensions + 1], init to: { sum , 0 , 0 , ... , 0 }
	Integer i;

	for (i = 0; i <= dimensions; ++i) {
		sums[i] = sum;
	}
	for (i = 1; i <= dimensions; ++i) {
		indices[i] = 0;
	}

	while (true) {
		// Function
		if (ItFn != nullptr) {
			ItFn(dimensions, sum, indices, data);
		}

		// Loop update
		i = dimensions;
		while (true) {
			--i;

			if (indices[i] != 0) {
				--indices[i];
				--sums[i];
				indices[dimensions] = 0;
				indices[i + 1] = sum - sums[i];
				break;
			}
			else { // if (indices[i] == 0) {
				// Completed
				if (i == 0) {
					// Cleanup
					delete [] sums;
					return;
				}

				sums[i] = sum;
			}
		}
	}
}

template <typename Integer, typename DataType, typename FunctionHelper<Integer, DataType>::IterationFunction ItFn>
void
iterate_range(
	Integer dimensions,
	Integer sum,
	Integer iteration_start,
	Integer iteration_count,
	DataType data
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(sum > 0);
	assert(iteration_start >= 0);
	assert(iteration_start < iterations<Integer>(dimensions, sum));
	assert(iteration_count >= 0);
	assert(iteration_count <= iterations<Integer>(dimensions, sum) - iteration_start);

	// Fast exit
	if (iteration_count == 0) return;

	// Init
	Integer* sums = new Integer[dimensions * 2 + 1]; // sums uses Integer[dimensions]
	Integer* indices = &sums[dimensions]; // indices uses Integer[dimensions + 1]
	Integer i;

	// Init indices/sums
	permutation<Integer>(
		dimensions,
		sum,
		iteration_start,
		indices
	);

	sums[0] = indices[0];
	for (i = 1; i < dimensions; ++i) {
		sums[i] = sums[i - 1] + indices[i];
	}

	while (true) {
		// Function
		if (ItFn != nullptr) {
			ItFn(dimensions, sum, indices, data);
		}

		// Termination
		if (--iteration_count == 0) {
			// Cleanup
			delete [] sums;
			return;
		}

		// Loop update
		i = dimensions;
		while (true) {
			--i;

			if (indices[i] != 0) {
				--indices[i];
				--sums[i];
				indices[dimensions] = 0;
				indices[i + 1] = sum - sums[i];
				break;
			}
			else { // if (indices[i] == 0) {
				// Completed
				assert(i != 0); // Shouldn't happen; the termination condition should cover this
				sums[i] = sum;
			}
		}
	}
}

template <typename Integer>
Integer
iterations(
	Integer dimensions,
	Integer sum
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(sum >= 0);

	Integer loop_count = sum + 1;
	Integer divisor = 1;
	Integer i = 1;
	Integer j = loop_count;
	Integer k;

	while (i < dimensions) {
		++i;
		++j;

		// Overflow
		if ((loop_count * j) / j != loop_count || (divisor * i) / i != divisor) {
			return 0;
		}

		divisor *= i;
		loop_count *= j;

		// Factorize divisor and loop_count to help prevent overflows
		for (k = divisor; k >= 2; --k) {
			if ((divisor % k) == 0 && (loop_count % k) == 0) {
				divisor /= k;
				loop_count /= k;
				k = divisor;
			}
		}
	}

	assert((loop_count % divisor) == 0);
	assert(loop_count >= 0);

	return loop_count / divisor;
}

template <typename Integer>
void
permutation(
	Integer dimensions,
	Integer sum,
	Integer index,
	Integer* result
) {
	assert(dimensions > 0);
	assert(dimensions < std::numeric_limits<Integer>::max());
	assert(sum > 0);
	assert(index >= 0);
	assert(index < iterations<Integer>(dimensions, sum));
	assert(result != nullptr);

	Integer n = --dimensions;
	Integer i = sum;
	Integer offset;

	// dimensions > 1 cases
	if (n > 0) {
		while (true) {
			offset = iterations<uint64_t>(n, sum - i);

			if (index < offset) {
				result[dimensions - n] = i;

				sum -= i;
				i = sum;

				if (n <= 1) break; // go to n == 0 case since it's linear
				--n;
			}
			else {
				index -= offset;
				assert(i > 0);
				--i;
			}
		}
	}

	// dimensions = 1 case
	sum -= index;
	result[dimensions] = sum;

	// inductive case
	result[dimensions + 1] = i - sum;
}


}


}


