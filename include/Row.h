#ifndef INC_2PHASELOCKING_ROW_H
#define INC_2PHASELOCKING_ROW_H
#include <assert.h>
#include "Attribute.h"

using namespace std;

class Row {
 public:
  vector<Attribute> attributes;

  Row(int size) {
    for(int i = 0; i < size; ++i) {
      Attribute attribute;
      attribute.data = 0;
      attributes.push_back(attribute);
    }
  }

  void print_row() {
    for(Attribute& attribute:attributes) {
      cout << attribute.data << " ";
    }
    cout << "\n";
  }

  void assert_attributes(int value) {
    for(Attribute& attribute:attributes) {
      assert(attribute.data == value);
    }
  }



};

#endif //INC_2PHASELOCKING_ROW_H
