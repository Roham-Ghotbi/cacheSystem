/* Summer 2017 */
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "setInCache.h"
#include "cacheRead.h"
#include "cacheWrite.h"
#include "getFromCache.h"
#include "mem.h"
#include "../part2/hitRate.h"

/*
	Takes in a cache and a block number and fetches that block of data, 
	returning it in a uint8_t* pointer.
*/
uint8_t* fetchBlock(cache_t* cache, uint32_t blockNumber) {
	uint64_t location = getDataLocation(cache, blockNumber, 0);
	uint32_t length = cache->blockDataSize;
	uint8_t* data = malloc(sizeof(uint8_t) << log_2(length));
	if (data == NULL) {
		allocationFailed();
	}
	int shiftAmount = location & 7;
	uint64_t byteLoc = location >> 3;
	if (shiftAmount == 0) {
		for (uint32_t i = 0; i < length; i++) {
			data[i] = cache->contents[byteLoc + i];
		}
	} else {
		length = length << 3;
		data[0] = cache->contents[byteLoc] << shiftAmount;
		length -= (8 - shiftAmount);
		int displacement = 1;
		while (length > 7) {
			data[displacement - 1] = data[displacement - 1] | (cache->contents[byteLoc + displacement] >> (8 - shiftAmount));
			data[displacement] = cache->contents[byteLoc + displacement] << shiftAmount;
			displacement++;
			length -= 8;
		}
		data[displacement - 1] = data[displacement - 1] | (cache->contents[byteLoc + displacement] >> (8 - shiftAmount));
	}
	return data;
}

/*
	Takes in a cache, an address, and a dataSize and reads from the cache at
	that address the number of bytes indicated by the size. If the data block 
	is already in the cache it retrieves the contents. If the contents are not
	in the cache it is read into a new slot and if necessary something is 
	evicted.
*/
uint8_t* readFromCache(cache_t* cache, uint32_t address, uint32_t dataSize) {
	/* Your Code Here. */;
	evictionInfo_t* readBlock = findEviction(cache, address);			// Get cache block we will try to read from
	uint32_t tag = getTag(cache, address);
	uint32_t index = getIndex(cache, address);
	uint32_t offset = getOffset(cache, address);
	uint32_t blockNumber = readBlock->blockNumber;
	uint8_t* data;
	if (readBlock->match) {												// If it's a hit
		data = getData(cache, offset, blockNumber, dataSize);
		updateLRU(cache, tag, index, readBlock->LRU);
		reportHit(cache);
		reportAccess(cache);
		free(readBlock);
		return data;
	} else {															// If it's a miss
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
		setDirty(cache, blockNumber, 0);
		setShared(cache, blockNumber, 0);
		setTag(cache, tag, blockNumber);
		setData(cache, newBlock, blockNumber, cache->blockDataSize, 0);
		data = getData(cache, offset, blockNumber, dataSize);
		updateLRU(cache, tag, index, readBlock->LRU);
		reportAccess(cache);
		free(readBlock);
		free(newBlock);
		return data;
	}
}

/*
	Takes in a cache and an address and fetches a byte of data.
	Returns a struct containing a bool field of whether or not
	data was successfully read and the data. This field should be
	false only if there is an alignment error or there is an invalid
	address selected.
*/
byteInfo_t readByte(cache_t* cache, uint32_t address) {
	byteInfo_t retVal;
	// Omid:
	uint8_t* byteData = readFromCache(cache, address, 1);
	retVal.data = *byteData;
	free(byteData);
	if (!validAddresses(address, 1)) {
		retVal.success = false;									// Check if address is valid and alignment is correct
	} else {
		retVal.success = true;
	}
	// End Omid
	return retVal;
}

