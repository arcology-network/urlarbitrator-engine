#include "stdafx.h"

void Urlarbitrator::Insert(std::string& key, Access* record) {
	if (accessInfo.find(key) == accessInfo.end()) {
		accessInfo.insert(std::make_pair(key, new AccessInfo(key)));
	}

	AccessInfo* info = accessInfo[key];
	record->path = &(info->path);
	info->Add(record);
}

void Urlarbitrator::Detect(tbb::concurrent_vector<Access*>& buffer, tbb::concurrent_unordered_set<uint32_t>& dict) {
	tbb::concurrent_vector<const std::string*> keys;
	keys.reserve(accessInfo.size());
	for (auto iter = accessInfo.begin(); iter != accessInfo.end(); iter++) {
		if (!iter->second->conflicts.empty()) {
			keys.push_back(&(iter->first));
		}
	}

	conflicts.clear();
	tbb::parallel_for(std::size_t(0), keys.size(), [&](std::size_t i) {
	//for (std::size_t i = 0; i < keys.size(); i++) {
		auto info = accessInfo[*keys[i]];
		if (!info->conflicts.empty()) {
			conflicts.push_back(info);
			info->Detect(dict);

			for (std::size_t j = 0; j < info->conflicts.size(); j++)
				buffer.push_back(info->conflicts[j]);
		}
	});
}

void Urlarbitrator::Clear() {
	std::for_each(accessInfo.begin(), accessInfo.end(), [](auto iter) { delete iter.second; });
	msg.clear();
	accessInfo.clear();
}

