#include "stdafx.h"


bool ConvertRecords(std::vector<Access>& records, std::vector<uint32_t> targetRemoval) {
	char msgBuffer[4096];
	std::vector<uint32_t> ids(records.size(), 0);
	std::vector<uint32_t> txs(records.size(), 0);
	std::vector<std::string> paths(records.size());
	std::vector<uint32_t> reads(records.size(), 0);
	std::vector<uint32_t> writes(records.size(), 0);
	bool* addOrDeletes = new bool[records.size()];
	bool* composite = new bool[records.size()];
	std::vector<uint32_t> pathLen(records.size(), 0);

	for (std::size_t i = 0; i < records.size(); i++) {
		txs[i] = records[i].tx;
		paths[i] = records[i].path;
		reads[i] = records[i].reads;
		writes[i] = records[i].writes;
		addOrDeletes[i] = records[i].addOrDelete;
		composite[i] = records[i].composite;
	}

	std::string concatenated;
	for (std::size_t i = 0; i < records.size(); i++) {
		concatenated += records[i].path;
		pathLen[i] = (records[i].path.size());
	}

	auto t0 = std::chrono::steady_clock::now();
	void* arbi = Start();
	std::cout << "Start(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	t0 = std::chrono::steady_clock::now();
	Insert(arbi,(char*)txs.data(), nullptr, (char*)concatenated.c_str(), (char*)reads.data(), (char*)writes.data(), (char*)addOrDeletes, (char*)composite, (char*)pathLen.data(), records.size());
	std::cout << "Insert(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	t0 = std::chrono::steady_clock::now();
	//std::vector<uint32_t> txs(records.size());
	std::vector<uint32_t> conflictGroup(records.size(), UINT32_MAX);
	std::vector<uint8_t> flags(records.size(), 0);
	uint32_t counter = 0;
	Detect(arbi,(char*)txs.data(), (char*)conflictGroup.data(), (char*)flags.data(), (char*)&counter, msgBuffer);
	std::cout << "Detect(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	std::vector<uint32_t> conflictTxs;
	for (std::size_t i = 0; i < txs.size(); i++) {
		if (flags[i])
			conflictTxs.push_back(txs[i]);
	}

	std::sort(conflictTxs.begin(), conflictTxs.end());
	auto last = std::unique(conflictTxs.begin(), conflictTxs.end());
	conflictTxs.erase(last, conflictTxs.end());

	if (conflictTxs != targetRemoval) {
		std::cout << "Error: Wrong removal list "<<std::endl;
		for (std::size_t i = 0; i < counter; i++) {
			std::cout << "tx = " << txs[i] << ", group = " << conflictGroup[i] << ", flag = " << int(flags[i]) << std::endl;
		}
	}
	return true;
}

bool TestDetection() {
	Access trans0(0, UINT32_MAX, "ctrn-0/ctrn-00/elem-0", 0, 1, false);
	Access trans1(0, UINT32_MAX, "ctrn-0/ctrn-00/elem-1", 0, 1, false);

	Access trans2(1, UINT32_MAX, "ctrn-0/ctrn-00/elem-2", 1, 0, true);
	Access trans3(1, UINT32_MAX, "ctrn-0/ctrn-00/elem-1", 0, 1, true);

	Access trans4(2, UINT32_MAX, "ctrn-0/ctrn-01/elem-2", 1, 0, false);
	Access trans5(2, UINT32_MAX, "ctrn-0/ctrn-01/elem-4", 1, 0, false);
	
	Access trans7(3, UINT32_MAX, "ctrn-0/ctrn-00", 0, 1, false);

	Access trans6(4, UINT32_MAX, "ctrn-0/ctrn-00", 1, 0, false);

	std::vector<Access> records{ trans0, trans1 , trans2, trans3 ,trans4, trans5, trans6, trans7 };
	ConvertRecords(records, {1, 4});
	return true;
}

