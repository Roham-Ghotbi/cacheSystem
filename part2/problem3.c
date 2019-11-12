/* Summer 2017 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../part1/utils.h"
#include "problem2.h"

/*
	Function used to return the result of the third cache parameters question.
*/
cache_t* params3() {
	uint32_t n = 4;
	uint32_t blockDataSize = 64;
	uint32_t totalDataSize = 512;
	char* fileName;
	fileName = "testFiles/physicalMemory1.txt";

	return createCache(n,blockDataSize, totalDataSize, fileName);
}