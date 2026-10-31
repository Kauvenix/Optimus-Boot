# Changelog

All notable changes to this project are documented in this file.

## [1.1] - 2026-10-30

### Fixed
- Boot menu not displaying on systems that use Plymouth (boot splash animation). The TUI did not load correctly over the splash screen.
- Config file (`/etc/optimus-boot.conf`) not being created correctly, which forced users to set it up manually. The config is now validated on startup, and an invalid or incomplete one (missing or empty `backend` / `backend_path`) is regenerated automatically.
- README: corrected the `journalctl` command in the support section (each unit needs its own `-u`).

### Changed
- The boot menu is now redrawn periodically (every ~2.5 seconds) while waiting for input, as an extra safeguard against the splash screen covering it.
- README: EnvyControl is now installed system-wide with `sudo pipx install --global`, so it is available at boot. This improves compatibility with distros other than Arch.
- Tested on Solus Linux, in addition to EndeavourOS.

## [1.0]

- Initial release.
