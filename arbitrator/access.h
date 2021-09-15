#pragma once
struct Access {
	std::string* path = nullptr;
	uint32_t reads = 0;
	uint32_t writes = 0;
	uint32_t compositeWrites = 0; // if a composite parent path access

	uint32_t branch = UINT32_MAX;
	uint32_t tx = UINT32_MAX;
	uint32_t group = UINT32_MAX;
	
	Access() {}
	Access(uint32_t tx, uint32_t branch, std::string* path, uint32_t reads, uint32_t writes, bool composite) // For unit tests only
		: tx(tx), branch(branch), path(path), reads(reads), writes(writes) {
		if (composite) {
			this->compositeWrites = writes;
			this->writes = 0;
		}
		assert(this->compositeWrites * this->writes == 0);
	}
	
	bool operator == (Access& rhs) {
		return path->size() == rhs.path->size() && std::memcmp(path->data(), rhs.path->data(), path->size()) == 0;
	}

	bool operator < (const Access &rhs) const {
		if (path->size() != rhs.path->size()) {
			return path->size() < rhs.path->size();
		}

		int flag = std::memcmp(path->data(), rhs.path->data(), path->size());
		if (flag != 0) {
			return flag < 0;
		}

		if (tx != rhs.tx) {
			return tx < rhs.tx;
		}

		if (writes != rhs.writes) {
			return writes > rhs.writes;
		}

		if (reads != rhs.reads) {
			return reads > rhs.reads;
		}
		return false;
	}

	bool IsCompositeOnly() {
		return writes == 0 && reads == 0 && compositeWrites > 0;
	}

	bool IsReadOnly() {
		return writes == 0 && reads > 0;
	}
};

struct Compare { 
	bool operator () (Access* lhs, Access* rhs) const { return *lhs < *rhs; }
};

struct CompareString {
	bool operator () (std::string* lhs, std::string* rhs) const {
		if (lhs->size() == rhs->size())
			return *lhs < *rhs;
		return lhs->size() < rhs->size();
	}
};


struct TxHasher {
	std::size_t operator()(const std::pair<Access*, Access*>& k) const {
		std::size_t h1 = std::hash<uint32_t>{}(k.first->tx);
		std::size_t h2 = std::hash<uint32_t>{}(k.second->tx);
		return h1 ^ (h2 << 1);
	}
};


struct TxEqual {
	bool operator()(const std::pair<Access*, Access*>& lhs, const std::pair<Access*, Access*>& rhs) const {
		return lhs.first->tx == rhs.first->tx && lhs.first->tx == rhs.first->tx;
	}
};