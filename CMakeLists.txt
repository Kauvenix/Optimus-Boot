cmake_minimum_required(VERSION 3.16)

project(optimus-boot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_compile_options(-Wall -Wextra)

include(GNUInstallDirs)

add_executable(optimus-boot
  src/main.cpp
  src/profile_manager.cpp
  src/command.cpp
  src/config.cpp
  src/parameter_utils.cpp
  src/backend.cpp
  src/envycontrol_utils.c
)

add_executable(optimus_boot_select_profile
  src/gpu_select_profile/gpu_select_profile.c
  src/envycontrol_utils.c
)

install(TARGETS optimus-boot optimus_boot_select_profile
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES src/optimus-boot.service
  DESTINATION /usr/lib/systemd/system
)

install(FILES src/gpu_select_profile/optimus-boot-select_profile.service
  DESTINATION /usr/lib/systemd/system
)
