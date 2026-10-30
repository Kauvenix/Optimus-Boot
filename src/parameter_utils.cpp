#include <cctype>
#include <fstream>
#include <parameter_utils.h>
#include <string>
#include <string_view>

using std::getline;
using std::ifstream;
using std::string;

namespace parameter_utils {
string read_parameter() {
  std::ifstream archive("/run/optimus-boot.txt");
  if (!archive.is_open()) {
    return {};
  }
  string optimus_boot_cache;
  getline(archive, optimus_boot_cache);
  return optimus_boot_cache;
}

string get_profile_parameter(const string &optimus_boot_cache) {
  constexpr std::string_view key_parameter = "gpu_profile=";

  auto start_index = optimus_boot_cache.find(key_parameter);
  if (start_index == string::npos) {
    return {};
  }
  start_index += key_parameter.length();

  auto end_index = optimus_boot_cache.find(" ", start_index);
  if (end_index == string::npos) {
    end_index = optimus_boot_cache.length();
  }

  const auto length_parameter = end_index - start_index;

  string value = optimus_boot_cache.substr(start_index, length_parameter);

  for (char &character : value) { // tolower value
    character = std::tolower(static_cast<unsigned char>(character));
  }
  return value;
}

bool is_valid_profile(const string &profile) {
  return profile == "integrated" || profile == "hybrid" ||
         profile == "nvidia" || profile == "current";
}
} // namespace parameter_utils
