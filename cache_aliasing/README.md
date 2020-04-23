Cache Aliasing
-

Usage:
	
	./cache_aliasing <size>

On an Intel i9-9940X processor with the following L1 cache
	
	LEVEL1_ICACHE_SIZE                 32768
	LEVEL1_ICACHE_ASSOC                8
	LEVEL1_ICACHE_LINESIZE             64

I get a 3X slower execution when all accesses are directed to the same
cache set:

	giulio@and:~/experiments/build$ ./cache_aliasing 5000000
	stride 512
	# ignore 9765
	elapsed time: 10 [millisec]
	# ignore 9765
	elapsed time: 3 [millisec]