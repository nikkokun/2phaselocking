#include <iostream>
#include <thread>
#include <vector>
#include "Attribute.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <Row.h>

using namespace std;

void perform_transaction(vector<Attribute>& v_data) {

  for(Attribute& element : v_data) {
    element.lock.lock();
  }

  for(Attribute& element : v_data) {
    int element_data = element.data;
    element_data += 1;
    element.data = element_data;
  }

  for(Attribute& element : v_data) {
    element.lock.unlock();
  }

}

void worker(vector<Row>& table) {



  int size = table.size();
  vector<int> indices;
  for(int i = 0; i < size; ++i) {
    indices.push_back(i);
  }

  auto rng = default_random_engine {};
  shuffle(indices.begin(), indices.end(), rng);

  for(int i : indices) {

    vector<Attribute>& v_data = table[i].attributes;

    perform_transaction(table[i].attributes);

  }

}

int main() {



  const int num_rows = 1000000;
  const int num_attributes = 20;
  const int num_threads = 4;
  vector<thread> threads(num_threads);
  vector<Row> table;

  for(int i = 0; i < num_rows; ++i) {
    Row row = Row(num_attributes);
    table.push_back(row);
  }

  cout << "Starting...\n";

  chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

  for (thread& t : threads) {
    t = thread(worker, ref(table));
  }

  for (std::thread& th : threads) {
    th.join();
  }

  chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

  double time = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

  cout << "Finished after " << time << " nanoseconds \n";



  cout << "Asserting all values are correct...\n";
  for(Row& row : table) {
    row.assert_attributes(num_threads);
  }
  cout << "All values are correct\n";
}


