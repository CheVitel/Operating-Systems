#include "../Lab2/Lab2.h"
#include <gtest/gtest.h>
#include <vector>
#include <Windows.h>

using namespace std;

TEST(ThreadProc, MinMaxSimpleExample) {
	vector<long long> v{ 5,2,9,1,7 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, MinMaxProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.min_element, 1);
	EXPECT_EQ(args.max_element, 9);
}

TEST(ThreadProc, MinMaxSimpleExampleWithNegative) {
	vector<long long> v{ -1000, 34, 35, -2, 0 , 853, -34 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, MinMaxProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.min_element, -1000);
	EXPECT_EQ(args.max_element, 853);
}

TEST(ThreadProc, MinMaxAllEqualsPositive) {
	vector<long long> v{ 9, 9, 9, 9, 9, 9, 9 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, MinMaxProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.min_element, 9);
	EXPECT_EQ(args.max_element, 9);
}

TEST(ThreadProc, MinMaxAllEqualsNegative) {
	vector<long long> v{ -52, -52, -52, -52, -52, -52, -52, -52, -52 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, MinMaxProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.min_element, -52);
	EXPECT_EQ(args.max_element, -52);
}

TEST(ThreadProc, AverageSimpleExample) {
	vector<long long> v{ 1,2,3,4 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, AverageProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.average, 2);
}

TEST(ThreadProc, AverageSimpleExampleWithNegative) {
	vector<long long> v{ 5, 3, -7, 9, 8 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, AverageProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.average, 3); 
}

TEST(ThreadProc, AverageAllNegative) {
	vector<long long> v{ -500, -200, -3, -2, -780 };
	ProcArgs args{ &v, NULL, NULL, 0 };

	HANDLE h = CreateThread(nullptr, 0, AverageProc, &args, 0, nullptr);

	ASSERT_NE(h, nullptr);

	WaitForSingleObject(h, INFINITE);
	CloseHandle(h);

	EXPECT_EQ(args.average, -297);
}