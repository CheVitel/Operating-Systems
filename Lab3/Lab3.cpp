#include "Lab3.h"
#include <iostream>
#include <vector>
using namespace std;

void PrintArray(const SharedData& sd)
{
    for (int i = 0; i < sd.arr_size; i++)
        cout << sd.arr[i] << " ";
    cout << endl;
}

DWORD WINAPI MarkerThread(LPVOID param)
{
    ThreadData* td = static_cast<ThreadData*>(param);
    SharedData& sd = *td->shared;
    const int id = td->id;

    // 1
    WaitForSingleObject(sd.hStartEvent, INFINITE);

    // 2
    srand(id);

    // 3
    while (true)
    {
        // 3.1–3.2
        int idx = rand() % sd.arr_size;

        // 3.3
        if (sd.arr[idx] == 0)
        {
            Sleep(5);
            sd.arr[idx] = id;
            Sleep(5);
        }
        else
        {
            // 3.4

            // 3.4.1
            int count = 0;
            for (int i = 0; i < sd.arr_size; i++)
                if (sd.arr[i] == id)
                    count++;

            EnterCriticalSection(&sd.cs_console);
            cout << "[Marker " << id << "] "
                << "помечено=" << count
                << ", заблокирован на индексе=" << idx
                << endl;
            LeaveCriticalSection(&sd.cs_console);

            // 3.4.2
            SetEvent(td->hCannotContinue);

            // 3.4.3
            HANDLE waitArr[2] = { td->hContinue, td->hTerminate };
            DWORD result = WaitForMultipleObjects(2, waitArr, FALSE, INFINITE);

            if (result == WAIT_OBJECT_0 + 1)
            {
                // 4.1
                for (int i = 0; i < sd.arr_size; i++)
                    if (sd.arr[i] == id)
                        sd.arr[i] = 0;
                // 4.2
                return 0;
            }
        }
    }
}

#ifndef BUILDING_FOR_TESTS
int main()
{
    try
    {
        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);

        SharedData sd{};

        // 1–2
        cout << "Введите размерность массива: ";
        cin >> sd.arr_size;
        sd.arr = new int[sd.arr_size]();

        // 3
        cout << "Введите количество потоков marker: ";
        cin >> sd.num_markers;

        InitializeCriticalSection(&sd.cs_console);
        sd.hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        vector<ThreadData> td(sd.num_markers);
        vector<HANDLE> hThreads(sd.num_markers);
        vector<bool> alive(sd.num_markers, true);

        for (int i = 0; i < sd.num_markers; i++)
        {
            td[i].id = i + 1;
            td[i].shared = &sd;
            td[i].hCannotContinue = CreateEvent(NULL, FALSE, FALSE, NULL);
            td[i].hContinue = CreateEvent(NULL, FALSE, FALSE, NULL);
            td[i].hTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
        }

        // 4
        for (int i = 0; i < sd.num_markers; i++)
        {
            DWORD tid;
            hThreads[i] = CreateThread(NULL, 0, MarkerThread, &td[i], 0, &tid);
            if (hThreads[i] == NULL)
            {
                cerr << "Ошибка создания потока " << i + 1 << endl;
                return GetLastError();
            }
        }

        // 5
        SetEvent(sd.hStartEvent);

        int remaining = sd.num_markers;

        while (remaining > 0)
        {
            // 6.1
            for (int i = 0; i < sd.num_markers; i++)
            {
                if (alive[i])
                    WaitForSingleObject(td[i].hCannotContinue, INFINITE);
            }

            // 6.2
            cout << "\n[Main] Состояние массива:\n";
            PrintArray(sd);

            // 6.3
            int term_id = -1;
            while (true)
            {
                cout << "[Main] Номер потока для завершения (1-" << sd.num_markers << "): ";
                cin >> term_id;
                if (term_id >= 1 && term_id <= sd.num_markers && alive[term_id - 1])
                    break;
                cout << "       Некорректный номер или поток уже завершён. Повторите.\n";
            }

            // 6.4
            SetEvent(td[term_id - 1].hTerminate);

            // 6.5
            WaitForSingleObject(hThreads[term_id - 1], INFINITE);
            alive[term_id - 1] = false;
            remaining--;

            // 6.6
            cout << "[Main] Массив после завершения потока " << term_id << ":\n";
            PrintArray(sd);

            // 6.7
            for (int i = 0; i < sd.num_markers; i++)
            {
                if (alive[i])
                    SetEvent(td[i].hContinue);
            }
        }
        // 7
        cout << "\n[Main] Все потоки marker завершены. Работа программы окончена.\n";

        delete[] sd.arr;
        CloseHandle(sd.hStartEvent);
        for (int i = 0; i < sd.num_markers; i++)
        {
            CloseHandle(td[i].hCannotContinue);
            CloseHandle(td[i].hContinue);
            CloseHandle(td[i].hTerminate);
            CloseHandle(hThreads[i]);
        }
        DeleteCriticalSection(&sd.cs_console);
    }
    catch (const ios_base::failure& e)
    {
        cerr << "Input error: invalid format." << endl;

        return 1;
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;

        return 1;
    }

    return 0;
}
#endif