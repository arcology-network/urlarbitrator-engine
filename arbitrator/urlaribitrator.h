#pragma once
struct Urlarbitrator {
	const static uint32_t MAX_ENTRIES = 65536;
	std::mutex mutex;
	tbb::concurrent_vector<AccessInfo*> conflicts;
	tbb::concurrent_unordered_map<std::string, AccessInfo*> byPath;
	tbb::concurrent_unordered_map<uint32_t, tbb::concurrent_vector<Access*>> byTx;
	std::string msg;

	std::vector<char*> pool;

	Urlarbitrator() {}
	~Urlarbitrator() { Clear();}

	AccessInfo* Insert(std::string& key, Access* record);
	void Detect(tbb::concurrent_vector<Access*>& buffer);
	void Clear();

	void ExportPaths(std::vector<std::string*>& paths, std::vector<uint32_t>& txs, std::vector<uint32_t>& idLengths) {} // Export conflicts grouped by path, a conflict path cause many transitions to fail
};

