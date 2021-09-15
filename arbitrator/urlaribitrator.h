#pragma once
struct Urlarbitrator {
	const static uint32_t MAX_ENTRIES = 65536;

	tbb::concurrent_vector<AccessInfo*> conflicts;
	tbb::concurrent_unordered_map<std::string, AccessInfo*> accessInfo;
	std::string msg;

	Urlarbitrator() {}
	~Urlarbitrator() { Clear();}

	void Insert(std::string& key, Access* record);
	void Detect(tbb::concurrent_vector<Access*>& buffer, tbb::concurrent_unordered_set<uint32_t>& dict);
	void Clear();

	void ExportPaths(std::vector<std::string*>& paths, std::vector<uint32_t>& txs, std::vector<uint32_t>& idLengths) {} // Export conflicts grouped by path, a conflict path cause many transitions to fail
};

