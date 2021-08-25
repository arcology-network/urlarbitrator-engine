#pragma once
//#pragma once

///*Adding headers for cgo*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	void* Start();
	void Clear(void* arbitratorPtr);
	void Stop(void* aribitratorPtr); 

	void Insert(void* arbitratorPtr, char* txs, char* branchIDs, char* paths, char* reads, char* writes, char* addOrDelete, char* composite, char* pathLen, uint32_t count);
	void Detect(void* arbitratorPtr, char* txs, char* groupIDs, char* conflictFlags, char* count, char* msg);
		
	uint64_t GetConflictTxTotal(void* arbitratorPtr);
	void ExportTxs(void* arbitratorPtr, char* lftBuffer, char* rgtBuffer, char* count);  // Export the conflict txs pairs

	uint64_t GetConflictPathsTotal(void* arbitratorPtr);
	void ExportPaths(void* arbitratorPtr, char* paths, char* pathLengths, char* txIDs, char* txCounts, char* count);  // Export the conflict paths and txs

	bool IsSparse(void* arbitratorPtr); //

	void GetVersion(char* version); // Get version 
	void GetProduct(char* product); // Get product name

#ifdef __cplusplus
}
#endif