bool TestFixedLengthArrayConflict() {
	Access trans0(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/arrayID", 1, 0, false);
	//Access trans1(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID",   0, 1, false, true);
	Access trans2(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/#", 1, 0, false);
	Access trans3(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/0", 0, 1, true);
	Access trans4(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/@", 1, 0, false);

	Access trans5(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/arrayID", 1, 0, false);
	//Access trans6(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID",   0, 1, false, true);
	Access trans7(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/#", 1, 0, false);
	Access trans8(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/1", 0, 1, true);
	Access trans9(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/arrayID/@", 1, 0, false);

	std::vector<Access> records{ trans0, trans2, trans3 ,trans4, trans5, trans7, trans8, trans9 };
	ConvertRecords(records, {});
	return true;
}

bool TestSortedMapConflict2() {
	Access trans0(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/!/map1", 1, 0, false, false);
	Access trans1(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/", 0, 1, false, true);
	Access trans2(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/!", 1, 0, false, false);
	Access trans3(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/$736f6d656b6579", 0, 1, true, false);
	Access trans4(2, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/@", 1, 0, false, false);

	Access trans5(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/!/map1", 1, 0, false, false);
	Access trans6(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/", 0, 1, false, true);
	Access trans7(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/!", 1, 0, false, false);
	Access trans8(3, UINT32_MAX, "blcc://eth1.0/accounts/contractAddress/storage/containers/map1/$736f6d656b6579", 0, 1, true, false);

	std::vector<Access> records{ trans0, trans1, trans2, trans3 ,trans4, trans5, trans6, trans7, trans8 };
	ConvertRecords(records, {3});
	return true;	
}

bool TestSortedMapConflict() {
	Access trans0(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	Access trans1(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 1, false);

	Access trans2(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	Access trans3(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 0, false);
	Access trans4(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/!", 1, 0, false);
	Access trans5(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/$key2", 0, 1, true);
	Access trans6(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/@", 1, 0, false);

	Access trans7(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/mapID", 1, 0, false);
	Access trans8(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/", 1, 0, false);
	Access trans9(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/!", 1, 0, false);
	Access trans10(3, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/mapID/$key3", 0, 1, true);
	Access trans11(3, UINT32_MAX,  "blcc://eth1.0/contractAddress/storage/containers/mapID/@", 1, 0, false);

	std::vector<Access> records{ trans0, trans1 , trans2, trans3 ,trans4, trans5, trans6, trans7, trans8, trans9, trans10, trans11 };
	ConvertRecords(records, { 2, 3 });

	tbb::parallel_sort(records.begin(), records.end(), [](auto& lft, auto& rgt) {
		return lft < rgt;
	});

	for (int i = 0; i < records.size(); i++) {
		std::cout << records[i].path << std::endl;
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

bool TestDetection10m() {
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

	std::vector<Access> records(N * 5);
	for (std::size_t i = 0; i < N; ++i) {
		records[i * 5] = Access(i + 1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i * 2] + "/balance", 0, 1, false, true);
		records[i * 5 + 1] = Access(i + 1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i * 2 + 1] + "/balance", 0, 1, false, true);
		records[i * 5 + 2] = Access(i + 1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[0] + "/balance", 0, 1, false, true);
		records[i * 5 + 3] = Access(i + 1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i * 2] + "/nonce", 0, 1, false, true);
		records[i * 5 + 4] = Access(i + 1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i * 2 + 1] + "/", 1, 0, false, false);
	}
	
	ConvertRecords(records, {});
	return true;
}

bool TestPerf() {
	int N = 50000;
	std::vector<std::string> addresses(N*2);
	for (int i = 0; i < N; i++) {
		char str[21];
		hex_string(str, 20);
		str[20] = 0;
		addresses[i*2] = std::string(str);
		hex_string(str, 20);
		str[20] = 0;
		addresses[i*2 + 1] = std::string(str);
	}
	
	std::vector<Access> records(N*5);
	for (std::size_t i = 0; i < N; ++i) {
		records[i*5] = Access(i+1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i*2] + "/balance", 0, 1, false, true);
		records[i*5+1] = Access(i+1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i*2+1] + "/balance", 0, 1, false, true);
		records[i*5+2] = Access(i+1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[0] + "/balance", 0, 1, false, true);
		records[i*5+3] = Access(i+1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i*2] + "/nonce", 0, 1, false, true);
		records[i*5+4] = Access(i+1, UINT32_MAX, "blcc://eth1.0/accounts/" + addresses[i*2+1] + "/", 1, 0, false, false);
	}
	ConvertRecords(records, {});
}

bool TestQueueConflict() {
	Access trans0(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/queueID", 4, 0, false, false);
	Access trans1(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/", 2, 2, false, false);
	Access trans2(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/!", 4, 0, false, false);
	Access trans3(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/$00000000000000640000000200000000", 1, 2, true, false);
	Access trans4(1, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/@", 4, 0, false, false);

	Access trans5(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/!/queueID", 1, 0, false, false);
	Access trans6(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/", 0, 1, false, true);
	Access trans7(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/!", 1, 0, false, false);
	Access trans8(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/$00000000000000640000000300000000", 0, 1, true, false);
	Access trans9(2, UINT32_MAX, "blcc://eth1.0/contractAddress/storage/containers/queueID/@", 1, 0, false, false);

	std::vector<Access> records{ trans0, trans1, trans2, trans3, trans4, trans5, trans6, trans7, trans8, trans9 };
	ConvertRecords(records, {2});

	return true;
}




