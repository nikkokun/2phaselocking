#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <row.h>

const int NUM_OPERATIONS = 20;
const int NUM_TRANSACTIONS = 4000000;
const int NUM_RECORDS = 1000000;
const int NUM_THREADS = 8;
const int STRIDE = NUM_TRANSACTIONS / NUM_THREADS;

using namespace std;


vector<vector<int>> generate_random_transactions(vector<int> main_index,
                                                 int num_transactions,
                                                 int num_operations) {

    vector<vector<int>> random_indices;

    auto min = min_element(begin(main_index), end(main_index));
    auto max = max_element(begin(main_index), end(main_index));

    random_device rd;
    mt19937 eng(rd());
    uniform_int_distribution<> distribution(*min, *max);

    for (int i = 0; i < num_transactions; ++i) {
        vector<int> operation_indices;
        for (int k = 0; k < num_operations; ++k) {
            operation_indices.push_back(distribution(eng));
        }
        random_indices.push_back(operation_indices);
    }

    return random_indices;
}

void insertion_sort(vector<int> &arr, int n) {
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


void worker(vector<Row> &table, vector<vector<int>> &random_transactions, const int stride, const int tid) {
    int start = tid * stride;
    int end = (start + stride) - 1;

    for(int i = start; i < end; ++i) {
        insertion_sort(random_transactions[i], NUM_OPERATIONS);
    }

    for (int i = start; i < end; ++i) {

        vector<int> sorted_transaction = random_transactions[i];

        sorted_transaction.erase(unique(sorted_transaction.begin(), sorted_transaction.end()),
                                 sorted_transaction.end());
        vector<int> locked_indices;
        bool is_abort = false;

        for (int index : sorted_transaction) {
            Row &row = table[index];
            bool is_acquired = row.lock.try_lock();
            if (!is_acquired) {
                is_abort = true;
                break;
            } else {
                locked_indices.push_back(index);
            }
        }
        if (!is_abort) {
            for (int index : sorted_transaction) {
                Row &row = table[index];
                int row_data = row.getData();
                row_data += 1;
                row.setData(row_data);
            }
        }
        for (int index : locked_indices) {
            Row &row = table[index];
            row.lock.unlock();
        }
    }

}

int main() {


    int thread_id = 0;
    double time;

    vector<thread> threads(NUM_THREADS);
    vector<Row> table;
    vector<int> indices;

    for (int i = 0; i < NUM_RECORDS; ++i) {
        Row row = Row(i, 0);
        table.push_back(row);
    }


    for (int i = 0; i < table.size(); ++i) {
        indices.push_back(i);
    }

    vector<vector<int>> random_transactions = generate_random_transactions(indices, NUM_TRANSACTIONS, NUM_OPERATIONS);

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    for (thread &t : threads) {
        t = thread(worker, ref(table), ref(random_transactions), STRIDE, thread_id);
        ++thread_id;
    }

    for (std::thread &th : threads) {
        th.join();
    }

    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Finished after " << time << " milliseconds \n";

}


