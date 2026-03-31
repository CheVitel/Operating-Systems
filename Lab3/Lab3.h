#pragma once
#include <windows.h>
#include <vector>

struct SharedData
{
    int* arr;
    int arr_size;
    int num_markers;
    HANDLE hStartEvent;
    CRITICAL_SECTION cs_console;
};

struct ThreadData
{
    int id;
    HANDLE hCannotContinue;
    HANDLE hContinue;
    HANDLE hTerminate;
    SharedData* shared;
};

DWORD WINAPI MarkerThread(LPVOID param);
void PrintArray(const SharedData& sd);