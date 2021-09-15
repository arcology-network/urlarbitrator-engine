#pragma once
struct AccessInfo {
	std::string path;
	std::mutex mutex;

	uint32_t totalRead = 0;
	uint32_t totalWrites = 0;
	uint32_t totalCompositeWrites = 0;

	std::set<Access*, Compare> records;
	std::vector<Access*> conflicts;
	std::vector<Access*> nonconflicts;

	AccessInfo(std::string& path) :path(path) {}
	~AccessInfo() {
		for (auto iter = records.begin(); iter != records.end(); iter++) {
			delete (*iter);
		}
	}
	
	void Add(Access* record);
	void Detect(tbb::concurrent_unordered_set<uint32_t>& whitelist);
	bool Predetermine(Access* record);
	void ExportCombinations(std::vector<std::pair<Access*, Access*>>& buffer, uint32_t maxSize);
};
