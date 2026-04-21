#pragma once
#include <windows.h>

#define MAX_MSG_LEN 20

struct MsgRecord
{
    char text[MAX_MSG_LEN];
};

struct QueueHeader
{
    int capacity;
    int head;
    int tail;
};

HANDLE OpenBinFile(const char* filename);
bool ReadHeader(HANDLE hFile, QueueHeader& hdr);
bool WriteHeader(HANDLE hFile, const QueueHeader& hdr);
bool ReadSlot(HANDLE hFile, int idx, MsgRecord& rec);
bool WriteSlot(HANDLE hFile, int idx, const MsgRecord& rec);
