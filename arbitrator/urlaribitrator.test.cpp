#include "stdafx.h"

struct AccessStub {
	std::string path;
	uint32_t reads = 0;
	uint32_t writes = 0;
	bool composite= false; // if a composite parent path access

	uint32_t group = UINT32_MAX;
	uint32_t tx = UINT32_MAX;
	
	AccessStub() {}
	AccessStub(uint32_t tx, uint32_t group, std::string path, uint32_t reads, uint32_t writes, bool composite) // For unit tests only
		: tx(tx), group(group), path(path), reads(reads), writes(writes), composite(composite) {}

	bool operator == (AccessStub& rhs) {
		return path.size() == rhs.path.size() && std::memcmp(path.data(), rhs.path.data(), path.size()) == 0;
	}

	bool operator < (const AccessStub &rhs) const {
		if (path.size() != rhs.path.size()) {
			return path.size() < rhs.path.size();
		}

		int flag = std::memcmp(path.data(), rhs.path.data(), path.size());
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
};

bool ConvertRecords(std::vector<AccessStub> ordered, std::vector<uint32_t> groupIDs, std::vector<uint32_t> groupIDslength, std::vector<uint32_t> targetRemoval) {
	char msgBuffer[4096];
	std::vector<uint32_t> ids(ordered.size(), 0);
	std::vector<uint32_t> txs(ordered.size(), 0);
	std::vector<std::string> paths(ordered.size());
	std::vector<uint32_t> reads(ordered.size(), 0);
	std::vector<uint32_t> writes(ordered.size(), 0);
	bool* addOrDeletes = new bool[ordered.size()];
	bool* composite = new bool[ordered.size()];
	std::vector<uint32_t> pathLen(ordered.size(), 0);

	for (std::size_t i = 0; i < ordered.size(); i++) {
		txs[i] = ordered[i].tx;
		paths[i] = ordered[i].path;
		reads[i] = ordered[i].reads;
		writes[i] = ordered[i].writes;
	//	addOrDeletes[i] = ordered[i].addOrDelete;
		composite[i] = ordered[i].composite;
	}

	std::string concatenated;
	for (std::size_t i = 0; i < ordered.size(); i++) {
		concatenated += ordered[i].path;
		pathLen[i] = (ordered[i].path.size());
	}

	auto t0 = std::chrono::steady_clock::now();
	void* arbi = UrlarbitratorStart();
	//std::cout << "Start(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	std::vector<uint32_t> conflictGroup(ordered.size(), UINT32_MAX);
	std::vector<uint8_t> flags(ordered.size(), 0);
	uint32_t counter = 0;

	t0 = std::chrono::steady_clock::now();
	UrlarbitratorInsert(arbi, (char*)txs.data(), (char*)concatenated.data(), (char*)pathLen.data(), (char*)reads.data(), (char*)writes.data(), (char*)composite, ordered.size());
	std::cout << "Insert(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	//t0 = std::chrono::steady_clock::now();
	UrlarbitratorDetect(arbi, (char*)groupIDs.data(), (char*)groupIDslength.data(), groupIDs.size(), (char*)txs.data(), (char*)&counter, msgBuffer);
	//std::cout << "AsynDetect(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	std::vector<uint32_t> left(Urlarbitrator::MAX_ENTRIES, 0);
	std::vector<uint32_t> right(Urlarbitrator::MAX_ENTRIES, 0);
	uint32_t ps = 0;
	std::cout << "Pair Begin:" << std::endl;
	UrlarbitratorExportConflictPairs(arbi, (char*)left.data(), (char*)right.data(), (char*)&ps);
	for (uint32_t i = 0; i < ps; i++) {
		std::cout << "left = " << left[i] << ", right = " << right[i] << std::endl;
	}
	std::cout << "Pair End." << std::endl;

	txs.resize(counter);
	if (txs != targetRemoval) {
		std::cout << "Error: Wrong removal list " << std::endl;
		for (std::size_t i = 0; i < counter; i++) {
			std::cout << "tx = " << txs[i] << ", group = " << conflictGroup[i] << ", flag = " << int(flags[i]) << std::endl;
		}
	}

	UrlarbitratorClear(arbi);
	return txs == targetRemoval;
}

bool TestDetection() {
	AccessStub trans0(0, 0, "ctrn-0/ctrn-00/elem-0", 0, 1, false);
	AccessStub trans1(0, 0, "ctrn-0/ctrn-00/elem-1", 0, 1, false);

	AccessStub trans2(1, 1, "ctrn-0/ctrn-00/elem-2", 1, 0, true);
	AccessStub trans3(1, 1, "ctrn-0/ctrn-00/elem-1", 0, 1, true);

	AccessStub trans4(2, 2, "ctrn-0/ctrn-01/elem-2", 1, 0, false);
	AccessStub trans5(2, 2, "ctrn-0/ctrn-01/elem-4", 1, 0, false);
	
	AccessStub trans7(3, 3, "ctrn-0/ctrn-00", 0, 1, false);

	AccessStub trans6(4, 4, "ctrn-0/ctrn-00", 1, 0, false);

	std::vector<AccessStub> ordered{ trans0, trans1 , trans2, trans3 ,trans4, trans5, trans6, trans7 };
	return ConvertRecords(ordered, { 0, 1, 2, 3, 4}, { 1, 1, 1, 1, 1 }, {1, 4});
}

bool TestFixedLengthArrayConflict() {
	AccessStub trans0(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/arrayID", 1, 0, false);
	//AccessStub trans1(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID",   0, 1, true);
	AccessStub trans2(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/#", 1, 0, false);
	AccessStub trans3(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/0", 0, 1, true);
	AccessStub trans4(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/@", 1, 0, false);

	AccessStub trans5(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/arrayID", 1, 0, false);
	//AccessStub trans6(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID",   0, 1, true);
	AccessStub trans7(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/#", 1, 0, false);
	AccessStub trans8(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/1", 0, 1, true);
	AccessStub trans9(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/@", 1, 0, false);

	std::vector<AccessStub> ordered{ trans0, trans2, trans3 ,trans4, trans5, trans7, trans8, trans9 };
	return ConvertRecords(ordered, { 2, 3 }, { 1, 1 }, {});
}

bool TestSortedMapConflict2() {
	AccessStub trans0(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/!/map1", 1, 0,  false);
	AccessStub trans1(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/", 0, 1, true);
	AccessStub trans2(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/!", 1, 0,  false);
	AccessStub trans3(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/$736f6d656b6579", 0, 1, false);
	AccessStub trans4(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/@", 1, 0,  false);

	AccessStub trans5(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/!/map1", 1, 0,  false);
	AccessStub trans6(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/", 0, 1, true);
	AccessStub trans7(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/!", 1, 0,  false);
	AccessStub trans8(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/$736f6d656b6579", 0, 1, false);

	std::vector<AccessStub> ordered{ trans0, trans1, trans2, trans3 ,trans4, trans5, trans6, trans7, trans8 };
	return ConvertRecords(ordered, { 2, 3 }, { 1, 1 }, {3});
}

bool TestSortedMapConflict() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	AccessStub trans1(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 1, false);

	AccessStub trans2(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	AccessStub trans3(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 0, false);
	AccessStub trans4(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/!", 1, 0, false);
	AccessStub trans5(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/$key2", 0, 1, true);
	AccessStub trans6(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/@", 1, 0, false);

	AccessStub trans7(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	AccessStub trans8(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 0, false);
	AccessStub trans9(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/!", 1, 0, false);
	AccessStub trans10(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/$key3", 0, 1, true);
	AccessStub trans11(3, UINT32_MAX,  "blcc://eth1.0/contractAddress/storage/containers/mapID/@", 1, 0, false);

	std::vector<AccessStub> ordered{ trans0, trans1 , trans2, trans3 ,trans4, trans5, trans6, trans7, trans8, trans9, trans10, trans11 };
	ConvertRecords(ordered, { 1, 2, 3 }, { 1, 1, 1}, { 2, 3 });

	tbb::parallel_sort(ordered.begin(), ordered.end(), [](auto& lft, auto& rgt) {
		return lft < rgt;
	});

	for (int i = 0; i < ordered.size(); i++) {
		std::cout << ordered[i].path << std::endl;
	}

	return true;
}

void hex_string(char str[], int length) {
	char hex_characters[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' }; 	//hexadecimal characters

	int i;
	for (i = 0; i < length; i++)
	{
		str[i] = hex_characters[rand() % 16];
	}
	str[length] = 0;
}

bool TestDetection1m() {
	int N = 250000;
	std::vector<std::string> addresses(N * 2);
	for (int i = 0; i < N; i++) {
		char str[21];
		hex_string(str, 20);
		str[20] = 0;
		addresses[i * 2] = std::string(str);
		hex_string(str, 20);
		str[20] = 0;
		addresses[i * 2 + 1] = std::string(str);
	}

	std::vector<uint32_t> txIds(N);
	std::vector<AccessStub> ordered(N * 5);
 	for (std::size_t i = 0; i < N; ++i) {
		ordered[i * 5] = AccessStub(i + 1, UINT32_MAX,	   "blcc://eth1.0/account/" + addresses[i * 2] + "/balance", 0, 1, true);
		ordered[i * 5 + 1] = AccessStub(i + 1, UINT32_MAX, "blcc://eth1.0/account/" + addresses[i * 2 + 1] + "/balance", 0, 1, true);
		ordered[i * 5 + 2] = AccessStub(i + 1, UINT32_MAX, "blcc://eth1.0/account/" + addresses[0] + "/balance", 0, 1, true);
		ordered[i * 5 + 3] = AccessStub(i + 1, UINT32_MAX, "blcc://eth1.0/account/" + addresses[i * 2] + "/nonce", 0, 1, true);
		ordered[i * 5 + 4] = AccessStub(i + 1, UINT32_MAX, "blcc://eth1.0/account/" + addresses[i * 2 + 1] + "/", 1, 0,  false);
		txIds[i] = i;
	}
		
	uint32_t i = 2;
	std::vector<uint32_t> targetRemoval(N - 1);
	std::for_each(targetRemoval.begin(), targetRemoval.end(), [&](uint32_t& iter) { iter = i++; });
	
	//std::size_t stride = 30000;
	//for (std::size_t i = 0; i < ordered.size() / stride - 1; i++) {
	//	std::vector<AccessStub> toInsert(ordered.begin() + i * stride, ordered.begin() + (i + 1) * stride);
		ConvertRecords(ordered, txIds, std::vector<uint32_t>(txIds.size(), 1), {});
	//}
	//return true;
}

bool TestDetection1mAsynchronous() {
	int N = 200000;
	std::vector<std::string> addresses(N * 2);
	for (int i = 0; i < N; i++) {
		char str[21];
		hex_string(str, 20);
		str[20] = 0;
		addresses[i * 2] = std::string(str);
		hex_string(str, 20);
		str[20] = 0;
		addresses[i * 2 + 1] = std::string(str);
	}

	std::vector<uint32_t> txIds(N);
	std::vector<AccessStub> ordered(N * 5);
	for (std::size_t i = 0; i < N; ++i) {

		ordered[i * 5] = AccessStub(i + 1, UINT32_MAX,      addresses[i * 2], 0, 1, true);
		ordered[i * 5 + 1] = AccessStub(i + 1, UINT32_MAX,  addresses[i * 2 + 1], 0, 1, true);
		ordered[i * 5 + 2] = AccessStub(i + 1, UINT32_MAX,  addresses[0], 0, 1, true);
		ordered[i * 5 + 3] = AccessStub(i + 1, UINT32_MAX,  addresses[i * 2], 0, 1, true);
		ordered[i * 5 + 4] = AccessStub(i + 1, UINT32_MAX,  addresses[i * 2 + 1], 1, 0,  false);
		txIds[i] = i;
	}

	ConvertRecords(ordered, txIds, std::vector<uint32_t>(txIds.size(), 1), {});
	return true;
}

bool TestQueueConflict() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/queueID", 4, 0,  false);
	AccessStub trans1(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/", 2, 2,  false);
	AccessStub trans2(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/!", 4, 0,  false);
	AccessStub trans3(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/$00000000000000640000000200000000", 1, 2, false);
	AccessStub trans4(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/@", 4, 0,  false);

	AccessStub trans5(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/queueID", 1, 0,  false);
	AccessStub trans6(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/", 0, 1, true);
	AccessStub trans7(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/!", 1, 0,  false);
	AccessStub trans8(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/$00000000000000640000000300000000", 0, 1, false);
	AccessStub trans9(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/@", 1, 0,  false);

	std::vector<AccessStub> ordered{ trans0, trans1, trans2, trans3, trans4, trans5, trans6, trans7, trans8, trans9 };
	return ConvertRecords(ordered, { 1, 2 }, {1, 1}, {2});
}

bool TestCase3() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);
	AccessStub trans1(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	
	AccessStub trans2(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key2", 0, 1,  false);	
	AccessStub trans3(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	
	AccessStub trans4(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);	
	AccessStub trans5(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	

	AccessStub trans6(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 1, 0,  false);
	AccessStub trans7(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	
	AccessStub trans8(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	
	AccessStub trans9(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans10(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key1", 0, 1,  false);	
	AccessStub trans11(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key2", 0, 1,  false);	
	AccessStub trans12(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans13(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key1", 0, 1,  false);	
	AccessStub trans14(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key2", 0, 1,  false);
	AccessStub trans15(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);	
	std::vector<AccessStub> ordered{trans0, trans1, trans2, trans3, trans4, trans5, trans6, trans7, trans8, trans9, trans10, trans11, trans12, trans13, trans14, trans15};
	return ConvertRecords(ordered, {1, 2, 3, 4}, { 1, 1, 1, 1 }, {2});
}

bool TestCaseAsynchronousConflictFree() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);
	AccessStub trans1(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans2(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key2", 0, 1,  false);
	AccessStub trans3(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans4(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);
	AccessStub trans5(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans6(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 1, 0,  false);
	AccessStub trans7(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans8(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans9(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans10(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key1", 0, 1,  false);
	AccessStub trans11(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key2", 0, 1,  false);
	AccessStub trans12(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans13(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key1", 0, 1,  false);
	AccessStub trans14(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key2", 0, 1,  false);
	AccessStub trans15(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	std::vector<AccessStub> ordered{ trans0, trans1, trans2, trans3, trans4, trans5, trans6, trans7, trans8, trans9, trans10, trans11, trans12, trans13, trans14, trans15 };
	return ConvertRecords(ordered, {1, 3, 4}, { 1, 1, 1}, {});
}

bool TestCaseAsynchronousWithConflict() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);
	AccessStub trans1(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans2(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key2", 0, 1,  false);
	AccessStub trans3(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans4(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 0, 1,  false);
	AccessStub trans5(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans6(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map1/key1", 1, 0,  false);
	AccessStub trans7(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans8(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	AccessStub trans9(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans10(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key1", 0, 1,  false);
	AccessStub trans11(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map2/key2", 0, 1,  false);
	AccessStub trans12(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);

	AccessStub trans13(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key1", 0, 1,  false);
	AccessStub trans14(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/containers/map3/key2", 0, 1,  false);
	AccessStub trans15(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/balance", 0, 1, true);
	std::vector<AccessStub> ordered{ trans0, trans1, trans2, trans3, trans4, trans5, trans6, trans7, trans8, trans9, trans10, trans11, trans12, trans13, trans14, trans15 };
	return ConvertRecords(ordered, { 1, 2, 3, 4}, { 1, 1, 1, 1}, { 2 });
}

bool TestCompositeAndReads() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, true);
	AccessStub trans1(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 1, 0, false);
	AccessStub trans2(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 1, 0, false);
	AccessStub trans3(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, true);
	return ConvertRecords({ trans0, trans1, trans2, trans3 }, {1, 2, 3, 4}, { 1, 1, 1, 1 }, {2, 3});
}

bool TestCompositeAndWrites() {
	AccessStub trans0(1, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, true);
	AccessStub trans1(2, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, true);
	AccessStub trans2(3, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, false);
	AccessStub trans3(4, UINT32_MAX, "blcc://eth1.0/accounts/Alice/storage/map1/key1", 0, 1, false);
	return ConvertRecords({ trans0, trans1, trans2, trans3 }, { 1, 2, 3, 4 }, { 1, 1, 1, 1 }, { 3, 4 });
}