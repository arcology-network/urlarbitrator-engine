#include "stdafx.h"

AccessInfo* Urlarbitrator::Insert(std::string& key, Access* record) {
	AccessInfo* info = byPath[key];
	record->path = &(info->path);
    info->Add(record);
	return info;
}

void Urlarbitrator::Detect(tbb::concurrent_vector<Access*>& buffer) {
	tbb::concurrent_vector<const std::string*> keys;
	keys.reserve(byPath.size());
	for (auto iter = byPath.begin(); iter != byPath.end(); iter++) {
		if (!iter->second->ordered.empty()) {
			keys.push_back(&(iter->first));
		}
	}

	conflicts.clear();
	tbb::parallel_for(std::size_t(0), keys.size(), [&](std::size_t i) {
	//for (std::size_t i = 0; i < keys.size(); i++) {
		auto info = byPath[*keys[i]];
		if (!info->ordered.empty()) {
			conflicts.push_back(info);
			info->Detect();

			for (std::size_t j = 0; j < info->conflicts.size(); j++)
				buffer.push_back(info->conflicts[j]);
		}
	});
}

void Urlarbitrator::Clear() {
	tbb::parallel_for_each(byPath.begin(), byPath.end(), [](auto iter) { delete iter.second; });
	tbb::parallel_for_each(byTx.begin(), byTx.end(), [](auto iter) {
		iter.second.clear(); 
	});

	for (std::size_t i = 0; i < pool.size(); i++) {
		delete[] pool[i];
	}
	pool.clear();

	tbb::parallel_invoke([&]() {
		msg.clear();
		conflicts.clear();
		byTx.clear();
	},
	[&]() {
		byPath.clear();
	});
}