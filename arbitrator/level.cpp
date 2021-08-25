#include "stdafx.h"

void Level::Insert(std::vector<Access*>& records) {
	if (records.empty())
		return;

	auto iter = indexerByTx.find(records[0]->tx);
	if (iter == indexerByTx.end()) {
		indexerByTx[records[0]->tx] = records;
	}
	else {
		indexerByTx[records[0]->tx].insert(indexerByTx[records[0]->tx].end(), records.begin(), records.end());
	}
	
	indexer.insert(indexer.end(), records.begin(), records.end());
	//orderedAccesses.insert(record);
}

void Level::Clear() {
	indexer.clear();
	orderedAccesses.clear();
	indexerByTx.clear();
	conflicts.clear();
}

void Level::GetRanges(std::vector<Access*>& indexer, std::vector<std::size_t>& ranges) {
	ranges.push_back(0);

	std::size_t p0 = 0;
	for (std::size_t i = p0; i < indexer.size(); i++) {		
		if (!indexer[i]->PathEqual(indexer[p0])) {
			ranges.push_back(i);
			p0 = i;
		}
	}

	if (indexer.size() > 1 && ranges.size() == 1) {
		ranges.push_back(indexer.size());
	}

	if (ranges[ranges.size()-1] != indexer.size()) {
		ranges.push_back(indexer.size());
	}
}

// Find all the possible combinations from a conflict
void Level::GetCombinations(std::vector<Access*>& indexer, std::size_t start, std::size_t end, tbb::concurrent_vector<std::pair<Access*, Access*>>& conflictPairs) {
	for (std::size_t i = start; i < std::min<uint32_t>(end - start, 8192); i++)
		for (std::size_t j = i; j < end; j++) {
			if ((i == j) || 
				(indexer[i]->tx == indexer[j]->tx) ||
				(indexer[i]->IsCompositeOnly() && indexer[j]->IsCompositeOnly()) ||
				(indexer[i]->IsReadOnly() && indexer[j]->IsReadOnly()))
				continue;		

			if (*indexer[i] < *indexer[j])
				conflictPairs.push_back(std::make_pair(indexer[i], indexer[j]));
			else
				conflictPairs.push_back(std::make_pair(indexer[j], indexer[i]));
		}
}

bool Level::MarkConflicts(std::vector<Access*>& indexer, std::size_t start, std::size_t end,  uint32_t conflictGroup) {	
	if (start + 1 == end && !indexer[start]->addOrDelete)
		return false;
		
	bool conflictFound = false;
	if (indexer[start]->writes > 0) {
		for (std::size_t i = start + 1; i < end; i++) {
			if (indexer[start]->IsCompositeOnly() && indexer[i]->IsCompositeOnly())
				continue;

			if (indexer[i]->tx != indexer[start]->tx) {
				indexer[i]->isValid = false;
				conflictFound = true ;
			}
		}
	}

	if (indexer[start]->IsReadOnly()) {  // read only
		for (std::size_t i = start + 1; i < end; i++) {
			if (indexer[i]->writes > 0 && indexer[i]->tx != indexer[start]->tx) {
				indexer[i]->isValid = false;
				conflictFound = true;
			}
		}
	}

	if (conflictFound) {
		tbb::parallel_for_each(indexer.begin() + start, indexer.begin() + end, [&](auto access) { return access->group = arbitrator->groupID; });
		arbitrator->groupID++;
	}
	return conflictFound;
}

void Level::LabelByTransaction(std::vector<uint32_t>& conflictTxs, tbb::concurrent_unordered_map<uint32_t, std::vector<Access*>>& indexerByTx) {
	tbb::parallel_for(std::size_t(0), conflictTxs.size(), [&](std::size_t i) {
		for (std::size_t j = 0; j < indexerByTx[conflictTxs[i]].size(); j++) {
			indexerByTx[conflictTxs[i]][j]->isValid = false;
		}
	});
}

void Level::Process() {
	if (indexer.empty())
		return;

	std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
	tbb::parallel_sort(indexer.begin(), indexer.end(), [](Access* lhs, Access* rhs) { return *lhs < *rhs; });

	//std::vector<Access*> indexer(orderedAccesses.size());
	//std::copy(orderedAccesses.begin(), orderedAccesses.end(), indexer.begin());
	//std::cout << " parallel_sort():" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;
	msg += "Level: " + std::to_string(depth) + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count()) + " ms" + "/n";

	t0 = std::chrono::steady_clock::now();
	std::vector<std::size_t> ranges;
	GetRanges(indexer, ranges);
	//std::cout <<" GetRanges():" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;
	msg += "GetRanges():" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count()) + " ms" + "/n";

	t0 = std::chrono::steady_clock::now();
	tbb::parallel_for(std::size_t(1), ranges.size(), [&](std::size_t i) {
	//for (std::size_t i = 1; i < ranges.size(); i++) {
		if(MarkConflicts(indexer, ranges[i - 1], ranges[i], i - 1))
			GetCombinations(indexer, ranges[i - 1], ranges[i], arbitrator->conflictPairs);
	});
	msg += "MarkConflicts() + GetCombinations():" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count()) + " ms" + "/n";
	
	t0 = std::chrono::steady_clock::now();
	tbb::parallel_for(std::size_t(1), indexer.size(), [&](std::size_t i) {
	//	for (std::size_t i = 0; i < indexer.size(); i++) {
			if (!indexer[i]->isValid)
				conflicts.push_back(indexer[i]);
	});	

	// Get the conflict txs
	std::vector<uint32_t> txs(conflicts.size());
	tbb::parallel_for(std::size_t(0), conflicts.size(), [&](std::size_t i) {
	//for (std::size_t i = 0; i < conflicts.size(); i++) {
		txs[i] = conflicts[i]->tx;
	});
	
	t0 = std::chrono::steady_clock::now();
	LabelByTransaction(txs, indexerByTx);
	msg += " LabelByTransaction():" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count()) + " ms" + "/n";
}