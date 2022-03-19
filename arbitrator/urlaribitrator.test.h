#pragma once
void hex_string(char str[], int length);

bool TestDetection();
bool TestDetection1m();
bool TestFixedLengthArrayConflict();
bool TestSortedMapConflict();
bool TestQueueConflict();
bool TestSortedMapConflict2();

bool TestCase3();
bool TestCaseAsynchronousConflictFree();
bool TestCaseAsynchronousWithConflict();
bool TestCompositeAndReads();
bool TestCompositeAndWrites();

bool TestDetection1mAsynchronous();