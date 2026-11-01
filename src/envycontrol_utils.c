#include <stdbool.h>
#include <sys/stat.h>

#include <envycontrol_utils.h>
#define BLACKLIST_PATH "/etc/modprobe.d/blacklist-nvidia.conf"
#define UDEV_INTEGRATED_PATH "/etc/udev/rules.d/50-remove-nvidia.rules"
#define UDEV_INTEGRATED_LIB_PATH "/lib/udev/rules.d/50-remove-nvidia.rules"
#define XORG_PATH "/etc/X11/xorg.conf"
#define MODESET_PATH "/etc/modprobe.d/nvidia.conf"

bool file_exists(const char *path) {
  struct stat st;
  return (stat(path, &st) == 0);
}

// C replica of get_current_mode() from EnvyControl v3.5.2
const char *envycontrol_get_current_profile(void) {
  const char *mode = "hybrid";
  if (file_exists(BLACKLIST_PATH) && (file_exists(UDEV_INTEGRATED_PATH) ||
                                      file_exists(UDEV_INTEGRATED_LIB_PATH)))
    mode = "integrated";

  else if (file_exists(XORG_PATH) && file_exists(MODESET_PATH))
    mode = "nvidia";

  return mode;
}
