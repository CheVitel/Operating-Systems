#include "Lab4.h"

HANDLE OpenBinFile(const char* filename)
{
    return CreateFileA(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,                                       
        FILE_ATTRIBUTE_NORMAL,
        NULL);
}

bool ReadHeader(HANDLE hFile, QueueHeader& hdr)
{
    DWORD dwRead;
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    return ReadFile(hFile, &hdr, sizeof(QueueHeader), &dwRead, NULL) 
        && dwRead == sizeof(QueueHeader);
}

bool WriteHeader(HANDLE hFile, const QueueHeader& hdr)
{
    DWORD dwWritten;
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    return WriteFile(hFile, &hdr, sizeof(QueueHeader), &dwWritten, NULL) 
        && dwWritten == sizeof(QueueHeader);
}

bool ReadSlot(HANDLE hFile, int idx, MsgRecord& rec)
{
    DWORD dwRead;
    LONG offset = static_cast<LONG>(sizeof(QueueHeader) + idx * sizeof(MsgRecord));
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    return ReadFile(hFile, &rec, sizeof(MsgRecord), &dwRead, NULL)
        && dwRead == sizeof(MsgRecord);
}

bool WriteSlot(HANDLE hFile, int idx, const MsgRecord& rec)
{
    DWORD dwWritten;
    LONG offset = static_cast<LONG>(sizeof(QueueHeader) + idx * sizeof(MsgRecord));
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    return WriteFile(hFile, &rec, sizeof(MsgRecord), &dwWritten, NULL) 
        && dwWritten == sizeof(MsgRecord);
}
