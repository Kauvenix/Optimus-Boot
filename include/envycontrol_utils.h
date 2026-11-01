
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
    #endif

    bool file_exists(const char *path);

    const char *envycontrol_get_current_profile(void);

    #ifdef __cplusplus
}
#endif
