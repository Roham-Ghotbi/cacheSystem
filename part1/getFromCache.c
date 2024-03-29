/* Summer 2017 */
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
// #include <omp.h>
#include "utils.h"
#include "getFromCache.h"

/*
	Takes in a cache and a blocknumber and returns that block's valid bit.
*/
uint8_t getValid(cache_t* cache, uint32_t blockNumber) {
    //Roham
    return getBit(cache, getValidLocation(cache, blockNumber));
    //EndRoham
}

/*
	Takes in a cache and a blocknumber and returns that block's dirty bit.
*/
uint8_t getDirty(cache_t* cache, uint32_t blockNumber) {
    //Roham
    return getBit(cache, getDirtyLocation(cache, blockNumber));
    //EndRoham
}

/*
	Takes in a cache and a blocknumber and returns that block's shared bit.
*/
uint8_t getShared(cache_t* cache, uint32_t blockNumber) {
    //Roham
    return getBit(cache, getSharedLocation(cache, blockNumber));
    //EndRoham
}

/* 
	Takes in a cache and a location of the cache in bits and returns the
	value of the bit at that location.
*/
uint8_t getBit(cache_t* cache, uint64_t location) {
	//Roham
    uint64_t locationByte = location / 8;
    uint8_t remainder = location % 8;
    uint8_t mask = 1;
	return (((uint8_t)(cache->contents[locationByte] << remainder)) >> 7) & mask;
    //EndRoham
}

/*
	Takes a cache and a block number and extracts the value of the LRU 
	for the block specified.
*/
long getLRU(cache_t* cache, uint32_t blockNumber) {
    //Roham
    uint8_t LRUSize = numLRUBits(cache);                        //get the size of LRU in bits
    uint64_t location = getLRULocation(cache, blockNumber);

    /*if(LRUSize + remainder > 8) {                               //getting the next byte if data is stored in two different bytes
        uint8_t byteData2 = cache->contents[byteLocation + 1];  //and also performing the process differently
        uint8_t NumBitsForPart2 = LRUSize - (8 - remainder);
        uint8_t shift2 = 8 - NumBitsForPart2;

        //part1
        uint8_t part1 = (uint8_t)(byteData1 << remainder) >> remainder;
        //part2
        uint8_t part2 = byteData2 >> shift2;
        return (part1 << NumBitsForPart2) + part2;
    } else {
        return byteData1 >> shift;          //if it starts from the beginning
    }*/

    //written using getBit
    uint8_t LRU = 0;
    for(int i = 0; i < LRUSize;i++) {
        LRU += getBit(cache, location + i) << (LRUSize - 1 - i);

    }
    return LRU;
    //EndRoham
}

/*
	Takes a cache and a block number and extracts the value of the tag 
	for the block specified.
*/
uint32_t extractTag(cache_t* cache, uint32_t blockNumber) {
	uint64_t location = getTagLocation(cache, blockNumber);
	uint64_t byteLoc = location >> 3;
	int shiftAmount = location & 7;
	if (shiftAmount != 0) {
		uint64_t newTag = (((uint64_t)  cache->contents[byteLoc]) << 56) + (((uint64_t) cache->contents[byteLoc + 1]) << 48)
	 	+ (((uint64_t)  cache->contents[byteLoc + 2]) << 40) + (((uint64_t) cache->contents[byteLoc + 3]) << 32)
	 	+ (((uint64_t) cache->contents[byteLoc + 4]) << 24);
	 	newTag = (newTag  << (shiftAmount));
	 	newTag = newTag >> (64 - getTagSize(cache));
		return ((uint32_t) newTag);
	} else {
		uint32_t newTag = (((uint32_t)  cache->contents[byteLoc]) << 24) + ((uint32_t) cache->contents[byteLoc + 1] << 16)
	 	+ (((uint32_t)  cache->contents[byteLoc + 2]) << 8) + ((uint32_t) cache->contents[byteLoc + 3]);
	 	newTag = newTag >> (32 - getTagSize(cache));
	 	return newTag;
	}
}

/*
	Takes a cache and a block number and extracts the value of the index 
	for the block specified.
*/
uint32_t extractIndex(cache_t* cache, uint32_t blockNumber) {
	return (uint32_t) (blockNumber >> log_2(cache->n));
}

