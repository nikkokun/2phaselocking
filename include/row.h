#ifndef INC_2PHASELOCKING_ROW_H
#define INC_2PHASELOCKING_ROW_H

#include <mutex>

using namespace std;

class Row {

 private:
  int key;
  int data;


 public:

  mutable mutex lock;

  int getKey() {
    return this->key;
  }

  void setKey(int key) {
    this->key = key;
  }

  int getData() {
    return this->data;
  }

  void setData(int d) {
    this->data = d;
  }

  Row(Row const &row) {
    this->key = row.key;
    this->data = row.data;
  }

  Row(int k, int d) {
    this->key = k;
    this->data = d;
  }

};

#endif //INC_2PHASELOCKING_ROW_H
