#pragma once
#include <vector>
#include <windows.h>

using namespace std;

struct ProcArgs {
	vector<long long>* vec;
	long long min_element;
	long long max_element;
	long long average;
};

DWORD WINAPI MinMaxProc(LPVOID lpParam);
DWORD WINAPI AverageProc(LPVOID lpParam);