#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

#include "../Employee.h"
using namespace std;

int main(int arg_count, char* arg_value[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    if (arg_count != 3) {
        cerr << "Пример использования: Creator.exe <Имя бинарного файла> <Количество сотрудников>\n";
        return 1;
    }

    const char* kFilename = arg_value[1];
    int count = stoi(arg_value[2]);

    if (count <= 0) {
        cerr << "Количество > 0\n";
        return 1;
    }

    ofstream ofs(kFilename, ios::binary);

    if (!ofs) {
        cerr << "Не получилось открыть файл: " << kFilename << "\n";
        return 1;
    }

    employee e;

    for (int i = 0; i < count; i++) {
        cout << "Введите 'число' 'имя' 'количество часов' для сотрудника номер" << (i + 1) << ":\n";
        if (!(cin >> e.num >> e.name >> e.hours)) {
            cerr << "Ошибка воода\n";
            return 1;
        }
        while (e.hours < 0) {
            cerr << "Неверное количество часов, должно быть >= 0\n";
            cin >> e.hours;
        }
        ofs.write(reinterpret_cast<char*>(&e), sizeof(e));
    }

    ofs.close();
    return 0;
}