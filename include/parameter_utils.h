#pragma once
#include <string>

using std::string;

namespace parameter_utils {
string read_parameter();

string get_profile_parameter(const string &optimus_boot_cache);

bool is_valid_profile(const string &profile);
} // namespace parameter_utils