/*
	Takes in a cache, a tag, a blocknumber, and an offset and extracts the
	original address.
*/
uint32_t extractAddress(cache_t* cache, uint32_t tag, uint32_t blockNumber, uint32_t offset) {
    //Roham
	uint32_t index = extractIndex(cache, blockNumber);
    uint32_t offsetSize = log_2(cache->blockDataSize);
    uint32_t indexSize = log_2(getNumSets(cache));
	return (tag << (indexSize + offsetSize)) + (index << offsetSize) + offset;
    //EndRoham
}

/*
	Takes in a cache and an address and finds the next block that should be 
	used for a cache operation on the address provided. If this address is
	already in memory it should return the block at which this address's
	operation would occur and indicates that this was a successful match.
	If this address is not stored in the cache then it should point to the next
	block that needs to be evicted as indicated by our LRU replacement system.
	If there are multiple blocks that could be evicted selects the
	block that occurs earlier in the cache. Returns a pointer to a struct
	which contains a block number, an LRU value, and whether or not the address
	is already stored in the cache (is a match).
*/
evictionInfo_t* findEviction(cache_t* cache, uint32_t address) {
	evictionInfo_t* info;
	info = malloc(sizeof(evictionInfo_t));
	if (info == NULL) {
		allocationFailed();
	}
	//Roham
    //Another solution
    uint32_t tag = getTag(cache, address);
    uint32_t index = getIndex(cache, address);
    uint8_t evictionLRU = (uint8_t) (cache->n - 1);
    uint32_t evictionBlocknumber = 0;
    uint32_t blockNumber = index * cache->n;                  //get the beginning of the block address might be in based on the index
    int invalidBlock = 0;
    int found = 0;
    for (int i = 0; i < cache->n; i++) {

        if(getValid(cache, blockNumber + i)) {
            if(tagEquals(blockNumber + i, tag, cache)){
                info->LRU = getLRU(cache, blockNumber + i);
                info->match = 1;
                info->blockNumber = blockNumber + i;
                found = 1;
                i = cache-> n;

            } else if((getLRU(cache, blockNumber + i) == evictionLRU) && (invalidBlock == 0)) {
                evictionBlocknumber = blockNumber + i;
                invalidBlock = 1;
            }
        } else if((getLRU(cache, blockNumber + i) == evictionLRU) && (invalidBlock == 0)) {
            evictionBlocknumber = blockNumber + i;
            invalidBlock = 1;
        }
    }
    if(found == 0) {
        info->blockNumber = evictionBlocknumber;
        info->match = 0;
        info->LRU = evictionLRU;
    }

    return info;
    //EndRoham
}

/*
	Takes in a cache and an address and returns the LRU
	value of that address in the cache. Used mostly for testing.
	Returns -1 if the information is not present in the cache.
*/
long getLRUAddress(cache_t* cache, uint32_t address){
	uint32_t tag;
	uint32_t idx = getIndex(cache, address);
	long tempLRU;
	for (int i = 0; i < cache->n; i++) {
		tempLRU = getLRU(cache, (idx << log_2(cache->n)) + i);
		tag = getTag(cache, address);
		if (tagEquals((idx << log_2(cache->n)) + i, tag, cache)) {
			return tempLRU;
		}
	}
	return -1;
}

/*
	Takes in a cache, an address, a blocknumber, and a size and
	returns a pointer to an array of the data that was read. ASSUMES THAT
	ALL THE DATA FITS IN THE BLOCK. This should be handled by a function
	higher up that calls this function.
*/
uint8_t* getData(cache_t* cache, uint32_t offset, uint32_t blockNumber, uint32_t size) {
	uint8_t* data;
	uint8_t mask;
	uint64_t location = getDataLocation(cache, blockNumber, offset);
	uint64_t byteLoc = location >> 3;
	uint8_t shiftAmount = location & 7;
	data = (uint8_t*) malloc(sizeof(uint8_t) * size);
	if (data == NULL) {
		allocationFailed();
	}
	if (shiftAmount == 0) {
		for (uint32_t i = 0; i < size; i++) {
			data[i] = cache->contents[byteLoc + i];
		} 
	} else {
		mask = 0;
		for (uint8_t j = 0; j < shiftAmount; j++) {
			mask += (1 << j);
		}
		for (uint32_t i = 0; i < size; i++) {
			data[i] = cache->contents[byteLoc + i] << shiftAmount;
			data[i] = data[i] | ((cache->contents[byteLoc + i + 1] >> (8 - shiftAmount)) & mask);
		}
	}
	return data;
}