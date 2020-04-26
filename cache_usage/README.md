Cache Usage
-

Usage:

    ./cache_usage <size>

A program that simulates a hardware cache and its usage.
In particular, it counts how many addresses
are mapped to the same cache set.
It can be useful to discover potential cache aliasing problems,
occurring when many different addresses are mapped to the same set.

In this test example, I am using a typical 8-way set-associative L1 cache
of size 32 KiB (the class `cache` in the code can be configured to simulate
any cache).

I am inspecting the cache usage for 4 different prefix-sum data structures
(only queries are considered here for simplicity; updates are not implemented):
(1) Fenwick-Tree; (2) Fenwick-Tree with holes; (3) Top-Down Segment-Tree;
(4) Bottom-Up Segment-Tree.

A workload of 10000 random queries is used.

The program shows that the Fenwick-Tree suffers from cache-aliasing for big
values of `<size>`, whereas all other implementations do not.

For example

    ./cache_usage 50000000

shows - on my Macbook Pro - that the first set of the cache is used 81588 times for the Fenwick-Tree,
but only 13107 for the Fenwick-Tree with holes.
