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

The cool property of this transformation is that it preserves
the lexicographic order of the strings, that is
if `str1 < str2`, then also `string_to_uint64(str1) < string_to_uint64(str1)`.

This means that with this transformation we can search through a set
of strings via integer binary search which is much faster than
binary searching a set of strings.

If the base of representation is ASCII, than one symbol is coded in 1 byte,
thus we can manage strings of size at most 8 with a 64-bit unsigned integer.
