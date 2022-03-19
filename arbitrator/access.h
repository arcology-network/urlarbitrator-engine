#pragma once
struct Access {
	std::string* path = nullptr;
	uint32_t reads = 0;
	uint32_t writes = 0;
	uint32_t compositeWrites = 0; // if a composite parent path access

	uint32_t group = UINT32_MAX;
	uint32_t tx = UINT32_MAX;
	
	Access() {}
	Access(uint32_t tx, std::string* path, uint32_t reads, uint32_t writes, bool composite) // For unit tests only
		: tx(tx), path(path), reads(reads), writes(writes) {
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
		return writes == 0 && compositeWrites >  0 && reads == 0;
	}

	bool IsReadOnly() {
		return writes == 0 && compositeWrites == 0 && reads > 0;
	}
};

struct Compare { 
	bool operator () (Access* lhs, Access* rhs) const { return *lhs < *rhs; }
};

struct StringEqual {
	bool operator () (std::string* lhs, std::string* rhs) const { return *lhs < *rhs; }
};

struct StringHasher {
	std::size_t operator()(std::string const& str) const noexcept {
		return  std::hash<std::string>{}(str);
	}
};