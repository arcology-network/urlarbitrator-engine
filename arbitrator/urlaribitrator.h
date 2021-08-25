#pragma once
struct Urlarbitrator {
	std::atomic<uint32_t> groupID = 0; // conflict group 	
	std::vector<Access*> indexer;
	tbb::concurrent_vector<uint32_t> conflicts;
	tbb::concurrent_vector<std::pair<Access*, Access*>> conflictPairs;
	std::vector<char*> memory;
	std::string msg;

	Urlarbitrator() {}
	~Urlarbitrator() {
		for (std::size_t i = 0; i < memory.size(); i++) {
			delete[] memory[i];
		}
	}
	
	void Insert(std::vector<Access*>& records) { 
		indexer.insert(indexer.end(), records.begin(), records.end());
	}
	
	void Detect();	
	void Reorganize();	 
	void Clear();

	void ExportTxs(std::vector<uint32_t>& lftTxs, std::vector<uint32_t>& rgtTxs); // Export conflicts grouped by tx id
	void ExportPaths(std::vector<std::string*>& paths, std::vector<uint32_t>& txs, std::vector<uint32_t>& idLengths) {} // Export conflicts grouped by path, a conflict path cause many transitions to fail

	void GetRanges(std::vector<Access*>& indexer, std::vector<std::size_t>& ranges);
	void GetCombinations(std::vector<Access*>& indexer, std::size_t start, std::size_t end, tbb::concurrent_vector<std::pair<Access*, Access*>>& conflictPairs);
	bool MarkConflicts(std::vector<Access*>& indexer, std::size_t start, std::size_t end, uint32_t conflictGroup);
};

