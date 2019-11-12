/* Summer 2017 */
#include <stdbool.h>
#include <stdint.h>
#include "coherenceUtils.h"
#include "coherenceRead.h"
#include "../part1/utils.h"
#include "../part1/setInCache.h"
#include "../part1/getFromCache.h"
#include "../part1/cacheRead.h"
#include "../part1/cacheWrite.h"
#include "../part1/mem.h"
#include "../part2/hitRate.h"

/*
	A function which processes all cache reads for an entire cache system.
	Takes in a cache system, an address, an id for a cache, and a size to read
	and calls the appropriate functions on the cache being selected to read
	the data. Returns the data.
*/
uint8_t* cacheSystemRead(cacheSystem_t* cacheSystem, uint32_t address, uint8_t ID, uint8_t size) {
	uint8_t* retVal;
	evictionInfo_t* dstCacheInfo;
	uint32_t evictionBlockNumber;
	cacheNode_t** caches;
	bool otherCacheContains = false;
	cache_t* dstCache = NULL;
	uint8_t counter = 0;
	caches = cacheSystem->caches;
	while (dstCache == NULL && counter < cacheSystem->size) { //Selects destination cache pointer from array of caches pointers
		if (caches[counter]->ID == ID) {
			dstCache = caches[counter]->cache;
		}
		counter++;
	}
	dstCacheInfo = findEviction(dstCache, address); //Finds block to evict and potential match
	evictionBlockNumber = dstCacheInfo->blockNumber;
	if (dstCacheInfo->match) {
		/*What do you do if it is in the cache?*/
		/*Your Code Here*/
		retVal = getData(dstCache, getOffset(dstCache, address), evictionBlockNumber, size);


	} else {
		uint32_t oldAddress = extractAddress(dstCache, extractTag(dstCache, evictionBlockNumber), evictionBlockNumber, 0);
		/*How do you need to update the snooper?*/
		/*How do you need to update states for what is getting evicted (don't worry about evicting this will be handled at a later step when you place data in the cache)?*/
		/*Your Code Here*/



		removeFromSnooper(cacheSystem->snooper, oldAddress, ID, cacheSystem->blockDataSize);

		int onlyCache = returnIDIf1(cacheSystem->snooper, oldAddress, cacheSystem->blockDataSize);
		if (onlyCache != -1) {
			updateState(getCacheFromID(cacheSystem, onlyCache), oldAddress, INVALID);
		}

		int val = returnFirstCacheID(cacheSystem->snooper, address, cacheSystem->blockDataSize);
		/*Check other caches???*/
		/*Your Code Here*/

		if (val != -1) {
			otherCacheContains = true;
			evict(dstCache, evictionBlockNumber);
			int blockAddress = address - (address % dstCache->blockDataSize);
			uint8_t* newBlock = readFromCache(getCacheFromID(cacheSystem, val), blockAddress, dstCache->blockDataSize);
			setTag(dstCache, getTag(dstCache, address), evictionBlockNumber);
			setData(dstCache, newBlock, evictionBlockNumber, dstCache->blockDataSize, 0);
			free(newBlock);
			retVal = getData(dstCache, getOffset(dstCache, address), evictionBlockNumber, size);
			updateLRU(dstCache, getTag(dstCache, address), getIndex(dstCache, address), dstCacheInfo->LRU);
		}


		if (!otherCacheContains) {
			/*Check Main memory?*/
			/*Your Code Here*/
			evict(dstCache, evictionBlockNumber);
			uint8_t* newBlock = malloc(sizeof(uint8_t) * dstCache->blockDataSize);
			int memAddress = address - (address % dstCache->blockDataSize);
			uint8_t* memData;
			for (int i = 0; i < dstCache->blockDataSize; i++) {
				memData = readFromMem(dstCache, memAddress + i);
				newBlock[i] = *memData;
				free(memData);
			}
			setState(dstCache, evictionBlockNumber, EXCLUSIVE);
			setTag(dstCache, getTag(dstCache, address), evictionBlockNumber);
			setData(dstCache, newBlock, evictionBlockNumber, dstCache->blockDataSize, 0);
			free(newBlock);
			retVal = getData(dstCache, getOffset(dstCache, address), evictionBlockNumber, size);
			updateLRU(dstCache, getTag(dstCache, address), getIndex(dstCache, address), dstCacheInfo->LRU);


		}

	}
	addToSnooper(cacheSystem->snooper, address, ID, cacheSystem->blockDataSize);
	if (otherCacheContains) {
		/*What states need to be updated?*/
		/*Your Code Here*/

		setState(dstCache, evictionBlockNumber, SHARED);

		for (int i = 0; i < cacheSystem->size; i++) {
			if (caches[i]->ID != ID) {
				updateState(caches[i]->cache, address, SHARED);
			}
		}

	}
	free(dstCacheInfo);
	return retVal;
}

