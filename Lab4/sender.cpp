#include "Lab5.h"
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

// Аргументы: sender.exe <fileName> <senderId> <capacity>
int main(int argc, char* argv[])
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    if (argc != 4)
    {
        cerr << "Использование: sender.exe <fileName> <senderId> <capacity>" << endl;
        return 1;
    }

    const char* fileName = argv[1];
    int senderId         = atoi(argv[2]);
    int capacity         = atoi(argv[3]);

    cout << "[Sender " << senderId << "] Запущен. Файл: " << fileName << endl;

    // 1. Открываем файл для передачи сообщений
    HANDLE hFile = CreateFileA(fileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        cerr << "[Sender " << senderId
             << "] Ошибка открытия файла: " << GetLastError() << endl;
        return 1;
    }

    // Подключаемся к объектам синхронизации
    string base = SafeName(string(fileName));
    HANDLE hSemEmpty = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, ("Global\\Empty_" + base).c_str());
    HANDLE hSemFull  = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, ("Global\\Full_"  + base).c_str());
    HANDLE hMutex    = OpenMutexA    (MUTEX_ALL_ACCESS,     FALSE, ("Global\\Mutex_" + base).c_str());
    if (!hSemEmpty || !hSemFull || !hMutex)
    {
        cerr << "[Sender " << senderId
             << "] Ошибка подключения к объектам синхронизации: "
             << GetLastError() << endl;
        CloseHandle(hFile); return 1;
    }

    // 2. Отправляем сигнал готовности Receiver'у
    string evName = "Global\\Ready_" + base + "_" + to_string(senderId);
    HANDLE hReady = OpenEventA(EVENT_ALL_ACCESS, FALSE, evName.c_str());
    if (!hReady)
    {
        cerr << "[Sender " << senderId
             << "] Ошибка открытия события: " << GetLastError() << endl;
        CloseHandle(hFile); return 1;
    }
    SetEvent(hReady);
    CloseHandle(hReady);
    cout << "[Sender " << senderId << "] Готов к работе." << endl;

    // 3. Основной цикл: s = отправить, q = завершить
    SharedData sd{ hSemEmpty, hSemFull, hMutex, hFile, capacity };
    cout << "\nКоманды: [s] отправить сообщение  [q] завершить\n" << endl;
    while (true)
    {
        cout << "Sender[" << senderId << "]> ";
        string cmd;
        cin >> cmd;

        if (cmd == "s" || cmd == "S")
        {
            cout << "Сообщение (до " << MSG_MAX_LEN - 1 << " символов): ";
            string msg;
            cin.ignore();
            getline(cin, msg);
            if ((int)msg.size() >= MSG_MAX_LEN)
                msg.resize(MSG_MAX_LEN - 1);

            if (Enqueue(sd, msg.c_str()))
                cout << "[Sender " << senderId
                     << "] Отправлено: \"" << msg << "\"" << endl;
            else
                cerr << "[Sender " << senderId << "] Ошибка записи." << endl;
        }
        else if (cmd == "q" || cmd == "Q")
        {
            cout << "[Sender " << senderId << "] Завершение." << endl;
            break;
        }
        else
        {
            cout << "Используйте 's' или 'q'." << endl;
        }
    }

    CloseHandle(hSemEmpty);
    CloseHandle(hSemFull);
    CloseHandle(hMutex);
    CloseHandle(hFile);
    return 0;
}
