#include <gtest/gtest.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "Lab3.h"

TEST(ArrayTest, ZeroInitialization) {
    int size = 1000;
    int* arr = new int[size]();
    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(arr[i], 0);
    }
    delete[] arr;
}

TEST(SyncObjectsTest, EventsCreation) {
    SharedData sd{};
    InitializeCriticalSection(&sd.cs_console);
    sd.hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    EXPECT_NE(sd.hStartEvent, nullptr);

    HANDLE hCannot = CreateEvent(NULL, FALSE, FALSE, NULL);
    HANDLE hContinue = CreateEvent(NULL, FALSE, FALSE, NULL);
    HANDLE hTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
    EXPECT_NE(hCannot, nullptr);
    EXPECT_NE(hContinue, nullptr);
    EXPECT_NE(hTerminate, nullptr);

    CloseHandle(sd.hStartEvent);
    CloseHandle(hCannot);
    CloseHandle(hContinue);
    CloseHandle(hTerminate);
    DeleteCriticalSection(&sd.cs_console);
}

TEST(MarkerLogicTest, SingleMarkerThreadMarksArray) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    SharedData sd{};
    InitializeCriticalSection(&sd.cs_console);
    sd.arr_size = 10;
    sd.arr = new int[sd.arr_size]();
    sd.num_markers = 1;

    sd.hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    ThreadData td{};
    td.id = 1;
    td.shared = &sd;
    td.hCannotContinue = CreateEvent(NULL, FALSE, FALSE, NULL);
    td.hContinue = CreateEvent(NULL, FALSE, FALSE, NULL);
    td.hTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);

    DWORD tid;
    HANDLE hThread = CreateThread(NULL, 0, MarkerThread, &td, 0, &tid);
    ASSERT_NE(hThread, nullptr);

    SetEvent(sd.hStartEvent);

    Sleep(100);

    bool foundMark = false;
    for (int i = 0; i < sd.arr_size; i++) {
        if (sd.arr[i] != 0) {
            foundMark = true;
            EXPECT_EQ(sd.arr[i], 1);  
        }
    }
    EXPECT_TRUE(foundMark);

    SetEvent(td.hTerminate);
    WaitForSingleObject(hThread, 1000);

    CloseHandle(hThread);
    delete[] sd.arr;
    CloseHandle(sd.hStartEvent);
    CloseHandle(td.hCannotContinue);
    CloseHandle(td.hContinue);
    CloseHandle(td.hTerminate);
    DeleteCriticalSection(&sd.cs_console);
}
