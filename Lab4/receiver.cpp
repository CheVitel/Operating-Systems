#include "Lab5.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    // 1. Вводим имя файла и ёмкость очереди
    string fileName;
    int capacity = 0;
    cout << "Введите имя бинарного файла: ";
    cin >> fileName;
    cout << "Введите количество записей (ёмкость): ";
    cin >> capacity;
    if (capacity <= 0) { cerr << "Ошибка: ёмкость должна быть > 0." << endl; return 1; }

    // 2. Создаём бинарный файл для сообщений
    HANDLE hFile = CreateFileA(fileName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        cerr << "Ошибка создания файла: " << GetLastError() << endl;
        return 1;
    }
    if (!CreateQueueFile(hFile, capacity))
    {
        cerr << "Ошибка инициализации файла." << endl;
        CloseHandle(hFile); return 1;
    }
    cout << "[Receiver] Файл создан: " << fileName << " (слотов: " << capacity << ")" << endl;

    // Создаём объекты синхронизации
    string base = SafeName(fileName);
    HANDLE hSemEmpty = CreateSemaphoreA(NULL, capacity, capacity, ("Global\\Empty_" + base).c_str());
    HANDLE hSemFull  = CreateSemaphoreA(NULL, 0,        capacity, ("Global\\Full_"  + base).c_str());
    HANDLE hMutex    = CreateMutexA    (NULL, FALSE,               ("Global\\Mutex_" + base).c_str());
    if (!hSemEmpty || !hSemFull || !hMutex)
    {
        cerr << "Ошибка создания объектов синхронизации: " << GetLastError() << endl;
        CloseHandle(hFile); return 1;
    }

    // 3. Вводим количество процессов Sender
    int numSenders = 0;
    cout << "Введите количество процессов Sender: ";
    cin >> numSenders;
    if (numSenders <= 0) { cerr << "Ошибка: кол-во Sender > 0." << endl; return 1; }

    // 4. Создаём события готовности для каждого Sender'а
    vector<HANDLE> evReady(numSenders, NULL);
    for (int i = 0; i < numSenders; i++)
    {
        string evName = "Global\\Ready_" + base + "_" + to_string(i);
        evReady[i] = CreateEventA(NULL, TRUE, FALSE, evName.c_str());
        if (!evReady[i])
        {
            cerr << "Ошибка создания события для Sender " << i
                 << ": " << GetLastError() << endl;
            CloseHandle(hFile); return 1;
        }
    }

    // Путь к sender.exe (лежит рядом с receiver.exe)
    char selfPath[MAX_PATH]{};
    GetModuleFileNameA(NULL, selfPath, MAX_PATH);
    string senderExe = selfPath;
    size_t sl = senderExe.find_last_of("\\/");
    if (sl != string::npos) senderExe = senderExe.substr(0, sl + 1);
    senderExe += "sender.exe";

    // Запускаем Sender'ы: sender.exe <fileName> <id> <capacity>
    vector<HANDLE> hProc(numSenders, NULL);
    for (int i = 0; i < numSenders; i++)
    {
        string cmd = "\"" + senderExe + "\" \"" + fileName + "\" "
                     + to_string(i) + " " + to_string(capacity);
        vector<char> buf(cmd.begin(), cmd.end());
        buf.push_back('\0');

        STARTUPINFOA si{};
        si.cb = sizeof(STARTUPINFOA);
        PROCESS_INFORMATION pi{};
        if (!CreateProcessA(NULL, buf.data(), NULL, NULL, FALSE,
                            CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
        {
            cerr << "[Receiver] Ошибка запуска Sender " << i
                 << ": " << GetLastError() << endl;
        }
        else
        {
            hProc[i] = pi.hProcess;
            CloseHandle(pi.hThread);
        }
    }
    cout << "[Receiver] Запущено Sender'ов: " << numSenders << endl;

    // 5. Ждём сигнал готовности от всех Sender'ов
    cout << "[Receiver] Ожидание готовности Sender'ов..." << endl;
    WaitForMultipleObjects((DWORD)numSenders, evReady.data(), TRUE, INFINITE);
    cout << "[Receiver] Все Sender'ы готовы." << endl;
    for (HANDLE ev : evReady) CloseHandle(ev);

    // 6. Основной цикл: r = читать, q = завершить
    SharedData sd{ hSemEmpty, hSemFull, hMutex, hFile, capacity };
    cout << "\nКоманды: [r] читать сообщение  [q] завершить\n" << endl;
    while (true)
    {
        cout << "Receiver> ";
        string cmd;
        cin >> cmd;

        if (cmd == "r" || cmd == "R")
        {
            char msg[MSG_MAX_LEN]{};
            if (Dequeue(sd, msg))
                cout << "[Receiver] Получено: \"" << msg << "\"" << endl;
            else
                cerr << "[Receiver] Ошибка чтения." << endl;
        }
        else if (cmd == "q" || cmd == "Q")
        {
            cout << "[Receiver] Завершение работы." << endl;
            break;
        }
        else
        {
            cout << "Используйте 'r' или 'q'." << endl;
        }
    }

    for (HANDLE hp : hProc) if (hp) CloseHandle(hp);
    CloseHandle(hSemEmpty);
    CloseHandle(hSemFull);
    CloseHandle(hMutex);
    CloseHandle(hFile);
    DeleteFileA(fileName.c_str());
    return 0;
}
