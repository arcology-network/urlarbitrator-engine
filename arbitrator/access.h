#pragma once
struct Access {
	bool isValid = false; // causing direct conflict or not

	std::string path = "";
	char* pathPtr = nullptr;
	uint32_t length = 0; // path length

	uint32_t reads = 0;
	uint32_t writes = 0;
	bool addOrDelete = false;

	uint32_t branch = UINT32_MAX;
	uint32_t tx = UINT32_MAX;
	uint32_t group = UINT32_MAX;
	bool composite = false; // if a composite parent path access

	Access() {}

	Access(uint32_t tx, uint32_t branch, char* pathPtr, uint32_t length, uint32_t reads, uint32_t writes, bool addOrDelete, bool composite = false)
		: tx(tx), branch(branch), pathPtr(pathPtr), length(length), reads(reads), writes(writes), addOrDelete(addOrDelete), isValid(true), composite(composite) {}

	Access(uint32_t tx, uint32_t branch, std::string path, uint32_t reads, uint32_t writes, bool addOrDelete, bool composite = false)
		: tx(tx), branch(branch), path(path), reads(reads), writes(writes), addOrDelete(addOrDelete), isValid(true), composite(composite){}
	
	bool operator == (Access& rgt) {
		return (PathEqual(&rgt) && reads == rgt.reads && writes == rgt.writes && addOrDelete == rgt.addOrDelete);
	}
	
	bool PathEqual( Access* rhs) {
		return length == rhs->length && std::memcmp(pathPtr, rhs->pathPtr, length) == 0;
	}

	bool PathLessThan(Access* rhs) {
		return length < rhs->length || std::memcmp(pathPtr, rhs->pathPtr, length) < 0;
	}

	bool operator < (const Access &rgt) const {
		if (length != rgt.length) {
			return length < rgt.length;
		}

		int flag = std::memcmp(pathPtr, rgt.pathPtr, rgt.length);
		if (flag != 0) {
			return flag < 0;
		}

		if (tx != rgt.tx) {
			return tx < rgt.tx;
		}

		if (writes != rgt.writes) {
			return writes > rgt.writes;
		}

		if (reads != rgt.reads) {
			return reads > rgt.reads;
		}
		return false;
	}

	bool IsCompositeOnly() {
		return writes != 0 && reads == 0 && composite;
	}

	bool IsReadOnly() {
		return writes == 0 && reads > 0;
	}

	std::size_t GetLevel() {
		int level = std::count(pathPtr, pathPtr + length, '/');
		if (length > 0 && pathPtr[length - 1] == '/') {
			-- level;
		}
		return std::max<int>(0, level);
	}
	
	static std::string ExtractParentPath(std::string& path) {
		size_t idx = path.find_last_of('/');
		if (path[path.size() - 1] == '/') {
			idx = path.find_last_of('/', path.size() - 1);
		}
		return path.substr(0, idx + 1);
	}
};

struct Compare { 
	bool operator () (Access* lhs, Access* rhs) const { return *lhs < *rhs; }
};
