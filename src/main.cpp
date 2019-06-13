#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <Row.h>

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



  for(int i = 0; i < num_transactions; ++i) {
    vector<int> operation_indices;

    for(int k = 0; k < num_operations; ++k) {
      operation_indices.push_back(distribution(eng));
    }

    random_indices.push_back(operation_indices);
  }

  return random_indices;
}

vector<vector<int>> sort_random_transactions(vector<vector<int>> random_transactions) {

  vector<vector<int>> sorted_transactions;
  for(int i = 0; i < random_transactions.size(); ++i) {
    vector<int> sorted_indices = random_transactions[i];
    sort(sorted_indices.begin(), sorted_indices.end());
    sorted_transactions.push_back(sorted_indices);
  }

  return sorted_transactions;

}

void worker(vector<Row>& table, vector<vector<int>> random_transactions) {

  vector<vector<int>> sorted_transactions = sort_random_transactions(random_transactions);

  cout << "Finished sorting \n";


  for(int i = 0; i < sorted_transactions.size(); ++i) {

    vector<int> sorted_transaction = sorted_transactions[i];
    vector<int> random_transaction = random_transactions[i];

    sorted_transaction.erase(unique(sorted_transaction.begin(), sorted_transaction.end()), sorted_transaction.end());

    for(int index : sorted_transaction) {
      Row& row = table[index];
      row.lock.lock();
    }

    for(int index : random_transaction) {
      Row& row = table[index];
      int row_data = row.getData();
      row_data += 1;
      row.setData(row_data);
    }

    for(int index : sorted_transaction) {
      Row& row = table[index];
      row.lock.unlock();
    }
  }
}



int main() {

  const int NUM_TRANSACTIONS = 1000000;
  const int NUM_ROWS = 1000000;
  const int NUM_OPERATIONS = 20;
  const int NUM_THREADS = 4;

  vector<thread> threads(NUM_THREADS);
  vector<Row> table;
  vector<int> indices;
  vector<vector<vector<int>>> thread_transactions;


  for(int i = 0; i < NUM_ROWS; ++i) {
    Row row = Row(i, 0);
    table.push_back(row);
  }

  u_long size = table.size();

  for(int i = 0; i < size; ++i) {
    indices.push_back(i);
  }

  for(int i = 0; i < NUM_THREADS; ++i) {
    thread_transactions.push_back(generate_random_transactions(indices, NUM_TRANSACTIONS, NUM_OPERATIONS));
  }


  cout << "Starting...\n";

  chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

  int thread_id = 0;

  for (thread& t : threads) {
    t = thread(worker, ref(table), thread_transactions[thread_id]);
    ++thread_id;
  }


  for (std::thread& th : threads) {
    th.join();
  }

  chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

  double time = chrono::duration_cast<chrono::microseconds>(end - start).count();

  cout << "Finished after " << time << " nanoseconds \n";

}


