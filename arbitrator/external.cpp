#include "stdafx.h"
void* UrlarbitratorStart() {
	return new Urlarbitrator();
}

void UrlarbitratorInsert(void* urlarbitratorPtr, char* txs, char* branchIDs, char* path, char* reads, char* writes, char* composite, char* pathLen, uint32_t count) {
	Urlarbitrator* arbi = (Urlarbitrator*)(urlarbitratorPtr);

	std::vector<uint32_t> positions(count + 1, 0);
	for (std::size_t i = 0; i < count; i++) {
		positions[i + 1] = positions[i] + ((uint32_t*)(pathLen))[i];
	}

	tbb::parallel_for(std::size_t(0), std::size_t(count), [&](std::size_t i) {
	//for (std::size_t i = 0; i < count; i++) {
		std::string key(path + positions[i], ((uint32_t*)(pathLen))[i]);
		Access* record = new Access(((uint32_t*)(txs))[i], ((uint32_t*)(txs))[i], nullptr, ((uint32_t*)(reads))[i], ((uint32_t*)(writes))[i], ((bool*)composite)[i]);
		arbi->Insert(key, record);
	});
}

void UrlarbitratorDetect(void* UrlarbitratorPtr, char* whitelist, uint32_t listCount, char* outTxBuf, char* outCount, char*  msgBuffer) {
	tbb::concurrent_unordered_set<uint32_t> dict;
	tbb::parallel_for(std::size_t(0), std::size_t(listCount), [&](std::size_t i) {
		dict.insert(((uint32_t*)(whitelist))[i]);
	});

	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));

	tbb::concurrent_vector<Access*> buffer;
	arbi->Detect(buffer, dict);
	
	tbb::parallel_sort(buffer.begin(), buffer.end(), [](Access* lhs, Access* rhs) { return lhs->tx < rhs->tx; });
	auto last = std::unique(buffer.begin(), buffer.end(), [](Access* lhs, Access* rhs) { return lhs->tx == rhs->tx; });
	buffer.resize(std::distance(buffer.begin(), last));

	for (std::size_t i = 0; i < buffer.size(); i++) {
		((uint32_t*)(outTxBuf))[i] = buffer[i]->tx;
	}
	*((uint32_t*)outCount) = buffer.size();
}

void UrlarbitratorExportConflictPairs(void* UrlarbitratorPtr, char* lhsBuf, char* rhsBuf, char* count) {
	Urlarbitrator* arbi = ((Urlarbitrator*)(UrlarbitratorPtr));	

	std::vector<std::pair<Access*, Access*>> pairs;
	pairs.reserve(Urlarbitrator::MAX_ENTRIES); // Max entries
	for (std::size_t i = 0; i < arbi->conflicts.size(); i++) {
		arbi->conflicts[i]->ExportCombinations(pairs, Urlarbitrator::MAX_ENTRIES);
	}

	for (std::size_t i = 0; i < pairs.size(); i++) {
		((uint32_t*)lhsBuf)[i] = pairs[i].first->tx;
		((uint32_t*)rhsBuf)[i] = pairs[i].second->tx;
	}
	*((uint32_t*)count) = pairs.size();
}

void UrlarbitratorExportConflictPaths(void* UrlarbitratorPtr, char* paths, char* pathLengths, char* txIDs, char* idLength, char* counts) {
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

void UrlarbitratorClear(void* UrlarbitratorPtr) {
	((Urlarbitrator*)(UrlarbitratorPtr))->Clear();
}

void UrlarbitratorStop(void* UrlarbitratorPtr) {
	delete (Urlarbitrator*)(UrlarbitratorPtr);
}

void UrlarbitratorGetVersion(char* ver) {
	std::strcpy(ver, version);
}

void UrlarbitratorGetProduct(char* productInfo) {
	std::strcpy(productInfo, product);
}