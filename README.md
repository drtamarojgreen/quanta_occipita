# QuantaOccipita: QuantaSoft Module Bootstrapper

QuantaOccipita is a developer tool designed to bootstrap and accelerate the development of new QuantaSoft modules. It generates standardized boilerplate code, configuration files, and documentation to ensure consistency and adherence to project standards.

## Features

- **Boilerplate Code Generation**: Generates starter source files (`.cpp`, `.h`) with module skeletons based on the QuantaSynapse template style.
- **Configuration File Initialization**: Creates a default YAML config file (`config.yaml`) with safe example values and explanatory comments.
- **Git Integration**: Prompts the user to initialize a git repository, commit the generated files, and push to a remote branch.
- **Issue Templates**: Generates `.github/ISSUE_TEMPLATE/` with templates for bug reports, feature requests, and ethical concerns.
- **Unit Test Skeletons**: Creates a `test/` directory with minimal test files using Catch2.
- **Documentation Indexing**: Updates the central `README.md` to index the new module's documentation.

## Requirements

- [PowerShell](https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell)
- [Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)

## Usage

To use QuantaOccipita, run the `QuantaOccipita.ps1` script from your terminal.

```powershell
./QuantaOccipita.ps1
```

The script will guide you through the process of creating a new module.

## Contributing

We welcome contributions! Please see our issue templates in the `.github/ISSUE_TEMPLATE` directory to get started.

## Future Enhancements

For a full list of planned enhancements and future roadmap, please see the [Enhancement Opportunities document](./docs/enhancements.md).