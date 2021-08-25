#include "stdafx.h"

void Urlarbitrator::GetRanges(std::vector<Access*>& indexer, std::vector<std::size_t>& ranges) {
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

	if (ranges[ranges.size() - 1] != indexer.size()) {
		ranges.push_back(indexer.size());
	}
}

// Find all the possible combinations from a conflict
void Urlarbitrator::GetCombinations(std::vector<Access*>& indexer, std::size_t start, std::size_t end, tbb::concurrent_vector<std::pair<Access*, Access*>>& conflictPairs) {
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

bool Urlarbitrator::MarkConflicts(std::vector<Access*>& indexer, std::size_t start, std::size_t end, uint32_t conflictGroup) {
	if (start + 1 == end && !indexer[start]->addOrDelete)
		return false;

	bool conflictFound = false;
	if (indexer[start]->writes > 0) {
		for (std::size_t i = start + 1; i < end; i++) {
			if (indexer[start]->IsCompositeOnly() && indexer[i]->IsCompositeOnly())
				continue;

			if (indexer[i]->tx != indexer[start]->tx) {
				indexer[i]->isValid = false;
				conflictFound = true;
			}
		}
	}
	else {
		for (std::size_t i = start + 1; i < end; i++) {
			if (indexer[i]->writes > 0 && indexer[i]->tx != indexer[start]->tx) {
				indexer[i]->isValid = false;
				conflictFound = true;
			}
		}
	}

	if (conflictFound) {
		tbb::parallel_for_each(indexer.begin() + start, indexer.begin() + end, [&](auto access) { return access->group = this->groupID; });
		this->groupID++;
	}
	return conflictFound;
}

void Urlarbitrator::Detect() {
	if (indexer.empty())
		return;
	
	tbb::parallel_sort(indexer.begin(), indexer.end(), [](Access* lhs, Access* rhs) { return *lhs < *rhs; });
	
	std::vector<std::size_t> ranges;
	GetRanges(indexer, ranges);

	tbb::parallel_for(std::size_t(1), ranges.size(), [&](std::size_t i) {
		//for (std::size_t i = 1; i < ranges.size(); i++) {
		if (MarkConflicts(indexer, ranges[i - 1], ranges[i], i - 1)) {
			GetCombinations(indexer, ranges[i - 1], ranges[i], this->conflictPairs);

			for (std::size_t j = ranges[i - 1]; j < ranges[i]; j ++) {
				if (!indexer[j]->isValid)
					conflicts.push_back(indexer[j]->tx);
			}
		}
	});
	Reorganize();
}

void Urlarbitrator::Clear() {
	for (std::size_t i = 0; i < memory.size(); i++)
		delete[] memory[i];
	memory.clear();

	indexer.clear();	
	conflicts.clear();
	conflictPairs.clear();
	groupID = 0;
	msg.clear();
}

void Urlarbitrator::Reorganize() {
	tbb::parallel_sort(conflictPairs.begin(), conflictPairs.end(), [](auto lft, auto rgt) {
		if (lft.first->PathEqual(rgt.first)) // If left ones are equal, compare the right
			return lft.second->PathLessThan(rgt.second);
		return lft.first->PathLessThan(rgt.first);
	});

	auto last = std::unique(conflictPairs.begin(), conflictPairs.end(), [](auto lft, auto rgt) {
		return lft.first->tx == rgt.first->tx && lft.second->tx == rgt.second->tx;
	});

	conflictPairs.resize(std::distance(conflictPairs.begin(), last)); // Remove duplicates
}

// Export conflicts grouped by tx id
void Urlarbitrator::ExportTxs(std::vector<uint32_t>& lftTxs, std::vector<uint32_t>& rgtTxs) {
	lftTxs.resize(conflictPairs.size());
	rgtTxs.resize(conflictPairs.size());
	for (std::size_t i = 0; i < conflictPairs.size(); i++) {
		lftTxs[i] = conflictPairs[i].first->tx;
		rgtTxs[i] = conflictPairs[i].second->tx;
	}
}