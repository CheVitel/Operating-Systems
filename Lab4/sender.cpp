// Sender.cpp
#include "Lab4.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <limits>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: Sender.exe <filename>" << endl;
        return 1;
    }
    const char* filename = argv[1];

    HANDLE hFile = OpenBinFile(filename);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        cerr << "Sender: failed to open file." << endl;
        return static_cast<int>(GetLastError());
    }

    HANDLE hSemFree = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "Lab4_SemFree");
    if (hSemFree == NULL)
    {
        cerr << "Sender: failed to open SemFree." << endl;
        CloseHandle(hFile);
        return static_cast<int>(GetLastError());
    }
    HANDLE hSemUsed = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "Lab4_SemUsed");
    if (hSemUsed == NULL)
    {
        cerr << "Sender: failed to open SemUsed." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        return static_cast<int>(GetLastError());
    }
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "Lab4_Mutex");
    if (hMutex == NULL)
    {
        cerr << "Sender: failed to open Mutex." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        CloseHandle(hSemUsed);
        return static_cast<int>(GetLastError());
    }
    HANDLE hReady = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "Lab4_SemReady");
    if (hReady == NULL)
    {
        cerr << "Sender: failed to open Ready semaphore." << endl;
        CloseHandle(hFile);
        CloseHandle(hSemFree);
        CloseHandle(hSemUsed);
        CloseHandle(hMutex);
        return static_cast<int>(GetLastError());
    }

    ReleaseSemaphore(hReady, 1, NULL);
    CloseHandle(hReady);

    while (true)
    {
        cout << "[s] send message  [q] quit: ";
        string cmd;
        cin >> cmd;
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');

        if (cmd == "q" || cmd == "Q")
            break;

        if (cmd == "s" || cmd == "S")
        {
            cout << "Enter message (up to " << (MAX_MSG_LEN - 1) << " chars): ";
            string input;
            getline(cin, input);

            if (static_cast<int>(input.size()) >= MAX_MSG_LEN)
            {
                input = input.substr(0, MAX_MSG_LEN - 1);
                cout << "Message truncated to " << (MAX_MSG_LEN - 1) << " chars." << endl;
            }

            // Блокируемся до освобождения хотя бы одного свободного слота
            WaitForSingleObject(hSemFree, INFINITE);

            WaitForSingleObject(hMutex, INFINITE);

            QueueHeader hdr;
            if (!ReadHeader(hFile, hdr))
            {
                cerr << "Failed to read header." << endl;
                ReleaseMutex(hMutex);
                ReleaseSemaphore(hSemFree, 1, NULL); // возвращаем слот
                break;
            }

            MsgRecord rec;
            ZeroMemory(&rec, sizeof(MsgRecord));
            strncpy_s(rec.text, MAX_MSG_LEN, input.c_str(), _TRUNCATE);
            if (!WriteSlot(hFile, hdr.tail, rec))
            {
                cerr << "Failed to write slot." << endl;
                ReleaseMutex(hMutex);
                ReleaseSemaphore(hSemFree, 1, NULL);
                break;
            }

            hdr.tail = (hdr.tail + 1) % hdr.capacity;
            if (!WriteHeader(hFile, hdr))
            {
                cerr << "Failed to write header." << endl;
                ReleaseMutex(hMutex);
                ReleaseSemaphore(hSemFree, 1, NULL);
                break;
            }

            ReleaseMutex(hMutex);

            // Увеличиваем счётчик занятых слотов
            ReleaseSemaphore(hSemUsed, 1, NULL);
            cout << "Message sent." << endl;
        }
        else
        {
            cout << "Unknown command. Use [s] or [q]." << endl;
        }
    }

    CloseHandle(hFile);
    CloseHandle(hSemFree);
    CloseHandle(hSemUsed);
    CloseHandle(hMutex);

    cout << "Sender done." << endl;
    return 0;
}