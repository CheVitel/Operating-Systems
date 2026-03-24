#include "Lab3.h"
#include "MarkerControl.h"

using namespace std;

HANDLE arrMutex = CreateMutex(
	NULL,
	FALSE,
	NULL
);

void showIntArr(vector<int>& arr, bool after) {
	cout << "Arr state" << (after ? " after removal:" : ":") << endl;

	for (int a : arr) {
		cout << a << " ";
	}

	cout << endl;
}

int main() {
	try {
		cin.exceptions(ios::failbit | ios::badbit);
		cout << "Enter size of array: ";
		unsigned int n_arr;
		cin >> n_arr;
		vector<int> arr(n_arr, 0);
		cout << "Enter the number of MARKER threads: ";
		unsigned int n_threads;
		cin >> n_threads;
		vector<MarkerControl*> threads;
		threads.reserve(n_threads);

		for (unsigned int i = 1; i <= n_threads; i++) {
			threads.push_back(new MarkerControl(i, arr));
			//threads.emplace_back(i, arr);
		}

		for (MarkerControl* t : threads) {
			t->start();
		}

		while (!threads.empty()) {
			vector<HANDLE> barriers;
			barriers.reserve(n_threads);

			for (MarkerControl* t : threads) {
				barriers.push_back(t->getСannotProceedEvent());
			}

			DWORD waitResult = WaitForMultipleObjects(
				threads.size(),
				barriers.data(),
				TRUE,
				INFINITE
			);

			for (MarkerControl* t : threads) {
				t->reset();
			}

			showIntArr(arr, false);
			cout << "Enter the sequence number of the marker thread that will be signaled to terminate its operation:" << endl;
			unsigned int id;
			cin >> id;
			auto it = find_if(threads.begin(), threads.end(), [id](MarkerControl* t) { return t->getId() == id; });

			if (it == threads.end()) {
				throw out_of_range("Invalid marker id");
			}

			MarkerControl* victim = *it;
			victim->stop();
			victim->join();
			delete victim;
			threads.erase(it);
			cout << "Joined marker " << id << endl;
			showIntArr(arr, true);

			for (MarkerControl* t : threads) {
				t->contin();
			}
		}

		CloseHandle(arrMutex);

		/*while (true) {
			vector<HANDLE> stopEvents;
			stopEvents.reserve(n_threads);

			for (MarkerControl& t : threads) {
				stopEvents.push_back(t.getStopEvent());
			}

			DWORD waitResult = WaitForMultipleObjects(
				n_threads,
				stopEvents.data(),
				TRUE,
				INFINITE
			);

			for (int a : arr) {
				cout << a << " ";
			}

			cout << endl << "Enter the sequence number of the marker thread that will be signaled to terminate its operation:" << endl;
			unsigned int id;
			cin >> id;
			threads[id - 1].stop();
			threads[id - 1].join();
			cout << "Joined marker " << id << endl;
		}*/
	}
	catch (const ios_base::failure& e) {
		cerr << "Input error: invalid format." << endl;

		return 1;
	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;

		return 1;
	}

	return 0;
}
