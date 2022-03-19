#include "stdafx.h"

void AccessInfo::Add(Access* record) {
	std::scoped_lock<std::mutex>lock{ mutex };
	totalRead += record->reads;
	totalCompositeWrites += record->compositeWrites;
	totalWrites += record->writes;

	if (!ordered.empty() || Predetermine(record)) {
		ordered.insert(record);
		ordered.insert(nonconflicts.begin(), nonconflicts.end());
		nonconflicts.clear();
		return;
	}
	nonconflicts.push_back(record);
}

bool AccessInfo::Predetermine(Access* record) {
	if (record->IsReadOnly() && totalWrites == 0 && totalCompositeWrites == 0)  // Read only
		return false;

	if (record->IsCompositeOnly() && totalRead == 0 && totalWrites == 0) // Composite write only
		return false;

	if (!ordered.empty() && (*ordered.begin())->tx == record->tx) {
		return false;
	}
	return true;
}

void AccessInfo::Detect() {
	if (ordered.size() <= 1)
		return;

	conflicts.clear();
	auto begin = ordered.begin();
	for (; begin != ordered.end(); begin++) {
		if ((*begin)->group != UINT32_MAX) {
			nonconflicts.push_back(*begin);
			break; // Get the first entry to start
		}
	}

	if (begin == ordered.end()) {
		return;
	}
	
	auto iter = begin;
	iter++;

	for (; iter != ordered.end(); iter++) {
		if ((*iter)->group == UINT32_MAX) {
			continue;
		}

		if ((*iter)->group == (*begin)->group) {// In the same group
			nonconflicts.push_back(*iter);			
			continue;
		}

		if ((*begin)->IsReadOnly() && (*iter)->IsReadOnly()) { // read only
			nonconflicts.push_back(*iter);
			continue;
		}
			
		if ((*begin)->IsCompositeOnly() && (*iter)->IsCompositeOnly()) {// Composite writes only
			nonconflicts.push_back(*iter);
			continue;
		}
		conflicts.push_back(*iter);		
	}
}

// Find all the possible combinations from a list of conflicts
void AccessInfo::ExportCombinations(std::vector<std::pair<Access*, Access*>>& buffer, uint32_t maxSize) {
	std::size_t counter = 0;
	for (std::size_t i = 0; i < nonconflicts.size(); i++) {
		for (std::size_t j = 0; j < conflicts.size(); j++) {
			if (*nonconflicts[i] < *conflicts[j])
				buffer.push_back(std::make_pair(nonconflicts[i], conflicts[j]));
			else
				buffer.push_back(std::make_pair(conflicts[j], nonconflicts[i]));

			if (buffer.size() >= maxSize)
				return;
		}
	}

	for (std::size_t i = 0; i < conflicts.size(); i++) {
		for (std::size_t j = i + 1; j < conflicts.size(); j++) {
			if ((conflicts[i]->IsCompositeOnly() && conflicts[j]->IsCompositeOnly()) ||
				(conflicts[i]->IsReadOnly() && conflicts[j]->IsReadOnly()))
				continue;

			if (*conflicts[i] < *conflicts[j])
				buffer.push_back(std::make_pair(conflicts[i], conflicts[j]));
			else
				buffer.push_back(std::make_pair(conflicts[j], conflicts[i]));

			if (buffer.size() >= maxSize)
				return;
		}
	}
}