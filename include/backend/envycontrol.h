#pragma once

#include <string>
using std::string;
class Envycontrol {
public:
  virtual string get_current_profile();
  virtual int set_profile(string parameter_value);
};
