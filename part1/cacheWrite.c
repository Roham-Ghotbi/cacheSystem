/* Summer 2017 */
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "cacheWrite.h"
#include "getFromCache.h"
#include "mem.h"
#include "setInCache.h"
#include "../part2/hitRate.h"

/*
	Takes in a cache and a block number and evicts the block at that number
	from the cache. This does not change any of the bits in the cache but 
	checks if data needs to be written to main memory or and then makes 
	calls to the appropriate functions to do so.
*/
void evict(cache_t* cache, uint32_t blockNumber) {
	uint8_t valid = getValid(cache, blockNumber);
	uint8_t dirty = getDirty(cache, blockNumber);
	if (valid && dirty) {
		uint32_t tag = extractTag(cache, blockNumber);
		uint32_t address = extractAddress(cache, tag, blockNumber, 0);
		writeToMem(cache, blockNumber, address);
	}
}

/*
	Takes in a cache, an address, a pointer to data, and a size of data
	and writes the updated data to the cache. If the data block is already
	in the cache it updates the contents and sets the dirty bit. If the
	contents are not in the cache it is written to a new slot and 
	if necessary something is evicted from the cache.
*/
void writeToCache(cache_t* cache, uint32_t address, uint8_t* data, uint32_t dataSize) {
	// Omid:
	evictionInfo_t* writeBlock = findEviction(cache, address);			// Get cache block we will try to write to
	uint32_t tag = getTag(cache, address);
	uint32_t blockNumber = writeBlock->blockNumber;
	if (writeBlock->match) {												// If it's a hit
		writeDataToCache(cache, address, data, dataSize, tag, writeBlock);
		reportHit(cache);
		reportAccess(cache);
	} else {																// If it's a miss
		evict(cache, blockNumber);
		uint8_t* newBlock = malloc(sizeof(uint8_t) * cache->blockDataSize);
		if (newBlock == NULL) {
			allocationFailed();
		}
		int memAddress = address - (address % cache->blockDataSize);
		uint8_t* memData;
		for (int i = 0; i < cache->blockDataSize; i++) {
			memData = readFromMem(cache, memAddress + i);
			newBlock[i] = *memData;
			free(memData);
		}
		setValid(cache, blockNumber, 1);
		setShared(cache, blockNumber, 0);
		setTag(cache, tag, blockNumber);
		setData(cache, newBlock, blockNumber, cache->blockDataSize, 0);
		writeDataToCache(cache, address, data, dataSize, tag, writeBlock);
		reportAccess(cache);
		free(newBlock);
	}
	free(writeBlock);
	// End Omid
}

/*
	Takes in a cache, an address to write to, a pointer containing the data
	to write, the size of the data, a tag, and a pointer to an evictionInfo
	struct and writes the data given to the cache based upon the location
	given by the evictionInfo struct.
*/
void writeDataToCache(cache_t* cache, uint32_t address, uint8_t* data, uint32_t dataSize, uint32_t tag, evictionInfo_t* evictionInfo) {
	uint32_t idx = getIndex(cache, address);
	setData(cache, data, evictionInfo->blockNumber, dataSize , getOffset(cache, address));
	setDirty(cache, evictionInfo->blockNumber, 1);
	setValid(cache, evictionInfo->blockNumber, 1);
	setShared(cache, evictionInfo->blockNumber, 0);
	updateLRU(cache, tag, idx, evictionInfo->LRU);
}

/*
	Takes in a cache, an address, and a byte of data and writes the byte
	of data to the cache. May evict something if the block is not already
	in the cache which may also require a fetch from memory. Returns -1
	if the address is invalid, otherwise 0.
*/
int writeByte(cache_t* cache, uint32_t address, uint8_t data) {
	if (!validAddresses(address, 1)) {
		return -1;									// Check if address is valid
	} else {
		writeToCache(cache, address, &data, 1);
		return 0;
	}
}

/*
	Takes in a cache, an address, and a halfword of data and writes the
	data to the cache. May evict something if the block is not already
	in the cache which may also require a fetch from memory. Returns 0
	for a success and -1 if there is an allignment error or an invalid
	address was used.
*/
int writeHalfWord(cache_t* cache, uint32_t address, uint16_t data) {
	if (!validAddresses(address, 2) || (address % 2 != 0)) {
		return -1;												// Check if address is valid and alignment is correct
	} else {
		if (cache->blockDataSize == 1) {
			uint8_t firstByte = (uint8_t) (data >> 8);
			uint8_t secondByte = (uint8_t) ((data << 8) >> 8);
			writeByte(cache, address, firstByte);
			writeByte(cache, address + 1, secondByte);
		} else {
			uint8_t* writeData = malloc(sizeof(uint8_t) * 2);
			if (writeData == NULL) {
				allocationFailed();
				return -1;
			}
			for (int i = 0; i < 2; i++) {
				writeData[i] = ((data << (i * 8)) >> 8);
			}
			writeToCache(cache, address, writeData, 2);
			free(writeData);
		}
		return 0;
	}
}

