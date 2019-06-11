#ifndef INC_2PHASELOCKING_ATTRIBUTE_H
#define INC_2PHASELOCKING_ATTRIBUTE_H

#include <mutex>

struct Attribute {
  int data;
  mutable std::mutex lock;

  Attribute() = default;

  Attribute(Attribute const&c) {
    std::lock_guard<std::mutex> (c.lock);
    data = c.data;
  }

};

typedef Attribute Attribute;

#endif //INC_2PHASELOCKING_ATTRIBUTE_H