/*
	A function used to request a byte from a specific cache in a cache system.
	Takes in a cache system, an address, and an ID for the cache which will be
	read from. Returns a struct with the data and a bool field indicating
	whether or not the read was a success.
*/
byteInfo_t cacheSystemByteRead(cacheSystem_t* cacheSystem, uint32_t address, uint8_t ID) {
	byteInfo_t retVal;
	uint8_t* data;
	/* Error Checking??*/
	if (cacheSystem == NULL || !validAddresses(address, 1) || (getCacheFromID(cacheSystem, ID) == NULL)) {
		retVal.success = false;
	}
	retVal.success = true;
	data = cacheSystemRead(cacheSystem, address, ID, 1);
	if (data == NULL) {
		return retVal;
	}
	retVal.data = data[0];
	free(data);
	return retVal;
}

/*
	A function used to request a halfword from a specific cache in a cache system.
	Takes in a cache system, an address, and an ID for the cache which will be
	read from. Returns a struct with the data and a bool field indicating
	whether or not the read was a success.
*/
halfWordInfo_t cacheSystemHalfWordRead(cacheSystem_t* cacheSystem, uint32_t address, uint8_t ID) {
	byteInfo_t temp;
	halfWordInfo_t retVal;
	uint8_t* data;
	/* Error Checking??*/
	if (cacheSystem == NULL || !validAddresses(address, 2) || (address % 2 != 0) || (getCacheFromID(cacheSystem, ID) == NULL)) {
		retVal.success = false;
	}
	retVal.success = true;
	if (cacheSystem->blockDataSize < 2) {
		temp = cacheSystemByteRead(cacheSystem, address, ID);
		retVal.data = temp.data;
		temp = cacheSystemByteRead(cacheSystem, address + 1, ID);
		retVal.data = (retVal.data << 8) | temp.data;
		return retVal;
	}
	data = cacheSystemRead(cacheSystem, address, ID, 2);
	if (data == NULL) {
		return retVal;
	}
	retVal.data = data[0];
	retVal.data = (retVal.data << 8) | data[1];
	free(data);
	return retVal;
}


/*
	A function used to request a word from a specific cache in a cache system.
	Takes in a cache system, an address, and an ID for the cache which will be
	read from. Returns a struct with the data and a bool field indicating
	whether or not the read was a success.
*/
wordInfo_t cacheSystemWordRead(cacheSystem_t* cacheSystem, uint32_t address, uint8_t ID) {
	halfWordInfo_t temp;
	wordInfo_t retVal;
	uint8_t* data;
	/* Error Checking??*/
	if (cacheSystem == NULL || !validAddresses(address, 4) || (address % 4 != 0) || (getCacheFromID(cacheSystem, ID) == NULL)) {
		retVal.success = false;
	}
	retVal.success = true;
	if (cacheSystem->blockDataSize < 4) {
		temp = cacheSystemHalfWordRead(cacheSystem, address, ID);
		retVal.data = temp.data;
		temp = cacheSystemHalfWordRead(cacheSystem, address + 2, ID);
		retVal.data = (retVal.data << 16) | temp.data;
		return retVal;
	}
	data = cacheSystemRead(cacheSystem, address, ID, 4);
	if (data == NULL) {
		return retVal;
	}
	retVal.data = data[0];
	retVal.data = (retVal.data << 8) | data[1];
	retVal.data = (retVal.data << 8) | data[2];
	retVal.data = (retVal.data << 8) | data[3];
	free(data);
	return retVal;
}

/*
	A function used to request a doubleword from a specific cache in a cache system.
	Takes in a cache system, an address, and an ID for the cache which will be
	read from. Returns a struct with the data and a bool field indicating
	whether or not the read was a success.
*/
doubleWordInfo_t cacheSystemDoubleWordRead(cacheSystem_t* cacheSystem, uint32_t address, uint8_t ID) {
	wordInfo_t temp;
	doubleWordInfo_t retVal;
	uint8_t* data;
	/* Error Checking??*/
	if (cacheSystem == NULL || !validAddresses(address, 8) || (address % 8 != 0) || (getCacheFromID(cacheSystem, ID) == NULL)) {
		retVal.success = false;
	}
	retVal.success = true;
	if (cacheSystem->blockDataSize < 8) {
		temp = cacheSystemWordRead(cacheSystem, address, ID);
		retVal.data = temp.data;
		temp = cacheSystemWordRead(cacheSystem, address + 4, ID);
		retVal.data = (retVal.data << 32) | temp.data;
		return retVal;
	}
	data = cacheSystemRead(cacheSystem, address, ID, 8);
	if (data == NULL) {
		return retVal;
	}
	retVal.data = data[0];
	retVal.data = (retVal.data << 8) | data[1];
	retVal.data = (retVal.data << 8) | data[2];
	retVal.data = (retVal.data << 8) | data[3];
	retVal.data = (retVal.data << 8) | data[4];
	retVal.data = (retVal.data << 8) | data[5];
	retVal.data = (retVal.data << 8) | data[6];
	retVal.data = (retVal.data << 8) | data[7];
	free(data);
	return retVal;
}