#include "stdafx.h"

void* Start() {
	return new Urlarbitrator();
}

void Insert(void* UrlarbitratorPtr, char* txs, char* branchIDs, char* path, char* reads, char* writes, char* addOrDelete, char* composite, char* pathLen, uint32_t count) {
	std::vector<uint32_t> positions(count + 1, 0);
	for (std::size_t i = 0; i < count; i++) {
		positions[i + 1] = positions[i] + ((uint32_t*)(pathLen))[i];
	}
		
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));
	arbi->memory.push_back(new char[sizeof(Access) * count]);

	char* mem = arbi->memory.back();
	std::vector<Access*> records(count);
	tbb::parallel_for(std::size_t(0), std::size_t(count), [&](std::size_t i) {
		records[i] = new(mem + sizeof(Access) * i)
			Access(((uint32_t*)(txs))[i], UINT32_MAX, path + positions[i], ((uint32_t*)(pathLen))[i], ((uint32_t*)(reads))[i], ((uint32_t*)(writes))[i], ((bool*)(addOrDelete))[i], ((bool*)composite)[i]);
	});
	((Urlarbitrator*)(UrlarbitratorPtr))->Insert(records);
}

void Detect(void* UrlarbitratorPtr, char* txs, char* group, char* conflictFlags, char* count, char*  msgBuffer) {
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));
	arbi->Detect();
	
	tbb::concurrent_vector<Access*> groupedConflicts;
	tbb::parallel_for(std::size_t(0), std::size_t(arbi->indexer.size()), [&](std::size_t i) {
		if (arbi->indexer[i]->group != UINT32_MAX) {
			groupedConflicts.push_back(arbi->indexer[i]);
		}
	});

	tbb::parallel_sort(groupedConflicts.begin(), groupedConflicts.end(), [](Access* lft, Access* rgt) { return lft->tx < rgt->tx; });
	
	tbb::parallel_for(std::size_t(0), std::size_t(groupedConflicts.size()), [&](std::size_t i) {
	//for (std::size_t i = 0; i < txIDs.size(); i++) {
		((uint32_t*)(txs))[i] = groupedConflicts[i]->tx;
		((uint32_t*)(group))[i] = groupedConflicts[i]->group;

		if (groupedConflicts[i]->isValid)
			((uint8_t*)(conflictFlags))[i] = 0;
		else
			((uint8_t*)(conflictFlags))[i] = 255;
	});

	*((uint32_t*)count) = arbi->indexer.size();
	//std::strncpy(msgBuffer, arbi->msg.c_str(), std::min<uint32_t>(arbi->msg.size(), 4096));
}

/* ------------------------------*Exports transactions causing conflicts ------------------------------*/
uint64_t GetConflictTxTotal(void* UrlarbitratorPtr) {
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));
	return arbi->conflictPairs.size();
}

void ExportTxs(void* UrlarbitratorPtr, char* lftBuffer, char* rgtBuffer, char* count) {
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));

	std::vector<uint32_t> lftTxs;
	std::vector<uint32_t> rgtTxs;
	arbi->ExportTxs(lftTxs, rgtTxs);

	*((uint32_t*)count) = lftTxs.size();
	for (std::size_t i = 0; i < lftTxs.size(); i++) {
		((uint32_t*)lftBuffer)[i] = lftTxs[i];
		((uint32_t*)rgtBuffer)[i] = rgtTxs[i];
	}
}

/* -----------------------------------------------------------------------------------------------------*/

uint64_t GetConflictPathsTotal(void* UrlarbitratorPtr) {
	return 0;
}

void ExportPaths(void* UrlarbitratorPtr, char* paths, char* pathLengths, char* txIDs, char* idLength, char* counts) {
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));

	std::vector<std::string*> conflictPaths;
	std::vector<uint32_t> txs;
	std::vector<uint32_t> idLengths;
	arbi->ExportPaths(conflictPaths, txs, idLengths);
		
	uint32_t offset = 0;
	for (std::size_t i = 0; i < conflictPaths.size(); i++) {
		std::memcpy(paths + offset, conflictPaths[i]->c_str(), conflictPaths[i]->size());
		((uint32_t*)pathLengths)[i] = conflictPaths[i]->size();
		offset += conflictPaths[i]->size();
	}

	for (std::size_t i = 1; i < idLengths.size(); i++) {
		((uint32_t*)idLength)[i - 1] = idLengths[i] - idLengths[i - 1];
	}
	*((uint32_t*)counts) = idLengths.size() - 1;
}

/* -----------------------------------------------------------------------------------------------------*/

bool IsSparse(void* UrlarbitratorPtr) {
	return true;
}

void Clear(void* UrlarbitratorPtr) {
	((Urlarbitrator*)(UrlarbitratorPtr))->Clear();
}

void Stop(void* UrlarbitratorPtr) {
	delete (Urlarbitrator*)(UrlarbitratorPtr);
}

void GetVersion(char* ver) {
	std::strcpy(ver, version);
}

void GetProduct(char* productInfo) {
	std::strcpy(productInfo, product);
}