/*
	Takes in a cache and an address and fetches a halfword of data.
	Returns a struct containing a bool field of whether or not
	data was successfully read and the data. This field should be
	false only if there is an alignment error or there is an invalid
	address selected.
*/
halfWordInfo_t readHalfWord(cache_t* cache, uint32_t address) {
	halfWordInfo_t retVal;
	// Omid:
	/* if (cache->blockDataSize == 1) {
		uint16_t firstByte = *(readFromCache(cache, address, 1));
		uint16_t secondByte = *(readFromCache(cache, address + 1, 1));
		retVal.data = (firstByte << 8) | secondByte;

	} else {
		retVal.data = *(readFromCache(cache, address, 2));
	} */
	// uint16_t firstByte = *(readFromCache(cache, address, 1));
	// uint16_t secondByte = *(readFromCache(cache, address + 1, 1));
	// retVal.data = (firstByte << 8) | secondByte;

	if (cache->blockDataSize == 1) {
		byteInfo_t firstByte = readByte(cache, address);
		byteInfo_t secondByte = readByte(cache, address + 1);
		retVal.data = (((uint16_t) firstByte.data) << 8) | (uint16_t) secondByte.data;
	} else {
		uint8_t* cacheData = readFromCache(cache, address, 2);
		uint8_t firstByte = cacheData[0];
		uint8_t secondByte = cacheData[1];
		retVal.data = (((uint16_t) firstByte) << 8) | (uint16_t) secondByte;
		free(cacheData);
	}
	if (!validAddresses(address, 2) || (address % 2 != 0)) {
		retVal.success = false;									// Check if address is valid and alignment is correct
	} else {
		retVal.success = true;
	}
	// End Omid
	return retVal;
}

/*
	Takes in a cache and an address and fetches a word of data.
	Returns a struct containing a bool field of whether or not
	data was successfully read and the data. This field should be
	false only if there is an alignment error or there is an invalid
	address selected.
*/
wordInfo_t readWord(cache_t* cache, uint32_t address) {
	wordInfo_t retVal;
	// Omid:
	/* if (cache->blockDataSize == 1) {
		uint32_t firstByte = *(readFromCache(cache, address, 1));
		uint32_t secondByte = *(readFromCache(cache, address + 1, 1));
		uint32_t thirdByte = *(readFromCache(cache, address + 2, 1));
		uint32_t fourthByte = *(readFromCache(cache, address + 3, 1));
		retVal.data = (((firstByte << 24) | (secondByte << 16)) | (thirdByte << 8)) | fourthByte;
	} else if (cache->blockDataSize == 2) {
		uint32_t firstHalfWord = *(readFromCache(cache, address, 2));
		uint32_t secondHalfWord = *(readFromCache(cache, address + 2, 2));
		retVal.data = (firstHalfWord << 16) | secondHalfWord;
	} else {
		retVal.data = *(readFromCache(cache, address, 4));
	} */
	// uint32_t firstByte = *(readFromCache(cache, address, 1));
	// uint32_t secondByte = *(readFromCache(cache, address + 1, 1));
	// uint32_t thirdByte = *(readFromCache(cache, address + 2, 1));
	// uint32_t fourthByte = *(readFromCache(cache, address + 3, 1));
	// retVal.data = (((firstByte << 24) | (secondByte << 16)) | (thirdByte << 8)) | fourthByte;
	if (cache->blockDataSize < 4) {
		halfWordInfo_t firstHalfWord = readHalfWord(cache, address);
		halfWordInfo_t secondHalfWord = readHalfWord(cache, address + 2);
		retVal.data = (((uint32_t) firstHalfWord.data) << 16) | (uint32_t) secondHalfWord.data;
	} else {
		uint8_t* cacheData = readFromCache(cache, address, 4);
		uint8_t firstByte = cacheData[0];
		uint8_t secondByte = cacheData[1];
		uint8_t thirdByte = cacheData[2];
		uint8_t fourthByte = cacheData[3];
		retVal.data = ((((uint32_t) firstByte << 24) | ((uint32_t) secondByte << 16)) | ((uint32_t) thirdByte << 8)) | (uint32_t) fourthByte;
		free(cacheData);
	}
	if (!validAddresses(address, 4) || (address % 4 != 0)) {
		retVal.success = false;									// Check if address is valid and alignment is correct
	} else {
		retVal.success = true;
	}
	// End Omid
	return retVal;
}

