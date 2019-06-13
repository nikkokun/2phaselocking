#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <Row.h>

using namespace std;

void perform_transaction(vector<Row>& rows) {

  for(Row& row : rows) {
    row.lock.lock();
  }

  for(Row& row : rows) {
    int row_data = row.getData();
    row_data += 1;
    row.setData(row_data);
  }

  for(Row& row : rows) {
    row.lock.unlock();
  }

}

void worker(vector<Row>& table, int transactions, int operations, int t_id) {

  int size = table.size();
  vector<int> indices;
  for(int j = 0; j < size; ++j) {
    indices.push_back(j);
  }


  for(int i = 0; i < transactions; ++i) {


    auto rng = default_random_engine {};
    shuffle(indices.begin(), indices.end(), rng);

    vector<int> sorted_indices;
    vector<int> operation_indices;

    for(int k = 0; k < operations; ++k) {
      sorted_indices.push_back(indices[k]);
      operation_indices.push_back(indices[k]);
    }

    sort(sorted_indices.begin(), sorted_indices.end());

    cout << "Thread " << t_id << " operating on indices: [";

    for(int index : sorted_indices) {
      cout << index << ",";
    }

    cout << "]\n";


    for(int index : sorted_indices) {
      Row& row = table[index];
      row.lock.lock();
    }

    for(int index : operation_indices) {
      Row& row = table[index];
      int row_data = row.getData();
      row_data += 1;
      row.setData(row_data);
    }

    for(int index : sorted_indices) {
      Row& row = table[index];
      row.lock.unlock();
    }

  }

}

int main() {

  const int NUM_TRANSACTIONS = 1000000;
  const int NUM_ROWS = 1000000;
  const int NUM_OPERATIONS = 10;
  const int NUM_THREADS = 4;

  vector<thread> threads(NUM_THREADS);
  vector<Row> table;

  for(int i = 0; i < NUM_ROWS; ++i) {
    Row row = Row(i, 0);
    table.push_back(row);
  }

  cout << "Starting...\n";

  chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

  int thread_id = 0;

  for (thread& t : threads) {
    t = thread(worker, ref(table), NUM_TRANSACTIONS, NUM_OPERATIONS, thread_id);
    ++thread_id;
  }


  for (std::thread& th : threads) {
    th.join();
  }

  chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

  double time = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

  cout << "Finished after " << time << " nanoseconds \n";

}


