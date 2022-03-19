#include "stdafx.h"
void* UrlarbitratorStart() {
	return new Urlarbitrator();
}

void UrlarbitratorInsert(void* urlarbitratorPtr, char* txs, char* path, char* pathLens, char* reads, char* writes, char* composite, uint32_t count) {
	Urlarbitrator* arbi = (Urlarbitrator*)urlarbitratorPtr;
	auto t0 = std::chrono::steady_clock::now();
	std::vector<uint32_t> positions(count + 1, 0);
	for (std::size_t i = 0; i < count; i++)
		positions[i + 1] = positions[i] + ((uint32_t*)(pathLens))[i];

	std::vector<Access*> records(count);
	std::vector<std::string> keys(count);

	tbb::concurrent_unordered_set<std::string> newKeys;
	tbb::concurrent_unordered_set <uint32_t> txIDs;
	
	char* mem = new char[sizeof(Access) * count];
	tbb::parallel_for(std::size_t(0), std::size_t(count), [&](std::size_t i) {
		keys[i] = std::string(path + positions[i], ((uint32_t*)(pathLens))[i]);
		if (arbi->byPath.find(keys[i]) == arbi->byPath.end())
			newKeys.insert(keys[i]);

		records[i] = new(mem + i * sizeof(Access)) Access(((uint32_t*)(txs))[i], nullptr, ((uint32_t*)(reads))[i], ((uint32_t*)(writes))[i], ((bool*)composite)[i]);
		if (arbi->byTx.find(records[i]->tx) == arbi->byTx.end())
			txIDs.insert(records[i]->tx);
	});	
	arbi->pool.push_back(mem);
	
	tbb::parallel_for_each(txIDs.begin(), txIDs.end(), [&](auto iter) {
		arbi->byTx.insert(std::make_pair(iter, tbb::concurrent_vector<Access*>()));
	});

	tbb::parallel_for_each(newKeys.begin(), newKeys.end(), [&](auto iter) {
		arbi->byPath.insert(std::make_pair(iter, new AccessInfo(iter)));
	});

	tbb::parallel_for(std::size_t(0), records.size(), [&](std::size_t i) {
		//for (std::size_t i = 0; i < records.size(); i++) {
			arbi->byTx[records[i]->tx].push_back(records[i]);
			arbi->Insert(keys[i], records[i]);
	});
	
	std::cout << "New Urlarbitrator.insert: "<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms"  <<  std::endl;
}

void UrlarbitratorDetect(void* UrlarbitratorPtr, char* groupIDs, char* groupIDLens, uint32_t groupIDCount, char* outTxBuf, char* outCount, char*  msgBuffer) {
	std::vector<uint32_t> positions(groupIDCount + 1, 0);
	for (std::size_t i = 0; i < groupIDCount; i++)
		positions[i + 1] = positions[i] + ((uint32_t*)(groupIDLens))[i];
	
	auto t0 = std::chrono::steady_clock::now();
	Urlarbitrator* arbi = (Urlarbitrator*)UrlarbitratorPtr;

	tbb::parallel_for(std::size_t(0), std::size_t(groupIDCount), [&](std::size_t i) {
		for (std::size_t j = positions[i]; j < positions[i + 1]; j++) { // # Unique IDs
			uint32_t tx = ((uint32_t*)(groupIDs))[j];
			tbb::concurrent_vector<Access*> *vec = &(arbi->byTx[tx]);
			for (std::size_t k = 0; k < vec->size(); k++)
				vec->at(k)->group = i;
		}
	});
	std::cout << "Set Groups ID: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	//t0 = std::chrono::steady_clock::now();
	tbb::concurrent_vector<Access*> buffer; // Conflict buffer;
    arbi->Detect(buffer);
	//std::cout << "Detect: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	//t0 = std::chrono::steady_clock::now();
	tbb::parallel_sort(buffer.begin(), buffer.end(), [](Access* lhs, Access* rhs) { return lhs->tx < rhs->tx; });
	auto last = std::unique(buffer.begin(), buffer.end(), [](Access* lhs, Access* rhs) { return lhs->tx == rhs->tx; });
	buffer.resize(std::distance(buffer.begin(), last));

	for (std::size_t i = 0; i < buffer.size(); i++) {
		((uint32_t*)(outTxBuf))[i] = buffer[i]->tx;
	}
	*((uint32_t*)outCount) = buffer.size();

	// Reset the groups IDs
	t0 = std::chrono::steady_clock::now();
	tbb::parallel_for(std::size_t(0), std::size_t(groupIDCount), [&](std::size_t i) {
		for (std::size_t j = positions[i]; j < positions[i + 1]; j++) { // # Unique IDs
			uint32_t tx = ((uint32_t*)(groupIDs))[j];
			tbb::concurrent_vector<Access*> *vec = &(arbi->byTx[tx]);
			for (std::size_t k = 0; k < vec->size(); k++)
				vec->at(k)->group = UINT32_MAX;
		}
	});
	std::cout << "Reset the groups IDs: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;
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