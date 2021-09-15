#include "stdafx.h"

void AccessInfo::Add(Access* record) {
	std::scoped_lock<std::mutex>lock{ mutex };
	totalRead += record->reads;
	totalCompositeWrites += record->compositeWrites;
	totalWrites += record->writes;

	if (!this->records.empty() && (*records.begin())->branch != record->branch) {
		if (Predetermine(record)) {
			conflicts.push_back(record);
		}		
	}
	records.insert(record);
}

bool AccessInfo::Predetermine(Access* record) {
	if (!conflicts.empty()) // Conflict found already
		return false;

	if (record->writes == 0 && totalWrites == 0 && totalCompositeWrites == 0)  // Read only
		return false;

	if (record->reads == 0 && record->compositeWrites > 0 && totalRead == 0 && totalWrites == 0 && totalCompositeWrites > 0) // Composite write only
		return false;
	
	return true;
}

void AccessInfo::Detect(tbb::concurrent_unordered_set<uint32_t>& whitelist) {
	if (records.size() <= 1)
		return;

	conflicts.resize(0);
	nonconflicts.resize(0);
	auto begin = records.begin();
	if (!whitelist.empty()) {
		for (; begin != records.end(); begin++) {
			if (whitelist.find((*begin)->tx) != whitelist.end()) {
				break; // Get the first entry on the whitelist
			}
		}
		if (begin == records.end())
			return;
	} 

	nonconflicts.push_back(*begin);
	auto iter = begin;
	iter++;
	if ((*begin)->writes > 0 || (*begin)->compositeWrites > 0) {
		for (; iter != records.end(); iter++) {
			if ((*iter)->branch == (*begin)->branch) {// In the same branch
				nonconflicts.push_back(*iter);			
				continue;
			}

			if (!whitelist.empty() && (whitelist.find((*iter)->tx) == whitelist.end())) {// The whitelist exists and the entry isn't on it
				nonconflicts.push_back(*iter);
				continue;
			}

			if ((*begin)->IsCompositeOnly() && (*iter)->IsCompositeOnly()) {// Composite writes
				nonconflicts.push_back(*iter);
				continue;
			}

			if ((*iter)->branch != (*begin)->branch)
				conflicts.push_back(*iter);
			else
				nonconflicts.push_back(*iter);
		}
	}
	else {
		for (; iter != records.end(); iter++) {
			if (!whitelist.empty() && (whitelist.find((*iter)->tx) == whitelist.end())) {
				nonconflicts.push_back(*iter);
				continue;
			}

			if (((*iter)->writes > 0 || (*iter)->compositeWrites > 0) && (*iter)->tx != (*records.begin())->tx)
				conflicts.push_back(*iter); 
			else 
				nonconflicts.push_back(*iter);
		}
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
			if ((conflicts[i]->branch == conflicts[j]->branch) ||
				(conflicts[i]->IsCompositeOnly() && conflicts[j]->IsCompositeOnly()) ||
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