/*
	Takes in a cache, an address, and a word of data and writes the
	data to the cache. May evict something if the block is not already
	in the cache which may also require a fetch from memory. Returns 0
	for a success and -1 if there is an allignment error or an invalid
	address was used.
*/
int writeWord(cache_t* cache, uint32_t address, uint32_t data) {
	if (!validAddresses(address, 4) || (address % 4 != 0)) {
		return -1;												// Check if address is valid and alignment is correct
	} else {
		// uint8_t firstByte = (uint8_t) (data >> 24);
		// uint8_t secondByte = (uint8_t) ((data << 8) >> 24);
		// uint8_t thirdByte = (uint8_t) ((data << 16) >> 24);
		// uint8_t fourthByte = (uint8_t) ((data << 24) >> 24);
		// writeByte(cache, address, firstByte);
		// writeByte(cache, address, secondByte);
		// writeByte(cache, address, thirdByte);
		// writeByte(cache, address, fourthByte);
		if (cache->blockDataSize < 4) {
			uint16_t firstHalfWord = (uint16_t) (data >> 16);
			uint16_t secondHalfWord = (uint16_t) ((data << 16) >> 16);
			writeHalfWord(cache, address, firstHalfWord);
			writeHalfWord(cache, address + 2, secondHalfWord);
		} else {
			uint8_t* writeData = malloc(sizeof(uint8_t) * 4);
			if (writeData == NULL) {
				allocationFailed();
				return -1;
			}
			for (int i = 0; i < 4; i++) {
				writeData[i] = ((data << (i * 8)) >> 24);
			}
			writeToCache(cache, address, writeData, 4);
			free(writeData);
		}
		return 0;
	}
}

/*
	Takes in a cache, an address, and a double word of data and writes the
	data to the cache. May evict something if the block is not already
	in the cache which may also require a fetch from memory. Returns 0
	for a success and -1 if there is an allignment error or an invalid address
	was used.
*/
int writeDoubleWord(cache_t* cache, uint32_t address, uint64_t data) {
	if (!validAddresses(address, 4) || (address % 8 != 0)) {
		return -1;												// Check if address is valid and alignment is correct
	} else {
		// uint8_t firstByte = (uint8_t) (data >> 56);
		// uint8_t secondByte = (uint8_t) ((data << 8) >> 56);
		// uint8_t thirdByte = (uint8_t) ((data << 16) >> 56);
		// uint8_t fourthByte = (uint8_t) ((data << 24) >> 56);
		// uint8_t fifthByte = (uint8_t) ((data << 32) >> 56);
		// uint8_t sixthByte = (uint8_t) ((data << 40) >> 56);
		// uint8_t seventhByte = (uint8_t) ((data << 48) >> 56);
		// uint8_t eighthByte = (uint8_t) ((data << 56) >> 56);
		// writeByte(cache, address, firstByte);
		// writeByte(cache, address, secondByte);
		// writeByte(cache, address, thirdByte);
		// writeByte(cache, address, fourthByte);
		// writeByte(cache, address, fifthByte);
		// writeByte(cache, address, sixthByte);
		// writeByte(cache, address, seventhByte);
		// writeByte(cache, address, eighthByte);
		if (cache->blockDataSize < 8) {
			uint32_t firstWord = (uint32_t) (data >> 32);
			uint32_t secondWord = (uint32_t) ((data << 32) >> 32);
			writeWord(cache, address, firstWord);
			writeWord(cache, address + 4, secondWord);
		} else {
			uint8_t* writeData = malloc(sizeof(uint8_t) * 8);
			if (writeData == NULL) {
				allocationFailed();
				return -1;
			}
			for (int i = 0; i < 8; i++) {
				writeData[i] = ((data << (i * 8)) >> 56);
			}
			writeToCache(cache, address, writeData, 8);
			free(writeData);
		}
		return 0;
	}
}

/*
	A function used to write a whole block to a cache without pulling it from
	physical memory. This is useful to transfer information between caches
	without needing to take an intermediate step of going through main memory,
	a primary advantage of MOESI. Takes in a cache to write to, an address
	which is being written to, the block number that the data will be written
	to and an entire block of data from another cache.
*/
void writeWholeBlock(cache_t* cache, uint32_t address, uint32_t evictionBlockNumber, uint8_t* data) {
	uint32_t idx = getIndex(cache, address);
	uint32_t tagVal = getTag(cache, address);
	int oldLRU = getLRU(cache, evictionBlockNumber);
	evict(cache, evictionBlockNumber);
	setValid(cache, evictionBlockNumber, 1);
	setDirty(cache, evictionBlockNumber, 0);
	setTag(cache, tagVal, evictionBlockNumber);
	setData(cache, data, evictionBlockNumber, cache->blockDataSize, 0);
	updateLRU(cache, tagVal, idx, oldLRU);
}
