#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

struct Data {
  int data;
  mutex lock;
};

void worker(vector<Data>& v_data) {

    for(Data& element : v_data) {
        element.lock.lock();
    }

    for(Data& element : v_data) {
        element.data += 1;
    }

    for(Data& element : v_data) {
        element.lock.unlock();
    }

}

int main() {

    vector<thread> threads(4);
    vector<Data> v_data(10);

    cout << "Start Value: ";
    for(Data& element : v_data) {
        element.data = 0;
        cout << element.data << " ";
    }
    cout << '\n';

    for (thread& t : threads) {
        t = thread(worker, ref(v_data));
    }

    for (std::thread& th : threads) {
        th.join();
    }

    cout << "End Value: ";
    for(Data& element : v_data) {
        cout << element.data << " ";
    }
    cout << '\n';
}