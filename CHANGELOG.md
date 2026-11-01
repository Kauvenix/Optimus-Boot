# Changelog

All notable changes to this project are documented in this file.

## [v1.1.2] - 2026-11-01

### Fixed
- Installation script missing `sudo cmake --install release`, which prevented automatic installation of binaries and systemd units. The install process now properly installs all components after building.


## [v1.1.1] - 2026-10-31

### Changed
- The current GPU profile is now detected natively in C (a replica of EnvyControl v3.5.2's `get_current_mode()`) instead of running `envycontrol --query`. The same code is shared by the main program and the boot menu (`optimus_boot_select_profile`).
- Removed the class-based backend design (the `Envycontrol` class and its virtual methods) in favor of plain functions, resulting in simpler code, fewer abstractions and less overhead.
- Both executables (`optimus-boot` and `optimus_boot_select_profile`) are now built by CMake, which also handles installing the binaries and the systemd units. `install.sh` was simplified accordingly.
- Enabled `-Wall -Wextra` compiler warnings.

## [v1.1.0] - 2026-10-30

### Fixed
- Boot menu not displaying on systems that use Plymouth (boot splash animation). The TUI did not load correctly over the splash screen.
- Config file (`/etc/optimus-boot.conf`) not being created correctly, which forced users to set it up manually. The config is now validated on startup, and an invalid or incomplete one (missing or empty `backend` / `backend_path`) is regenerated automatically.
- README: corrected the `journalctl` command in the support section (each unit needs its own `-u`).

### Changed
- The boot menu is now redrawn periodically (every ~2.5 seconds) while waiting for input, as an extra safeguard against the splash screen covering it.
- README: EnvyControl is now installed system-wide with `sudo pipx install --global`, so it is available at boot. This improves compatibility with distros other than Arch.
- Tested on Solus Linux, in addition to EndeavourOS.

## [v1.0.0]

- Initial release.
