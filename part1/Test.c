#include <stdint.h>
#include <stdio.h>
#include "../part1/utils.h"
#include "../part1/getFromCache.h"
#include "../part1/setInCache.h"
#include "../part1/mem.h"
#include "../part1/cacheRead.h"
#include "../part1/cacheWrite.h"


int main(void){
    uint8_t n;
    uint32_t blockDataSize;
    uint32_t totalDataSize;
    char* memFile;
    cache_t* cache;
    memFile = "testFiles/50AddressTest.txt";


    //Direct Mapped Cache Test
    n = 1;
    blockDataSize = 16;
    totalDataSize = 128;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache) != 1248){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 156){
        printf("s\n", "cacheSizeBytes");
    }
    if(numGarbageBits(cache)!= 0){
        printf("s\n", "numGarbageBits");
    }

    if(totalBlockBits(cache)!= 156){
        printf("s\n", "totalBlockBits");
    }
    deleteCache(cache);

    //Fully Associative Cache Test
    n = 16;
    blockDataSize = 32;
    totalDataSize = 512;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache)!= 4640){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 580){
        printf("s\n", "cacheSizeBits");
    }
    if(numGarbageBits(cache)!= 0){
        printf("s\n", "cacheSizeBits");
    }
    if(totalBlockBits(cache)!= 290){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);


    //Four Way Set Associtive Cache Test
    n = 4;
    blockDataSize = 32;
    totalDataSize = 256;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache)!= 2296){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 287){
        printf("s\n", "cacheSizeBits");
    }
    if(numGarbageBits(cache)!= 0){
        printf("s\n", "cacheSizeBits");
    }
    if(totalBlockBits(cache)!= 287){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);

    //1 Block Test
    n = 1;
    blockDataSize = 2147483648;
    totalDataSize = 2147483648;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache)!= 17179869188){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 2147483649){
        printf("s\n", "cacheSizeBits");
    }
    if(numGarbageBits(cache)!= 4){
        printf("s\n", "cacheSizeBits");
    }
    if(totalBlockBits(cache)!= 17179869188){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);

    //All Blocks Test
    n = 1;
    blockDataSize = 1;
    totalDataSize = 65536;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache)!= 1769472){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 221184){
        printf("s\n", "cacheSizeBits");
    }
    if(numGarbageBits(cache)!= 0){
        printf("s\n", "cacheSizeBits");
    }
    if(totalBlockBits(cache)!= 27){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);

    //Smallest Cache Test
    n = 1;
    blockDataSize = 1;
    totalDataSize = 1;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(cacheSizeBits(cache)!= 43){
        printf("s\n", "cacheSizeBits");
    }
    if(cacheSizeBytes(cache)!= 6){
        printf("s\n", "cacheSizeBits");
    }
    if(numGarbageBits(cache)!= 5){
        printf("s\n", "cacheSizeBits");
    }
    if(totalBlockBits(cache)!= 43){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);



    //test_Locations



    memFile = "testFiles/50AddressTest.txt";

    n = 1;
    blockDataSize = 16;
    totalDataSize = 128;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(getValidLocation(cache, 2)!= 312){
        printf("s\n", "cacheSizeBits");
    }
    if(getDirtyLocation(cache, 1)!= 157){
        printf("s\n", "cacheSizeBits");
    }
    if(getSharedLocation(cache, 3)!= 470){
        printf("s\n", "cacheSizeBits");
    }
    if(getLRULocation(cache, 0)!= 3){
        printf("s\n", "cacheSizeBits");
    }
    if(getTagLocation(cache, 0)!= 3){
        printf("s\n", "cacheSizeBits");
    }
    if(getDataLocation(cache, 2, 9)!= 412){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);


    n = 16;
    blockDataSize = 32;
    totalDataSize = 512;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(getValidLocation(cache, 6)!= 1740){
        printf("s\n", "cacheSizeBits");
    }
    if(getDirtyLocation(cache, 11)!= 3191){
        printf("s\n", "cacheSizeBits");
    }
    if(getSharedLocation(cache, 9)!= 2612){
        printf("s\n", "cacheSizeBits");
    }
    if(getLRULocation(cache, 14)!= 4063){
        printf("s\n", "cacheSizeBits");
    }
    if(getTagLocation(cache, 1)!= 297){
        printf("s\n", "cacheSizeBits");
    }
    if(getDataLocation(cache, 15, 30)!= 4624){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);

    n = 1;
    blockDataSize = 1;
    totalDataSize = 1;
    cache = createCache(n, blockDataSize, totalDataSize, memFile);
    if(cache == NULL) {
        printf("s\n", "The cache is NULL");
    }
    if(getValidLocation(cache, 0)!= 5){
        printf("s\n", "cacheSizeBits");
    }
    if(getDirtyLocation(cache, 0)!= 6){
        printf("s\n", "cacheSizeBits");
    }
    if(getSharedLocation(cache, 0)!= 7){
        printf("s\n", "cacheSizeBits");
    }
    if(getLRULocation(cache, 0)!= 8){
        printf("s\n", "cacheSizeBits");
    }
    if(getTagLocation(cache, 0)!= 8){
        printf("s\n", "cacheSizeBits");
    }
    if(getDataLocation(cache, 0, 0)!= 40){
        printf("s\n", "cacheSizeBits");
    }
    deleteCache(cache);
}
