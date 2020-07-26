#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <string>
#include <atomic>

using namespace std;

#define NUM_OPERATIONS 10
#define NUM_TRANSACTIONS 8388608
#define NUM_RECORDS 1000000
#define NUM_RUNS 10

struct atomic_mutex {
    atomic<int> val{0};
};

bool lock(atomic_mutex *lock) {
    int expected = 0;
    bool is_locked = lock->val.compare_exchange_strong(expected, 1);
    return is_locked;
}

void unlock(atomic_mutex *lock) {
    lock->val.exchange(0);
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

void worker(vector<int> &table, vector<atomic_mutex> &lock_table, vector<vector<int>> &random_transactions, const int stride, const int tid) {
    int start = tid * stride;
    int end = (start + stride) - 1;

    //sort phase
    for(int i = start; i < end; ++i) {
        insertion_sort(random_transactions[i], NUM_OPERATIONS);
    }
    //process transactions
    for (int i = start; i < end; ++i) {
        int locked_indices[NUM_OPERATIONS];
        memset(locked_indices, -1, sizeof(locked_indices));
        bool is_abort = false;
        int prev = -1;

        //growing phase
        for (int j = 0; j < NUM_OPERATIONS; ++j) {
            int idx = random_transactions[i][j];
            if (idx != prev) {
                bool is_acquired = lock(&lock_table[idx]);
                if (!is_acquired) {
                    is_abort = true;
                    break;
                } else {
                    locked_indices[j] = idx;
                }
            }
            prev = idx;
        }

        if (is_abort == 0) {
            //critical section
            for (int j = 0; j < NUM_OPERATIONS; ++j) {
                int idx = locked_indices[j];
                if (idx != -1) {
                    ++table[idx];
                }
            }
        }

        for (int j = 0; j < NUM_OPERATIONS; ++j) {
            int idx = locked_indices[j];
            if (idx != -1) {
                unlock(&lock_table[idx]);
            }
        }
    }
}

int main() {
    string OUTPUT_FP = "/home/nicoroble/2phaselocking/data/transactions_readsetsize-" + to_string(NUM_OPERATIONS) + "_transactions-" + to_string(NUM_TRANSACTIONS) + "_tablesize-" + to_string(NUM_RECORDS) + ".tsv";

    const vector<int> NUM_THREAD_VEC= {1,2,4,8,16,32,64,128};

    for(int x = 0; x < NUM_THREAD_VEC.size(); ++x) {
        const int NUM_THREADS = NUM_THREAD_VEC[x];
        const int STRIDE = NUM_TRANSACTIONS / NUM_THREADS;
        vector<double> runtimes;

        for(int n = 0; n < NUM_RUNS; ++n) {
            int thread_id = 0;
            double time;

            vector<thread> threads(NUM_THREADS);
            vector<int> table(NUM_RECORDS, 0);
            vector<atomic_mutex> lock_table(NUM_RECORDS);
            vector<vector<int>> random_transactions;

            random_device rd;
            mt19937 eng(rd());
            uniform_int_distribution<> distribution(0, NUM_RECORDS - 1);


            for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
                vector<int> operation_indices(NUM_OPERATIONS);
                for (int j = 0; j < NUM_OPERATIONS; ++j) {
                    operation_indices[j] = distribution(eng);
                }
                random_transactions.push_back(operation_indices);
            }

            chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

            for (thread &t : threads) {
                t = thread(worker, ref(table), ref(lock_table), ref(random_transactions), STRIDE, thread_id);
                ++thread_id;
            }

            for (std::thread &th : threads) {
                th.join();
            }

            chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

            time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            runtimes.push_back(time);
        }
        ofstream outfile;
        outfile.open(OUTPUT_FP, ios_base::app);
        float average_runtime = accumulate(runtimes.begin(), runtimes.end(), 0.0) / runtimes.size();
        outfile << NUM_THREADS << "\t" << average_runtime  << "\n";
        outfile.close();
    }
}


