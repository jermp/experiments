Binary to char conversion
-------------------------

Usage:

	./bin_to_char_conversion <num_vectors> <alg> <output_filename>
	Option 'alg' includes: 'operator<<', 'manual', 'binary'.


-------------

In this experiment, we create a collection of `std::vector<uint32_t>`, of possibly different lengths, and write all vectors to a file converting the `uint32_t` numbers to their string representation.

We compare: the standard `operator<<` of `std::ostream`, a manual conversion routine (as suggested by Jarno Alanko), and a routine that does not perform any conversion but directly writes the numbers in binary format.

Here are some performance numbers on MacBook Pro with a 2.3 GHz Dual-Core Intel Core i5 processor.


	./bin_to_char_conversion 500 'operator<<' /dev/null
	collection created in: 314 [millisec]
	elapsed time: 2875 [millisec]
	
	./bin_to_char_conversion 500 'manual' /dev/null
	collection created in: 316 [millisec]
	elapsed time: 370 [millisec]
	
	./bin_to_char_conversion 500 'binary' /dev/null
	collection created in: 315 [millisec]
	elapsed time: 7 [millisec]
	
Conclusions: a manual conversion function can be nearly 10X faster than `operator<<`, but directly writing binary data is so much faster (here, more than 400X faster).