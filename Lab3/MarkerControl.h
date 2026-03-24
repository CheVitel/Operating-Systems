#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

class MarkerControl {
private:
    unsigned int id_;
    HANDLE threadHandle_;
    HANDLE startEvent_;
    HANDLE stopEvent_;
    HANDLE cannotProceedEvent_;
    HANDLE continueEvent_;
    vector<int>& arr_;

    DWORD marker();

public:
    MarkerControl(unsigned int id, vector<int>& arr);
    ~MarkerControl();

    static DWORD WINAPI ThreadProc(LPVOID param);
    unsigned int getId();
    HANDLE getStartEvent();
    HANDLE getStopEvent();
    HANDLE getThreadHandle();
    HANDLE get—annotProceedEvent();
    HANDLE getContinueEvent();
    void start();
    void stop();
    void join();
    void reset();
    void contin();
    void cannotProceed();
};