#include <iostream>
#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "Lab2.h"

using namespace std;

DWORD WINAPI MinMaxProc(LPVOID lpParam) {
	auto args = static_cast<ProcArgs*>(lpParam);
	vector<long long>& vec = *args->vec;
	args->max_element = vec[0];
	args->min_element = vec[0];

	for (long long x : vec) {
		if (args->max_element < x) {
			args->max_element = x;
		}
		else if (args->min_element > x) {
			args->min_element = x;
		}

		Sleep(7);
	}

	cout << endl << "Min element: " << args->min_element << endl
		<< "Max element: " << args->max_element << endl;

	return 0;
}

DWORD WINAPI AverageProc(LPVOID lpParam) {
	auto args = static_cast<ProcArgs*>(lpParam);
	vector<long long>& vec = *args->vec;
	args->average = 0;

	for (long long x : vec) {
		args->average += x;
		Sleep(12);
	}

	args->average /= static_cast<long>(vec.size());

	cout << endl << "Avg of elements: " << args->average << endl;

	return 0;
}

#ifndef BUILDING_FOR_TESTS
int main() {
	try {
		cin.exceptions(ios::failbit | ios::badbit);

		long n;
		cout << "Enter the count of elements: ";

		cin >> n;

		while (n < 1) {
			cout << "Count should be >= 1. Try again: ";
			cin >> n;
		}

		vector<long long> arr(n);

		cout << "Enter the array elements:" << endl;

		for (long i = 0; i < n; i++) {
			cin >> arr[i];
		}

		ProcArgs proc_args{ &arr, NULL, NULL, NULL };

		HANDLE min_max_thread = CreateThread(NULL, 0, MinMaxProc, &proc_args, 0, NULL);

		if (NULL == min_max_thread) {
			throw runtime_error("Min-max thread wasn't created.");
		}

		HANDLE average_thread = CreateThread(NULL, 0, AverageProc, &proc_args, 0, NULL);

		if (NULL == average_thread) {
			throw runtime_error("Average thread wasn't created.");
		}

		WaitForSingleObject(min_max_thread, INFINITE);
		WaitForSingleObject(average_thread, INFINITE);
		CloseHandle(min_max_thread);
		CloseHandle(average_thread);

		replace(arr.begin(), arr.end(), proc_args.max_element, proc_args.average);
		replace(arr.begin(), arr.end(), proc_args.min_element, proc_args.average);

		cout << endl << "Array of elements:" << endl;

		for (long long x : arr) {
			cout << x << endl;
		}

		return 0;
	}
	catch (const ios_base::failure& e) {
		cerr << "Input error: invalid format." << endl;

		return 1;
	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;

		return 1;
	}
}
#endif