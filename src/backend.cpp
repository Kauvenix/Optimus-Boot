#include "config.h"
#include <command.h>
#include <envycontrol_utils.h>
#include <string>
using std::string;

namespace Backend {
int set_profile(string parameter_value) {
  string output;
  return Command::start_process(Config::get_config()["backend_path"].c_str(),
                                {"-s", parameter_value.c_str()}, output);
}
} // namespace Backend
