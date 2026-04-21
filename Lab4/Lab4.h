#pragma once
#include <windows.h>
#include <string>

// Максимальная длина сообщения (включая '\0')
static const int MSG_MAX_LEN = 20;

// ─── Структуры файла-очереди ──────────────────────────────────────────────

#pragma pack(push, 1)

// Заголовок: первая запись файла (занимает MSG_MAX_LEN байт)
struct QueueHeader
{
    int head;       // индекс слота для чтения
    int tail;       // индекс слота для записи
    int count;      // текущее кол-во сообщений
    int capacity;   // максимальное кол-во сообщений
    char pad[MSG_MAX_LEN - 4 * (int)sizeof(int)];
};

// Одна запись сообщения (MSG_MAX_LEN байт)
struct MessageRecord
{
    char text[MSG_MAX_LEN];
};

#pragma pack(pop)

// ─── Данные, общие для Receiver и Sender ─────────────────────────────────

struct SharedData
{
    HANDLE hSemEmpty;   // счётчик свободных слотов
    HANDLE hSemFull;    // счётчик занятых слотов
    HANDLE hMutex;      // защита файла
    HANDLE hFile;       // дескриптор файла очереди
    int    capacity;    // ёмкость очереди
};

// ─── Объявления общих функций ─────────────────────────────────────────────

bool      WriteAt        (HANDLE hFile, long long offset, const void* buf, DWORD size);
bool      ReadAt         (HANDLE hFile, long long offset,       void* buf, DWORD size);
long long SlotOffset     (int idx);
std::string SafeName     (const std::string& fileName);
bool      CreateQueueFile(HANDLE hFile, int capacity);

bool      Enqueue(SharedData& sd, const char* text);
bool      Dequeue(SharedData& sd, char out[MSG_MAX_LEN]);
