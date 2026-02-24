#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

#include "../Employee.h"

using namespace std;


bool RunProcess(const string& kCmdLine) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);

    if (!CreateProcessA(
        NULL,
        const_cast<LPSTR>(kCmdLine.c_str()),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi))
    {
        cerr << "Ошибка загрузки: " << kCmdLine << ", code=" << GetLastError() << "\n";

        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

void ShowBinaryFile(const string& kBinFile) {
    ifstream ifs(kBinFile, ios::binary);

    if (!ifs) {
        cerr << "Не удалось открыть бинарный файл: " << kBinFile << "\n";
        return;
    }

    cout << "\n--- Бинарный файл " << kBinFile << " ---\n";
    employee e;

    while (ifs.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        cout << e.num << " " << e.name << " " << e.hours << "\n";
    }

    cout << "\n\n";
    ifs.close();
}


void ShowTextFile(const string& kTxtFile) {
    ifstream ifs(kTxtFile);

    if (!ifs) {
        cerr << "Не удалось открыть файл отчёта: " << kTxtFile << "\n";
        return;
    }

    cout << "\n--- Файл отчёта " << kTxtFile << " ---\n";
    string line;

    while (getline(ifs, line)) {
        cout << line << "\n";
    }

    cout << "\n\n";
    ifs.close();
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_NUMERIC, "C");

    cout << "Введите название бинарного файла и количесвто сотрудников: ";
    string kBinFile;
    int count;

    if (!(cin >> kBinFile >> count)) {
        cerr << "Ошибка ввода\n";
        return 1;
    }

    string creatorCmd = "Creator.exe " + kBinFile + " " + to_string(count);

    if (!RunProcess(creatorCmd)) {
        return 1;
    }

    ShowBinaryFile(kBinFile);

    cout << "Введите название файла отчёта и оплату за час: ";
    string reportFile;
    double rate;

    if (!(cin >> reportFile >> rate)) {
        cerr << "Ошибка ввода\n";
        return 1;
    }

    string reporterCmd = "Reporter.exe " + kBinFile + " " + reportFile + " " + to_string(rate);

    if (!RunProcess(reporterCmd)) {
        return 1;
    }

    ShowTextFile(reportFile);
    cout << "\nПрограмма завершена. Нажмите Enter для выхода...";
    cin.ignore(10000, '\n'); 
    cin.get();
    return 0;
}