Integer Search for Strings
--------------------------

Usage:

    ./integer_search_for_strings <sorted_strings_filename>

--------------------------

From a string whose size if <= 8 obtain its 64-bit integer representation
as follows:

```C++
uint64_t string_to_uint64(std::string const& str) {
    std::string tmp = str;
    std::reverse(tmp.begin(), tmp.end());
    return *reinterpret_cast<uint64_t const*>(tmp.data());
}
```
An even faster version assuming that the input string is of size at least 8
is given below:

```C++
#include <immintrin.h>  // for __builtin_bswap64
uint64_t string8_to_uint64(std::string const& str) {
	return __builtin_bswap64(*reinterpret_cast<uint64_t const*>(str.data()));
}
```

The cool property of this transformation is that it preserves
the lexicographic order of the strings, that is
if `str1 < str2`, then also `string_to_uint64(str1) < string_to_uint64(str1)`.

This means that with this transformation we can search through a set
of strings via integer binary search which is much faster than
binary searching a set of strings.

If the base of representation is ASCII, than one symbol is coded in 1 byte,
thus we can manage strings of size at most 8 with a 64-bit unsigned integer.

I've got a 2.4X time improvement on a recent processor
and a large dataset of strings (AOL):

	giulio@and:~/experiments/build$ ./integer_search_for_strings ~/aol.sorted 
	read 1000000 strings
	read 2000000 strings
	read 3000000 strings
	read 4000000 strings
	read 5000000 strings
	read 6000000 strings
	read 7000000 strings
	read 8000000 strings
	read 9000000 strings
	read 10000000 strings
	num_strings 10142395
	max_string_length 126
	total_length 222967274
	avg_string_length 21.98
	elapsed 901563
	##ignore 107790308
	integer vector IS SORTED
	elapsed 372834
	##ignore 16296122582491315816