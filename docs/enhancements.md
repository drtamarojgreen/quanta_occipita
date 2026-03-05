# 🌱 Module Enhancement Opportunities

## Core Status
- The QuantaOccipita tool has been successfully ported to C++ (v1.1.0), implementing a wide range of previously requested features and architectural improvements.

## UX or CLI Improvements
- [X] Add command-line flags to selectively enable or disable features (e.g., `--no-tests`).
- [X] Implement a `--dry-run` mode.
- [X] Add an interactive mode for user prompts.
- [X] Persistent configuration via `.quanta_config`.
- [X] Support for timestamped directories.

## Resilience Features
- [X] Robust checks to prevent overwriting existing files without `--force`.
- [X] Dependency verification for Git.
- [X] Detailed action logging and optional report generation.

---

## Community-Suggested Enhancements (Batch 1 Status)

### CLI & User Experience (UX)
1.  [X] Add a `--version` flag.
2.  [X] Implement a `--help` command.
3.  [X] Add a `--quiet` / `--silent` mode.
4.  [X] Implement color-coded output.
5.  [X] Add a confirmation prompt before generating files in a non-empty directory.
6.  [X] Display a summary before proceeding (interactive or info log).
9.  [X] Add a `--no-banner` flag.
10. [X] Remember the last-used settings in `.quanta_config`.
11. [X] Add a `--report` flag for action summaries.
15. [X] Add a `--timestamp` flag.

### Code Generation (C++ & Boilerplate)
16. [X] Option for header guards (`--header-guard`) vs `#pragma once`.
17. [X] Allow specifying the C++ standard version (e.g., 11, 14, 17, 20) to be included in comments or build scripts.
18. [X] Generate a basic `Doxyfile` for Doxygen documentation.
19. [X] Include a sample `.clang-format` file with default styling rules.
24. [X] Placeholder for namespaces in generated files.
25. [X] Generate a `.editorconfig` file.
27. [X] Include a pre-filled `.gitignore`.
29. [X] Generate a simple `CMakeLists.txt` file for the module.
31. [X] Include a basic `LICENSE` file based on user choice.
35. [X] Include comment blocks for license/copyright.

### Configuration (YAML)
36. [X] Add a `--no-config` flag.
37. [X] Validate module names via regex.
38. [X] Allow custom output paths (`--output`).
41. [X] Add comments to `config.yaml`.
43. [X] Metadata support (author, email, version).

### Git Integration
46. [X] Custom initial commit message (`--message`).
48. [X] Specify initial branch name (`--branch`).
51. [X] Check for existing git repo.
55. [X] Allow custom remote setup.

### Documentation Generation
56. [X] Module-specific `README.md`.
60. [X] Skip documentation generation (`--no-docs`).

### Testing (Catch2 Skeletons)
76. [X] Option to skip `test/` directory (`--no-tests`).
83. [X] Test file naming convention based on module name.
84. [X] Simple bash script (`run_tests.sh`) to build and run tests.
85. [X] Pre-filled Catch2 main macro.

---

## Future Roadmap (Next Phases)
- [ ] Add support for more programming languages (Python, Rust, Go).
- [ ] Allow users to provide custom templates.
- [ ] Federated integration across machines.
- [ ] Plugin system for extending functionality.
