#include "Lab4.h"
#include <cstring>
using namespace std;

// ─── Файловый ввод-вывод ─────────────────────────────────────────────────

bool WriteAt(HANDLE hFile, long long offset, const void* buf, DWORD size)
{
    LARGE_INTEGER li;
    li.QuadPart = offset;
    if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN)) return false;
    DWORD written = 0;
    return WriteFile(hFile, buf, size, &written, NULL) && written == size;
}

bool ReadAt(HANDLE hFile, long long offset, void* buf, DWORD size)
{
    LARGE_INTEGER li;
    li.QuadPart = offset;
    if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN)) return false;
    DWORD readed = 0;
    return ReadFile(hFile, buf, size, &readed, NULL) && readed == size;
}

// Смещение слота idx (+1 чтобы перепрыгнуть через заголовок)
long long SlotOffset(int idx)
{
    return (long long)(idx + 1) * MSG_MAX_LEN;
}

// ─── Вспомогательные ─────────────────────────────────────────────────────

// Из имени файла делаем безопасный суффикс для имён объектов ядра
string SafeName(const string& fileName)
{
    string s = fileName;
    for (char& c : s)
        if (c == '\\' || c == '/' || c == ':') c = '_';
    return s;
}

bool CreateQueueFile(HANDLE hFile, int capacity)
{
    LARGE_INTEGER sz;
    sz.QuadPart = (long long)(capacity + 1) * MSG_MAX_LEN;
    if (!SetFilePointerEx(hFile, sz, NULL, FILE_BEGIN)) return false;
    if (!SetEndOfFile(hFile)) return false;

    QueueHeader hdr{};
    hdr.capacity = capacity;
    LARGE_INTEGER zero{};
    zero.QuadPart = 0;
    SetFilePointerEx(hFile, zero, NULL, FILE_BEGIN);
    DWORD w = 0;
    return WriteFile(hFile, &hdr, sizeof(hdr), &w, NULL) && w == sizeof(hdr);
}

// ─── Очередь: запись и чтение ────────────────────────────────────────────

bool Enqueue(SharedData& sd, const char* text)
{
    if (WaitForSingleObject(sd.hSemEmpty, INFINITE) != WAIT_OBJECT_0) return false;

    WaitForSingleObject(sd.hMutex, INFINITE);

    QueueHeader hdr{};
    ReadAt(sd.hFile, 0, &hdr, sizeof(hdr));

    MessageRecord rec{};
    strncpy_s(rec.text, text, MSG_MAX_LEN - 1);

    WriteAt(sd.hFile, SlotOffset(hdr.tail), &rec, MSG_MAX_LEN);

    hdr.tail = (hdr.tail + 1) % hdr.capacity;
    hdr.count++;
    WriteAt(sd.hFile, 0, &hdr, sizeof(hdr));

    ReleaseMutex(sd.hMutex);
    ReleaseSemaphore(sd.hSemFull, 1, NULL);
    return true;
}

bool Dequeue(SharedData& sd, char out[MSG_MAX_LEN])
{
    if (WaitForSingleObject(sd.hSemFull, INFINITE) != WAIT_OBJECT_0) return false;

    WaitForSingleObject(sd.hMutex, INFINITE);

    QueueHeader hdr{};
    ReadAt(sd.hFile, 0, &hdr, sizeof(hdr));

    MessageRecord rec{};
    ReadAt(sd.hFile, SlotOffset(hdr.head), &rec, MSG_MAX_LEN);
    rec.text[MSG_MAX_LEN - 1] = '\0';
    strncpy_s(out, MSG_MAX_LEN, rec.text, MSG_MAX_LEN - 1);

    hdr.head = (hdr.head + 1) % hdr.capacity;
    hdr.count--;
    WriteAt(sd.hFile, 0, &hdr, sizeof(hdr));

    ReleaseMutex(sd.hMutex);
    ReleaseSemaphore(sd.hSemEmpty, 1, NULL);
    return true;
}
