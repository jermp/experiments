Sorted sets with `memmove`
-

Usage:

    ./memmove
-

Often we need to keep a sorted set of items. Classical solutions having log-time
complexity involve search trees, like Red-Black or AVL.
However, if the set to be kept is small, we can achieve a better running time
with a simple `std::vector`. Upon insertions (deletions), we just shift the items
by one location to the right (left). Despite the linear-time complexity,
we expect this to work well due to the cache (memory is contiguous in an array)
and code simplicity.

Shifting the elements can be achieved with [`memmove`](http://www.cplusplus.com/reference/cstring/memmove/).
The `sorted_vector` class implements this solution. 

Now, the question is: up to which set size (named `n` in the following)
the `sorted_vector` is faster than,
say, a regular `std::set` (Red-Black Tree)?

-

I run the `./memmove` program on my Intel i7 (@2.6 GHz) and obtained the following
result. For this example, the sets store 4-byte signed keys.

![](results/test_small.pdf)

![](results/test_large.pdf)

The `sorted_vector` solution is faster (by ~2X) up to approximately 800 keys, then
the difference starts to diminish till the curves finally cross around 2400.

Alright, if you have sets smaller than 2K keys, just stick to a `std::vector`:
it will be faster, simpler, cache-friendly, space-efficient (no space overhead wrt
a tree-shaped solution), and supports random access.

-

The plots can be draw by running:

	./memmove 2> results.txt
	python3 plot_timings.py results.txt small insert   0 150 std::set sorted_vector
	python3 plot_timings.py results.txt large insert 150 400 std::set sorted_vector
