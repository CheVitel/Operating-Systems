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

    if (arg_count != 4) {
        cerr << "Пример использования: Reporter.exe <Имя бинарного файла> <Имя отчёта> <Оплата за час>\n";
        return 1;
    }

    if (stof(arg_value[3]) <= 0) {
        cerr << "Оплата за час должна быть > 0\n";
        return 1;
    }

    const char* kBinfile = arg_value[1];
    const char* kTxtfile = arg_value[2];
    double rate = stof(arg_value[3]);
    ifstream ifs(kBinfile, ios::binary);

    if (!ifs) {
        cerr << "Не получается открыть бинарный файл: " << kBinfile << "\n";
        return 1;
    }

    ofstream ofs(kTxtfile);

    if (!ofs) {
        cerr << "Не получается открыть файл отчёта: " << kTxtfile << "\n";
        return 1;
    }

    ofs << "Отчёт по файлу «" << kBinfile << "».\n";

    employee e;

    while (ifs.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        ofs << e.num << " " << e.name << " " << e.hours << " " << e.hours * rate << "\n";
    }

    ifs.close();
    ofs.close();
    return 0;
}