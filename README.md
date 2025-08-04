Extended QuantaOccipita Capabilities
1. Boilerplate Code Generation

    Generate starter source files (.cpp, .h) with module skeletons based on the QuantaSynapse template style.

    Include standard includes, namespace, and placeholders for key methods.

2. Configuration File Initialization

    Create a default YAML config file (config.yaml) with safe example values.

    Add comments for each config field referencing the ethical and operational priorities.

3. Git Commit & Push Prompt

    After creating files, prompt the user to optionally initialize git repo (if not already).

    Add all files, commit with a standard message, and push to the specified remote branch (e.g., quanta_<modulename>).

4. Issue Templates

    Generate .github/ISSUE_TEMPLATE/ directory with markdown templates for:

        Bug reports

        Feature requests

        Ethical concerns

5. Unit Test Skeletons

    Create a test directory with minimal test files using a chosen framework (e.g., Catch2 for C++).

    Include placeholder tests to verify basic module behaviors.

6. Documentation Indexing

    Append or create a central README.md index in the repo root.

    Add entries linking to the new module's plan.md, enhancements.md, and code docs.