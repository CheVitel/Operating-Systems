#include <catch2/catch_all.hpp>
#include "../MarkerControl.h"
#include <vector>

//void ExitTest(MarkerControl& a) {
//    SetEvent(a.getStopEvent());
//    WaitForSingleObject(a.getThreadHandle(), INFINITE);
//    CloseHandle(a.getThreadHandle());
//    CloseHandle(a.getStartEvent());
//    CloseHandle(a.getStopEvent());
//}

static void cleanup(MarkerControl& a) {
    a.start();
    a.stop();
    a.join();
}

TEST_CASE("MarkerControl: constructor initializes id and all handles", "[MarkerControl][constructor]") {
    unsigned int expectedId = 42;
    vector<int> data(5, 0);
    MarkerControl ctrl(expectedId, data);

    REQUIRE(ctrl.getId() == expectedId);

    HANDLE hStart = ctrl.getStartEvent();
    HANDLE hStop = ctrl.getStopEvent();
    HANDLE hCannot = ctrl.getÑannotProceedEvent();
    HANDLE hContinue = ctrl.getContinueEvent();
    HANDLE hThread = ctrl.getThreadHandle();

    REQUIRE(hStart != NULL);
    REQUIRE(hStop != NULL);
    REQUIRE(hCannot != NULL);
    REQUIRE(hContinue != NULL);
    REQUIRE(hThread != NULL);

    REQUIRE(hStart != hStop);
    REQUIRE(hStart != hCannot);
    REQUIRE(hStart != hContinue);
    REQUIRE(hStop != hCannot);
    REQUIRE(hStop != hContinue);
    REQUIRE(hCannot != hContinue);

    REQUIRE(WaitForSingleObject(hStart, 0) == WAIT_TIMEOUT);
    REQUIRE(WaitForSingleObject(hStop, 0) == WAIT_TIMEOUT);
    REQUIRE(WaitForSingleObject(hCannot, 0) == WAIT_TIMEOUT);
    REQUIRE(WaitForSingleObject(hContinue, 0) == WAIT_TIMEOUT);

    REQUIRE(WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT);

    SetEvent(hContinue);

    REQUIRE(WaitForSingleObject(hContinue, 0) == WAIT_OBJECT_0);
    REQUIRE(WaitForSingleObject(hContinue, 0) == WAIT_TIMEOUT);

    SetEvent(hCannot);

    REQUIRE(WaitForSingleObject(hCannot, 0) == WAIT_OBJECT_0);
    REQUIRE(WaitForSingleObject(hCannot, 0) == WAIT_OBJECT_0);

    ResetEvent(hCannot);

    REQUIRE(WaitForSingleObject(hCannot, 0) == WAIT_TIMEOUT);

    cleanup(ctrl);
}

TEST_CASE("MarkerControl: ThreadProc", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(1900, data);
    a.start();
    a.stop();
    DWORD result = MarkerControl::ThreadProc(&a);

    REQUIRE(result == 0);

    cleanup(a);
}

TEST_CASE("MarkerControl: getId", "[MarkerControl]") {
    const unsigned int expectedId = 123;
    vector<int> data(3);
    MarkerControl a(expectedId, data);

    REQUIRE(a.getId() == expectedId);

    cleanup(a);
}

TEST_CASE("MarkerControl: getStartEvent", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(1, data);

    REQUIRE(a.getStartEvent() != NULL);
    REQUIRE(WaitForSingleObject(a.getStartEvent(), 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: getStopEvent", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(2, data);

    REQUIRE(a.getStopEvent() != NULL);
    REQUIRE(WaitForSingleObject(a.getStopEvent(), 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: getThreadHandle", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(3, data);

    REQUIRE(a.getThreadHandle() != NULL);
    REQUIRE(WaitForSingleObject(a.getThreadHandle(), 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: getÑannotProceedEvent", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(8, data);

    REQUIRE(a.getÑannotProceedEvent() != NULL);
    REQUIRE(WaitForSingleObject(a.getÑannotProceedEvent(), 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: getContinueEvent", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(33, data);
    HANDLE hEvent = a.getContinueEvent();

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    SetEvent(hEvent);

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);

    ResetEvent(hEvent);

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: start", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(340, data);
    a.start();

    REQUIRE(WaitForSingleObject(a.getStartEvent(), 0) == WAIT_OBJECT_0);

    cleanup(a);
}

TEST_CASE("MarkerControl: stop", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(225, data);
    a.stop();

    REQUIRE(WaitForSingleObject(a.getStopEvent(), 0) == WAIT_OBJECT_0);

    cleanup(a);
}

TEST_CASE("MarkerControl: join", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(42, data);
    a.start();
    a.stop();
    auto t0 = std::chrono::steady_clock::now();
    a.join();
    auto t1 = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

    REQUIRE(dur.count() < 500);

    cleanup(a);
}

TEST_CASE("MarkerControl: reset", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(33, data);
    HANDLE hEvent = a.getÑannotProceedEvent();

    REQUIRE(hEvent != NULL);
    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    SetEvent(hEvent);

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);

    a.reset();

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: contin", "[MarkerControl]") {
    vector<int> data(3);
    MarkerControl a(33, data);
    HANDLE hEvent = a.getContinueEvent();

    REQUIRE(hEvent != NULL);
    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    a.contin();

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);
    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    cleanup(a);
}

TEST_CASE("MarkerControl: cannotProceed", "[MarkerControl]") {
    vector<int> data(2);
    MarkerControl a(84, data);
    HANDLE hEvent = a.getÑannotProceedEvent();

    REQUIRE(hEvent != NULL);
    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT);

    a.cannotProceed();

    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);
    REQUIRE(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);

    a.reset();
    cleanup(a);
}