// Receiver.cpp
#include "Lab4.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <limits>
using namespace std;

int main()
{
    char filename[256];
    int  capacity;
    cout << "Enter binary file name: ";
    cin >> filename;
    cout << "Enter number of slots: ";
    cin >> capacity;
    if (capacity <= 0)
    {
        cerr << "Number of slots must be > 0." << endl;
        return 1;
    }

    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        cerr << "Failed to create file." << endl;
        return static_cast<int>(GetLastError());
    }

    QueueHeader hdr;
    hdr.capacity = capacity;
    hdr.head = 0;
    hdr.tail = 0;
    if (!WriteHeader(hFile, hdr))
    {
        cerr << "Failed to write header." << endl;
        CloseHandle(hFile);
        return 1;
    }

    MsgRecord empty;
    ZeroMemory(&empty, sizeof(MsgRecord));
    for (int i = 0; i < capacity; i++)
    {
        if (!WriteSlot(hFile, i, empty))
        {
            cerr << "Failed to initialize slot " << i << endl;
            CloseHandle(hFile);
            return 1;
        }
    }

    HANDLE hSemFree = CreateSemaphoreA(NULL, capacity, capacity, "Lab4_SemFree");
    if (hSemFree == NULL)
    {
        cerr << "Failed to create SemFree." << endl;
        CloseHandle(hFile);
        return static_cast<int>(GetLastError());
    }
    HANDLE hSemUsed = CreateSemaphoreA(NULL, 0, capacity, "Lab4_SemUsed");
    if (hSemUsed == NULL)
    {
        cerr << "Failed to create SemUsed." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        return static_cast<int>(GetLastError());
    }
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Lab4_Mutex");
    if (hMutex == NULL)
    {
        cerr << "Failed to create Mutex." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        CloseHandle(hSemUsed);
        return static_cast<int>(GetLastError());
    }

    int senderCount;
    cout << "Enter number of Sender processes: ";
    cin >> senderCount;
    if (senderCount <= 0)
    {
        cerr << "Number of Senders must be > 0." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        CloseHandle(hSemUsed);
        CloseHandle(hMutex);
        return 1;
    }
    if (senderCount > 20)
    {
        cerr << "Too much." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        CloseHandle(hSemUsed);
        CloseHandle(hMutex);
        return 1;
    }

    HANDLE hReady = CreateSemaphoreA(NULL, 0, senderCount, "Lab4_SemReady");
    if (hReady == NULL)
    {
        cerr << "Failed to create Ready semaphore." << endl;
        return static_cast<int>(GetLastError());
    }

    vector<PROCESS_INFORMATION> pis(senderCount);
    for (int i = 0; i < senderCount; i++)
    {
        char cmdLine[512];
        sprintf_s(cmdLine, sizeof(cmdLine), "Sender.exe %s", filename);

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(STARTUPINFOA));
        si.cb = sizeof(STARTUPINFOA);

        if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE, NULL, NULL, &si, &pis[i]))
        {
            cerr << "Failed to launch Sender #" << (i + 1) << "." << endl;
            return static_cast<int>(GetLastError());
        }
        cout << "Sender #" << (i + 1) << " started." << endl;
    }

    cout << "Waiting for all Senders..." << endl;
    for (int i = 0; i < senderCount; i++)
        WaitForSingleObject(hReady, INFINITE);
    cout << "All Senders ready." << endl;

    while (true)
    {
        cout << "\n[r] read message  [q] quit: ";
        char cmd;
        cin >> cmd;
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');

        if (cmd == 'q' || cmd == 'Q')
            break;

        if (cmd == 'r' || cmd == 'R')
        {
            WaitForSingleObject(hSemUsed, INFINITE);

            WaitForSingleObject(hMutex, INFINITE);

            if (!ReadHeader(hFile, hdr))
            {
                cerr << "Failed to read header." << endl;
                ReleaseMutex(hMutex);
                break;
            }

            MsgRecord rec;
            if (!ReadSlot(hFile, hdr.head, rec))
            {
                cerr << "Failed to read slot." << endl;
                ReleaseMutex(hMutex);
                break;
            }

            MsgRecord empty2;
            ZeroMemory(&empty2, sizeof(MsgRecord));
            if (!WriteSlot(hFile, hdr.head, empty2))
            {
                cerr << "Failed to clear slot." << endl;
                ReleaseMutex(hMutex);
                break;
            }

            hdr.head = (hdr.head + 1) % hdr.capacity;
            if (!WriteHeader(hFile, hdr))
            {
                cerr << "Failed to write header." << endl;
                ReleaseMutex(hMutex);
                break;
            }

            ReleaseMutex(hMutex);

            cout << "[Receiver] Got: " << rec.text << endl;

            // Освобождаем один слот для писателей
            ReleaseSemaphore(hSemFree, 1, NULL);
        }
        else
        {
            cout << "Unknown command. Use [r] or [q]." << endl;
        }
    }

    // Ожидаем завершения всех процессов Sender (необязательно, но корректно)
    for (int i = 0; i < senderCount; i++)
        WaitForSingleObject(pis[i].hProcess, INFINITE);

    CloseHandle(hFile);
    CloseHandle(hSemFree);
    CloseHandle(hSemUsed);
    CloseHandle(hMutex);
    CloseHandle(hReady);
    for (int i = 0; i < senderCount; i++)
    {
        CloseHandle(pis[i].hProcess);
        CloseHandle(pis[i].hThread);
    }

    cout << "Receiver done." << endl;
    return 0;
}