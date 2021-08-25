#pragma once
class Urlarbitrator;

struct Level {
	std::vector<Access*> indexer;
	std::set<Access*, Compare> orderedAccesses;
	tbb::concurrent_unordered_map<uint32_t, std::vector<Access*>> indexerByTx;
	tbb::concurrent_vector<Access*> conflicts;
	std::string msg;

	uint32_t depth = 0;
	Urlarbitrator * arbitrator = nullptr;

	Level(Urlarbitrator* arbitrator, uint32_t depth) :arbitrator(arbitrator), depth(depth) {
		conflicts.reserve(50000);
	}

	void Insert(std::vector<Access*>& records);
	void Clear();
	 
	static void GetRanges(std::vector<Access*>& indexer, std::vector<std::size_t>& ranges);
	static bool MarkConflicts(std::vector<Access*>& indexer, std::size_t start, std::size_t end, uint32_t conflictGroup);
	static void LabelByTransaction(std::vector<uint32_t>& txs, tbb::concurrent_unordered_map<uint32_t, std::vector<Access*> >& indexerByTx);

	static void GetCombinations(std::vector<Access*>& indexer, std::size_t start, std::size_t end, tbb::concurrent_vector<std::pair<Access*, Access*>>& conflictPairs);
	void Process();
};
