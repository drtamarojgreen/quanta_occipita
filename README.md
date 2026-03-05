# QuantaOccipita: QuantaSoft Module Bootstrapper

QuantaOccipita is a developer tool designed to bootstrap and accelerate the development of new QuantaSoft modules. It generates standardized boilerplate code, configuration files, and documentation to ensure consistency and adherence to project standards.

## Features

- **Boilerplate Code Generation**: Generates starter source files (`.cpp`, `.h`) with module skeletons based on the QuantaSynapse template style.
- **Configuration File Initialization**: Creates a default YAML config file (`config.yaml`) with safe example values and explanatory comments.
- **Git Integration**: Prompts the user to initialize a git repository, commit the generated files, and push to a remote branch.
- **Issue Templates**: Generates `.github/ISSUE_TEMPLATE/` with templates for bug reports, feature requests, and ethical concerns.
- **Unit Test Skeletons**: Creates a `test/` directory with minimal test files using Catch2.
- **Documentation Indexing**: Updates the central `README.md` to index the new module's documentation.
- **Persistent Configuration**: Saves author identity and defaults in `.quanta_config`.
- **Timestamped Directories**: Option to append timestamps to module folders.
- **Dry Run Mode**: Preview actions without making changes.

## Requirements

- [C++17 Compiler](https://gcc.gnu.org/) (e.g., g++)
- [Make](https://www.gnu.org/software/make/)
- [Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)

## Building (C++ Implementation)

To build the C++ version of QuantaOccipita:

```bash
make
```

This will produce the `quanta_occipita` executable.

## Usage

### C++ Application

```bash
./quanta_occipita --module "QuantaCortex"
```

Common options:
- `--no-tests`: Skip test directory generation.
- `--dry-run`: Preview changes.
- `--license mit`: Specify license (mit, apache2, gpl3).
- `--author "Name"`: Set author name.

### Legacy PowerShell Script

The original PowerShell implementation is still available for environments with PowerShell support.

```powershell
./QuantaOccipita.ps1
```

## Contributing

We welcome contributions! Please see our issue templates in the `.github/ISSUE_TEMPLATE` directory to get started.

## Future Enhancements

For a full list of planned enhancements and future roadmap, please see the [Enhancement Opportunities document](./docs/enhancements.md).
