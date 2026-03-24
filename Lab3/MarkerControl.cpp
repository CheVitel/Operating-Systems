#include "MarkerControl.h"
#include "Lab3.h"

//static bool firstConflictReported = false;

MarkerControl::MarkerControl(unsigned int id, vector<int>& arr) : id_(id), arr_(arr) {
	startEvent_ = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);
	stopEvent_ = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);
	threadHandle_ = CreateThread(
		NULL,
		0,
		&MarkerControl::ThreadProc,
		this,
		0,
		NULL
	);
	cannotProceedEvent_ = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);
	continueEvent_ = CreateEvent(
		NULL,
		FALSE,
		FALSE,
		NULL);
}

MarkerControl::~MarkerControl() {
	if (startEvent_) {
		CloseHandle(startEvent_);
	}

	if (stopEvent_) {
		CloseHandle(stopEvent_);
	}
	
	if (cannotProceedEvent_) {
		CloseHandle(cannotProceedEvent_);
	}

	if (continueEvent_) {
		CloseHandle(continueEvent_);
	}
}

DWORD WINAPI MarkerControl::ThreadProc(LPVOID param) {
	return static_cast<MarkerControl*>(param)->marker();
}

DWORD MarkerControl::marker() {
	// Waiting for the "START" signal from main
	WaitForSingleObject(startEvent_, INFINITE);
	WaitForSingleObject(arrMutex, INFINITE);
	cout << "Marker " << id_ << " started" << endl;
	ReleaseMutex(arrMutex);
	srand(id_);
	unsigned placed_count = 0;

	while (true) {
		int idx = rand() % arr_.size();
		WaitForSingleObject(arrMutex, INFINITE);

		if (arr_[idx] == 0) {
			placed_count++;
			Sleep(5);
			arr_[idx] = id_;
			Sleep(5);
			ReleaseMutex(arrMutex);
			continue;
		}

		else {
			cout << "Marker " << id_
				<< ", placed=" << placed_count
				<< ", conflict at index=" << idx
				<< endl;
		}

		ReleaseMutex(arrMutex);
		this->cannotProceed();

		HANDLE h[2] = { continueEvent_, stopEvent_ };
		DWORD ans = WaitForMultipleObjects(2, h, FALSE, INFINITE);

		if (ans == WAIT_OBJECT_0) {
			firstConflictReported = false;
			continue;
		}
		else {
			break;
		}
	}

	WaitForSingleObject(arrMutex, INFINITE);

	for (int& v : arr_) {
		if (v == id_) {
			v = 0;
		}
	}

	cout << "Marker " << id_ << " exiting. Total placed=" << placed_count << endl;
	ReleaseMutex(arrMutex);

	return 0;
}

unsigned int MarkerControl::getId() {
	return id_;
}

HANDLE MarkerControl::getStartEvent() {
	return startEvent_;
}

HANDLE MarkerControl::getStopEvent() {
	return stopEvent_;
}

HANDLE MarkerControl::getThreadHandle() {
	return threadHandle_;
}

HANDLE MarkerControl::getСannotProceedEvent() {
	return cannotProceedEvent_;
}

HANDLE MarkerControl::getContinueEvent() {
	return continueEvent_;
}

void MarkerControl::start() {
	SetEvent(startEvent_);
}

void MarkerControl::stop() {
	SetEvent(stopEvent_);
}

void MarkerControl::join() {
	if (threadHandle_) {
		WaitForSingleObject(threadHandle_, INFINITE);
		CloseHandle(threadHandle_);
		threadHandle_ = NULL;
	}
}

void MarkerControl::reset() {
	ResetEvent(cannotProceedEvent_);
}

void MarkerControl::contin() {
	SetEvent(continueEvent_);
}

void MarkerControl::cannotProceed() {
	SetEvent(cannotProceedEvent_);
}