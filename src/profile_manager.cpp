#include <iostream>

#include "command.h"
#include "envycontrol_utils.h"
#include <backend.h>
#include <parameter_utils.h>
#include <profile_manager.h>

using std::string;

namespace ProfileManager {
// AI assistance was used for debugging and comments.
// The implementation was written and reviewed by the author.
int run() {
  std::cout << "Starting Optimus-Boot...\n"
            << "Reading selected GPU profile...\n";
  const string optimus_boot_cache = parameter_utils::read_parameter();
  if (optimus_boot_cache.empty()) {
    std::cerr << "Failed to read /run/optimus-boot.txt.\n";
    return 1;
  }

  std::cout << "Searching for gpu_profile parameter...\n";
  const string parameter_value =
      parameter_utils::get_profile_parameter(optimus_boot_cache);
  if (parameter_value.empty()) {
    std::cerr << "No gpu_profile parameter found. Nothing to do.\n";
    return 0;
  }
  std::cout << "Requested GPU profile: " << parameter_value
            << "\nValidating requested profile...\n";
  if (!parameter_utils::is_valid_profile(parameter_value)) {
    std::cerr << "Invalid GPU profile: " << parameter_value << std::endl;
    return 1;
  }

  if (parameter_value == "current") {
    return 0;
  }

  std::cout << "Reading current GPU profile...\n";
  string current_profile = envycontrol_get_current_profile();
  std::cout << "Current GPU profile: " << current_profile << '\n';

  if (current_profile == parameter_value) {
    std::cout << "Requested profile is already active.\n";
    return 0;
  }
  std::cout << "Applying GPU profile: " << parameter_value << '\n';
  if (Backend::set_profile(parameter_value) != 0) {
    std::cerr << "Failed to update GPU profile.\n";
    return 1;
  }
  std::cout << "GPU profile updated successfully.\n"
            << "Reboot required to apply changes.\n";
  string cache;
  std::cout << "Requesting system reboot...\n";
  if (Command::start_process("systemctl", {"reboot"}, cache) != 0) {
    std::cerr << "Failed to reboot the system.\n";
    return 1;
  }

  return 0;
}
} // namespace ProfileManager
