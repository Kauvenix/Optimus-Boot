#include <fstream>
#include <map>
#include <string>

#include <command.h>
#include <config.h>

namespace Config {

bool save(const std::string &backend, const std::string &path) {
  std::ofstream file(CONFIG_FILE);

  if (!file)
    return false;

  file << "backend=" << backend << "\n";
  file << "backend_path=" << path << "\n";

  return true;
}

std::map<std::string, std::string> get_config() {

  std::ifstream file(CONFIG_FILE);

  std::map<std::string, std::string> config;

  std::string line;
  while (std::getline(file, line)) {
    auto pos = line.find('=');
    if (pos == std::string::npos)
      continue;

    config[line.substr(0, pos)] = line.substr(pos + 1);
  }

  return config;
}

bool update_config(const std::map<std::string, std::string> &config) {
  std::ofstream file(CONFIG_FILE);

  if (!file)
    return false;

  for (const auto &[key, value] : config)
    file << key << '=' << value << '\n';

  return true;
}

void default_setup() { save(backed_names[0], "/usr/local/bin/envycontrol"); }

bool is_valid_config_file() {
  if (!std::ifstream(Config::CONFIG_FILE))
    return false;

  auto file = Config::get_config();
  if (file["backend_path"] == "" || file["backend"] == "")
    return false;
  return true;
}
} // namespace Config