/*
	Takes in a cache and an address and fetches a double word of data.
	Returns a struct containing a bool field of whether or not
	data was successfully read and the data. This field should be
	false only if there is an alignment error or there is an invalid
	address selected.
*/
doubleWordInfo_t readDoubleWord(cache_t* cache, uint32_t address) {
	doubleWordInfo_t retVal;
	// Omid:
	/* if (cache->blockDataSize == 1) {
		uint64_t firstByte = *(readFromCache(cache, address, 1));
		uint64_t secondByte = *(readFromCache(cache, address + 1, 1));
		uint64_t thirdByte = *(readFromCache(cache, address + 2, 1));
		uint64_t fourthByte = *(readFromCache(cache, address + 3, 1));
		uint64_t fifthByte = *(readFromCache(cache, address + 4, 1));
		uint64_t sixthByte = *(readFromCache(cache, address + 5, 1));
		uint64_t seventhByte = *(readFromCache(cache, address + 6, 1));
		uint64_t eighthByte = *(readFromCache(cache, address + 7, 1));
		retVal.data = (((((((firstByte << 56) | (secondByte << 48)) | (thirdByte << 40)) | (fourthByte << 32))
						| (fifthByte << 24)) | (sixthByte << 16)) | (seventhByte << 8)) | eighthByte;
	} else if (cache->blockDataSize == 2) {
		uint64_t firstHalfWord = *(readFromCache(cache, address, 2));
		uint64_t secondHalfWord = *(readFromCache(cache, address + 2, 2));
		uint64_t thirdHalfWord = *(readFromCache(cache, address + 4, 2));
		uint64_t fourthHalfWord = *(readFromCache(cache, address + 6, 2));
		retVal.data = (((firstHalfWord << 48) | (secondHalfWord << 32)) | (thirdHalfWord << 16)) | fourthHalfWord;
	} else if (cache->blockDataSize == 4) {
		uint64_t firstWord = *(readFromCache(cache, address, 4));
		uint64_t secondWord = *(readFromCache(cache, address + 4, 4));
		retVal.data = (firstWord << 32) | secondWord;
	} else {
		retVal.data = *(readFromCache(cache, address, 8));
	} */
	// uint64_t firstByte = *(readFromCache(cache, address, 1));
	// uint64_t secondByte = *(readFromCache(cache, address + 1, 1));
	// uint64_t thirdByte = *(readFromCache(cache, address + 2, 1));
	// uint64_t fourthByte = *(readFromCache(cache, address + 3, 1));
	// uint64_t fifthByte = *(readFromCache(cache, address + 4, 1));
	// uint64_t sixthByte = *(readFromCache(cache, address + 5, 1));
	// uint64_t seventhByte = *(readFromCache(cache, address + 6, 1));
	// uint64_t eighthByte = *(readFromCache(cache, address + 7, 1));
	// retVal.data = (((((((firstByte << 56) | (secondByte << 48)) | (thirdByte << 40)) | (fourthByte << 32))
	// 				| (fifthByte << 24)) | (sixthByte << 16)) | (seventhByte << 8)) | eighthByte;
	if (cache->blockDataSize < 8) {
		wordInfo_t firstWord = readWord(cache, address);
		wordInfo_t secondWord = readWord(cache, address + 4);
		retVal.data = (((uint64_t) firstWord.data) << 32) | (uint64_t) secondWord.data;
	} else {
		uint8_t* cacheData = readFromCache(cache, address, 8);
		uint8_t firstByte = cacheData[0];
		uint8_t secondByte = cacheData[1];
		uint8_t thirdByte = cacheData[2];
		uint8_t fourthByte = cacheData[3];
		uint8_t fifthByte = cacheData[4];
		uint8_t sixthByte = cacheData[5];
		uint8_t seventhByte = cacheData[6];
		uint8_t eighthByte = cacheData[7];
		retVal.data = ((((((((uint64_t) firstByte << 56) | ((uint64_t) secondByte << 48)) | ((uint64_t) thirdByte << 40)) | ((uint64_t) fourthByte << 32))
	 				| ((uint64_t) fifthByte << 24)) | ((uint64_t) sixthByte << 16)) | ((uint64_t) seventhByte << 8)) | (uint64_t) eighthByte;
		free(cacheData);
	}
	if (!validAddresses(address, 8) || (address % 8 != 0)) {
		retVal.success = false;									// Check if address is valid and alignment is correct
	} else {
		retVal.success = true;
	}
	// End Omid
	return retVal;
}
