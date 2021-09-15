#pragma once
//#pragma once

///*Adding headers for cgo*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	void* UrlarbitratorStart();
	void UrlarbitratorClear(void* arbitratorPtr);
	void UrlarbitratorStop(void* aribitratorPtr);

	void UrlarbitratorInsert(void* arbitratorPtr, char* txs, char* branchIDs, char* paths, char* reads, char* writes, char* composite, char* pathLen, uint32_t count);
	void UrlarbitratorDetect(void* arbitratorPtr, char* whitelist, uint32_t listLen, char* txBuf, char* count, char* msg);

	void UrlarbitratorExportConflictPairs(void* arbitratorPtr, char* lftBuffer, char* rgtBuffer, char* count);  // Export the conflict txs pairs
	void UrlarbitratorExportConflictPaths(void* arbitratorPtr, char* paths, char* pathLengths, char* txIDs, char* txCounts, char* count);  // Export the conflict paths and txs

	void UrlarbitratorGetVersion(char* version); // Get version 
	void UrlarbitratorGetProduct(char* product); // Get product name

#ifdef __cplusplus
}
#endif
