#include "stdafx.h"

void BenchmarkDatastructs() {
	int N = 500000;
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

	auto t0 = std::chrono::steady_clock::now();
	tbb::parallel_sort(addresses.begin(), addresses.end());
	std::cout << "Vec() + parallel_sort() addresses : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;


	t0 = std::chrono::steady_clock::now();
	std::vector<std::string> nums(1000000);
	for (std::size_t j = 0; j < nums.size(); j++) {
		nums[j] = std::to_string(rand()) + std::to_string(rand());
	}

	tbb::parallel_sort(nums.begin(), nums.end());
	std::cout << "Vec() + parallel_sort(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;


	t0 = std::chrono::steady_clock::now();
	std::set<std::string> numSet;
	for (std::size_t j = 0; j < nums.size() / 10; j++) {
		numSet.insert(std::to_string(rand()) + std::to_string(rand()));
	}
	std::cout << "Set(): " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	t0 = std::chrono::steady_clock::now();
	for (std::size_t i = 0; i < 200000; i++) {
		std::vector<std::string> nums(5);
		for (std::size_t j = 0; j < nums.size(); j++) {
			nums[j] = std::to_string(rand()) + std::to_string(rand());
		}
		tbb::parallel_sort(nums.begin(), nums.end());
	}
	std::cout << "Vec() + parallel_sort()  5 x 200k = 1m: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	std::string str;
	t0 = std::chrono::steady_clock::now();
	for (std::size_t i = 0; i < nums.size() / 10; i++) {
		str = addresses[i];
	}
	//std::cout << str;
	std::cout << "vec loop  1m: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	t0 = std::chrono::steady_clock::now();
	for (auto iter = numSet.begin(); iter != numSet.end(); iter++) {
		str = (*iter);
	}
//	std::cout << str;
	std::cout << "set loop  1m: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	// Get all map keys;
	t0 = std::chrono::steady_clock::now();
	tbb::concurrent_unordered_map<std::string, std::string> numMap(1000000);
	for (std::size_t j = 0; j < 1000000; j++) {
		numMap[std::to_string(rand())] = std::to_string(rand()) + std::to_string(rand());
	}

	t0 = std::chrono::steady_clock::now();
	tbb::concurrent_vector<const std::string*> keys;
	tbb::parallel_for_each(numMap.begin(), numMap.end(), [&](auto iter) { keys.push_back(&(iter.first)); });
	std::cout << "Get all map keys Parallel: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

	t0 = std::chrono::steady_clock::now();
	tbb::concurrent_vector<const std::string*> keys2;
	for (auto iter = numMap.begin();iter != numMap.end(); iter++) {
		keys2.push_back(&(iter->first));
	}
	std::cout << "Get all map keys Sequential: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count() << " ms" << std::endl;

}

int main() {
	if (!TestCompositeAndReads()) {
		std::cout << "Error: TestCompositeAndReads() failed !!!" << std::endl;
	}

	if (!TestDetection()) {
		std::cout << "Error: TestDetection() failed !!!" << std::endl;
	}

	if (!TestFixedLengthArrayConflict()) {
		std::cout << "Error: TestFixedLengthArrayConflict failed !!!" << std::endl;
	}

	if (!TestQueueConflict()) {
		std::cout << "Error: TestQueueConflict failed !!!" << std::endl;
	}

	if (!TestSortedMapConflict2()) {
		std::cout << "Error: TestSortedMapConflict2 failed !!!" << std::endl;
	}

	if (!TestCaseAsynchronousConflictFree()) {
		std::cout << "Error: TestCaseAsynchronousConflictFree failed !!!" << std::endl;
	}

	if (!TestCase3()) {
		std::cout << "Error: TestCase3 failed" << std::endl;
	}

	//if (!TestDetection1m()) {
	//	std::cout << "Error: TestDetection1m failed" << std::endl;
	//}

	//if (!TestDetection1mAsynchronous()) {
	//	std::cout << "Error: TestDetection1mAsynchronous failed" << std::endl;
	//}
}
 
