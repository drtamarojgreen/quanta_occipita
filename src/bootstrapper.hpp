#pragma once

#include "cli.hpp"
#include "console.hpp"
#include "licenses.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <functional>
#include <stdexcept>

namespace fs = std::filesystem;

// ─── Bootstrapper ─────────────────────────────────────────────────────────────

class Bootstrapper {
public:
    explicit Bootstrapper(const Config& cfg) : cfg_(cfg) {}

    void run();

private:
    const Config& cfg_;

    // action log for --report
    std::vector<std::string> log_;

    // ── Helpers ────────────────────────────────────────────────────────────
    fs::path rootPath() const;
    std::string currentYear() const;
    std::string timestamp() const;
    std::string authorDisplay() const;

    void makeDir(const fs::path& p);
    void writeFile(const fs::path& p, const std::string& content);
    void action(const std::string& msg, Console::Color c = Console::Color::Green);

    // ── Feature generators ─────────────────────────────────────────────────
    void genSourceFiles(const fs::path& root);
    void genConfigYaml(const fs::path& root);
    void genConfigJson(const fs::path& root);
    void genDocs(const fs::path& root);
    void genIssueTemplates(const fs::path& root);
    void genTests(const fs::path& root);
    void genRootReadme(const fs::path& root);
    void genLicense(const fs::path& root);
    void genGitignore(const fs::path& root);
    void genChangelog(const fs::path& root);
    void genEditorconfig(const fs::path& root);
    void genClangFormat(const fs::path& root);
    void genCMakeLists(const fs::path& root);
    void genDoxyfile(const fs::path& root);
    void handleGit(const fs::path& root);
    void writeReport(const fs::path& root);
    void listTemplates() const;
};

// ═════════════════════════════════════════════════════════════════════════════
//  Implementation
// ═════════════════════════════════════════════════════════════════════════════

inline void Bootstrapper::run() {
    if (cfg_.listTemplates) { listTemplates(); return; }

    fs::path root = rootPath();

    if (!cfg_.dryRun) {
        if (fs::exists(root) && !cfg_.force) {
            // Check non-empty
            bool nonEmpty = false;
            if (fs::is_directory(root)) {
                for ([[maybe_unused]] auto& e : fs::directory_iterator(root)) { nonEmpty = true; break; }
            }
            if (nonEmpty) {
                if (cfg_.quiet) {
                    Console::error("Directory '" + root.string() + "' is non-empty. Use --force to proceed.");
                    return;
                }
                std::cout << "\033[33m[!] Directory '" << root.string() << "' is not empty. Continue? (Y/N): \033[0m";
                std::string response;
                std::getline(std::cin, response);
                if (response != "Y" && response != "y") {
                    Console::info("Aborted by user.");
                    return;
                }
            }
        }
        makeDir(root);
    } else {
        action("Would create root: " + root.string(), Console::Color::Yellow);
    }

    if (!cfg_.quiet) Console::info("Bootstrapping module: " + cfg_.moduleName);

    genSourceFiles(root);
    if (!cfg_.noConfig) {
        if (cfg_.jsonConfig) genConfigJson(root);
        else genConfigYaml(root);
    }
    if (!cfg_.noDocs)   genDocs(root);
    genIssueTemplates(root);
    if (!cfg_.noTests)  genTests(root);
    genRootReadme(root);
    if (cfg_.license != "none") genLicense(root);
    genGitignore(root);
    genChangelog(root);
    genEditorconfig(root);
    genClangFormat(root);
    genCMakeLists(root);
    genDoxyfile(root);
    if (!cfg_.skipGit)  handleGit(root);
    if (cfg_.report)    writeReport(root);
}

// ─── Path & Utilities ─────────────────────────────────────────────────────────

inline fs::path Bootstrapper::rootPath() const {
    std::string name = cfg_.moduleName;
    if (cfg_.timestamp) name += "_" + timestamp();
    if (cfg_.flatStructure) return fs::path(cfg_.outputPath);
    return fs::path(cfg_.outputPath) / name;
}

inline std::string Bootstrapper::currentYear() const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buf[8]; std::strftime(buf, sizeof(buf), "%Y", &tm);
    return buf;
}

inline std::string Bootstrapper::timestamp() const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buf[16]; std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
    return buf;
}

inline std::string Bootstrapper::authorDisplay() const {
    if (!cfg_.authorName.empty()) return cfg_.authorName;
    return cfg_.moduleName + " Authors";
}

inline void Bootstrapper::makeDir(const fs::path& p) {
    if (!fs::exists(p)) {
        fs::create_directories(p);
        action("Created directory: " + p.string());
    }
}

inline void Bootstrapper::writeFile(const fs::path& p, const std::string& content) {
    if (cfg_.dryRun) {
        Console::print("  [dry-run] Would write: " + p.string(), Console::Color::Yellow);
        log_.push_back("[dry-run] " + p.string());
        return;
    }
    if (fs::exists(p) && !cfg_.force) {
        Console::warn("Skipping (exists): " + p.string() + "  (use --force to overwrite)");
        return;
    }
    // Ensure parent exists
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    if (!f) throw std::runtime_error("Cannot write: " + p.string());
    f << content;
    action("  [+] " + p.string());
    log_.push_back(p.string());
}

inline void Bootstrapper::action(const std::string& msg, Console::Color c) {
    if (!cfg_.quiet) Console::print(msg, c);
    log_.push_back(msg);
}

// ─── Source Files ─────────────────────────────────────────────────────────────

inline void Bootstrapper::genSourceFiles(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string guard    = M;
    for (auto& ch : guard) ch = std::toupper(ch);
    guard += "_HPP";

    std::string licHdr = Licenses::headerComment(cfg_.license, authorDisplay(), currentYear());

    // Header
    std::string h;
    h += licHdr;
    h += "\n";
    if (cfg_.headerGuard) {
        h += "#ifndef " + guard + "\n#define " + guard + "\n\n";
    } else {
        h += "#pragma once\n\n";
    }
    h += "#include <string>\n";
    if (cfg_.pimpl) h += "#include <memory>\n";
    h += "\n";
    h += "namespace " + M + " {\n\n";
    h += "/**\n * @brief Core class for the " + M + " module.\n *\n";
    h += " * Replace this placeholder with your module's actual interface.\n */\n";
    h += "class " + M + " {\npublic:\n";
    if (cfg_.singleton) {
        h += "    static " + M + "& instance();\n";
        h += "    " + M + "(const " + M + "&) = delete;\n";
        h += "    " + M + "& operator=(const " + M + "&) = delete;\n\n";
    } else {
        h += "    explicit " + M + "(const std::string& name);\n";
    }
    h += "    ~" + M + "() = default;\n\n";
    h += "    /** @brief Returns the module's identifier. */\n";
    h += "    std::string name() const;\n\n";
    h += "    /** @brief Primary entry point for module execution. */\n";
    h += "    void run();\n\n";
    h += "private:\n";
    if (cfg_.singleton) {
        h += "    " + M + "();\n";
    }
    if (cfg_.pimpl) {
        h += "    struct Impl;\n";
        h += "    std::unique_ptr<Impl> impl_;\n";
    } else {
        h += "    std::string name_;\n";
    }
    h += "};\n\n";
    h += "} // namespace " + M + "\n";
    if (cfg_.headerGuard) h += "\n#endif // " + guard + "\n";

    // Source
    std::string cpp;
    cpp += licHdr;
    cpp += "\n#include \"" + M + ".hpp\"\n#include <iostream>\n\n";
    cpp += "namespace " + M + " {\n\n";

    if (cfg_.pimpl) {
        cpp += "struct " + M + "::Impl {\n";
        cpp += "    std::string name;\n";
        cpp += "};\n\n";
    }

    if (cfg_.singleton) {
        cpp += M + "& " + M + "::instance() {\n";
        cpp += "    static " + M + " inst;\n";
        cpp += "    return inst;\n";
        cpp += "}\n\n";
        cpp += M + "::" + M + "() ";
        if (cfg_.pimpl) cpp += ": impl_(std::make_unique<Impl>(Impl{\"Singleton " + M + "\"}))";
        else cpp += ": name_(\"Singleton " + M + "\")";
        cpp += " {}\n\n";
    } else {
        cpp += M + "::" + M + "(const std::string& name) ";
        if (cfg_.pimpl) cpp += ": impl_(std::make_unique<Impl>(Impl{name}))";
        else cpp += ": name_(name)";
        cpp += " {}\n\n";
    }

    cpp += "std::string " + M + "::name() const { return ";
    if (cfg_.pimpl) cpp += "impl_->name";
    else cpp += "name_";
    cpp += "; }\n\n";

    cpp += "void " + M + "::run() {\n";
    cpp += "    // TODO: implement " + M + " logic\n";
    cpp += "    std::cout << \"[\" << name() << \"] running...\\n\";\n";
    cpp += "}\n\n";
    cpp += "} // namespace " + M + "\n";

    // Demo main
    std::string maincpp;
    maincpp += licHdr;
    maincpp += "\n// Demo: instantiate and run " + M + "\n";
    maincpp += "#include \"" + M + ".hpp\"\n\n";
    maincpp += "int main() {\n";
    if (cfg_.singleton) {
        maincpp += "    auto& module = " + M + "::" + M + "::instance();\n";
    } else {
        maincpp += "    " + M + "::" + M + " module(\"" + M + "\");\n";
    }
    maincpp += "    module.run();\n";
    maincpp += "    return 0;\n}\n";

    fs::path src = root / "src";
    writeFile(src / (M + ".hpp"),  h);
    writeFile(src / (M + ".cpp"),  cpp);
    writeFile(src / "main.cpp",    maincpp);
}

// ─── Config YAML ─────────────────────────────────────────────────────────────

inline void Bootstrapper::genConfigYaml(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string y;
    y += "# config.yaml — " + M + " module configuration\n";
    y += "# Generated by QuantaOccipita v" + std::string(QUANTA_VERSION) + "\n\n";
    y += "metadata:\n";
    y += "  module: \"" + M + "\"\n";
    y += "  version: \"0.1.0\"\n";
    y += "  author: \"" + authorDisplay() + "\"\n";
    if (!cfg_.authorEmail.empty())
        y += "  email: \"" + cfg_.authorEmail + "\"\n";
    y += "\n";
    y += "runtime:\n";
    y += "  log_level: \"info\"        # debug | info | warn | error\n";
    y += "  max_retries: 3\n";
    y += "  timeout_ms: 5000\n";
    y += "\n";
    y += "features:\n";
    y += "  enabled: true\n";
    y += "  # List of sub-feature flags\n";
    y += "  flags:\n";
    y += "    - name: \"feature_alpha\"\n";
    y += "      active: false\n";
    y += "    - name: \"feature_beta\"\n";
    y += "      active: true\n";
    y += "\n";
    y += "paths:\n";
    y += "  data: \"data/\"\n";
    y += "  output: \"output/\"\n";
    y += "  cache: \".cache/\"\n";

    writeFile(root / "config.yaml", y);
}

inline void Bootstrapper::genConfigJson(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string j;
    j += "{\n";
    j += "  \"metadata\": {\n";
    j += "    \"module\": \"" + M + "\",\n";
    j += "    \"version\": \"0.1.0\",\n";
    j += "    \"author\": \"" + authorDisplay() + "\"";
    if (!cfg_.authorEmail.empty())
        j += ",\n    \"email\": \"" + cfg_.authorEmail + "\"";
    j += "\n  },\n";
    j += "  \"runtime\": {\n";
    j += "    \"log_level\": \"info\",\n";
    j += "    \"max_retries\": 3,\n";
    j += "    \"timeout_ms\": 5000\n";
    j += "  },\n";
    j += "  \"features\": {\n";
    j += "    \"enabled\": true,\n";
    j += "    \"flags\": [\n";
    j += "      { \"name\": \"feature_alpha\", \"active\": false },\n";
    j += "      { \"name\": \"feature_beta\", \"active\": true }\n";
    j += "    ]\n";
    j += "  }\n";
    j += "}\n";

    writeFile(root / "config.json", j);
}

// ─── Docs ─────────────────────────────────────────────────────────────────────

inline void Bootstrapper::genDocs(const fs::path& root) {
    const std::string& M = cfg_.moduleName;

    // plan.md
    std::string plan;
    plan += "# 📘 Module Planning Document\n\n";
    plan += "## Module Name\n- " + M + "\n\n";
    plan += "## Purpose\n- Define the purpose of this module in the QuantaSoft system.\n- What role does it play within the agent ecosystem?\n\n";
    plan += "## Goals\n- [ ] Define inputs and outputs\n- [ ] Define triggers and dependencies\n- [ ] Establish ethical decision logic (if applicable)\n\n";
    plan += "## Integration Plan\n- How will this module communicate with:\n";
    plan += "  - 🧠 QuantaSensa\n  - 🕊️ QuantaEthos\n  - 🧬 QuantaSynapse\n  - 🧹 QuantaGlia\n\n";
    plan += "## Notes\n- Anticipated constraints:\n- Potential edge cases:\n";

    // enhancements.md
    std::string enh;
    enh += "# 🌱 Enhancement Opportunities — " + M + "\n\n";
    enh += "## UX Improvements\n- [ ] Streamline feedback or log formats\n- [ ] Add verbosity toggles\n\n";
    enh += "## Learning & Adaptation\n- [ ] Integrate new pattern recognition logic\n- [ ] Enable self-update from QuantaGlia knowledge\n\n";
    enh += "## Resilience Features\n- [ ] Better handling of low-resource environments\n- [ ] Soft fallback routines for unknown inputs\n\n";
    enh += "## Future Roadmap\n- [ ] Federated integration across machines\n- [ ] Test in embedded or containerized environments\n";

    // Module README
    std::string mdme;
    mdme += "# " + M + "\n\n";
    mdme += "> Part of the QuantaSoft ecosystem.\n\n";
    mdme += "## Overview\n\nDescribe what this module does.\n\n";
    mdme += "## Building\n\n```bash\ng++ -std=c++" + cfg_.cppStd + " -o " + M + " src/main.cpp src/" + M + ".cpp\n```\n\n";
    mdme += "## Running\n\n```bash\n./" + M + "\n```\n\n";
    mdme += "## Testing\n\n```bash\n# From the test/ directory\ng++ -std=c++" + cfg_.cppStd + " -o run_tests test/" + M + ".test.cpp && ./run_tests\n```\n\n";
    mdme += "## Configuration\n\nEdit `config.yaml` to adjust runtime behaviour.\n\n";
    mdme += "## License\n\nSee [LICENSE](LICENSE).\n";

    writeFile(root / "docs" / "plan.md",         plan);
    writeFile(root / "docs" / "enhancements.md", enh);
    writeFile(root / "README.md",                mdme);
}

// ─── GitHub Issue Templates ──────────────────────────────────────────────────

inline void Bootstrapper::genIssueTemplates(const fs::path& root) {
    fs::path tpl = root / ".github" / "ISSUE_TEMPLATE";

    std::string bug = R"(---
name: Bug Report
about: Report a bug in this module
labels: bug
---

## Description
A clear and concise description of the bug.

## Steps to Reproduce
1.
2.
3.

## Expected Behaviour
What did you expect?

## Actual Behaviour
What happened instead?

## Environment
- OS:
- Compiler:
- Version:
)";

    std::string feat = R"(---
name: Feature Request
about: Suggest a new feature
labels: enhancement
---

## Summary
A brief description of the feature.

## Motivation
Why is this feature needed?

## Proposed Solution


## Alternatives Considered

)";

    std::string ethical = R"(---
name: Ethical Concern
about: Raise an ethical concern about module behaviour
labels: ethics
---

## Concern
Describe the ethical concern clearly and concisely.

## Potential Impact
Who or what could be affected?

## Suggested Mitigation

)";

    writeFile(tpl / "bug_report.md",      bug);
    writeFile(tpl / "feature_request.md", feat);
    writeFile(tpl / "ethical_concern.md", ethical);
}

// ─── Tests ────────────────────────────────────────────────────────────────────

inline void Bootstrapper::genTests(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string test;
    test += "// " + M + " — Catch2 unit tests\n";
    test += "// Run: g++ -std=c++" + cfg_.cppStd + " -o run_tests " + M + ".test.cpp ../src/" + M + ".cpp && ./run_tests\n\n";
    test += "#define CATCH_CONFIG_MAIN\n";
    test += "#include \"catch2/catch.hpp\"\n";
    test += "#include \"../src/" + M + ".hpp\"\n\n";
    test += "TEST_CASE(\"" + M + " initialises correctly\", \"[" + M + "]\") {\n";
    if (cfg_.singleton) {
        test += "    auto& mod = " + M + "::" + M + "::instance();\n";
        test += "    REQUIRE(mod.name() == \"Singleton " + M + "\");\n";
    } else {
        test += "    " + M + "::" + M + " mod(\"test_module\");\n";
        test += "    REQUIRE(mod.name() == \"test_module\");\n";
    }
    test += "}\n\n";
    test += "TEST_CASE(\"" + M + " run() executes without exception\", \"[" + M + "]\") {\n";
    if (cfg_.singleton) {
        test += "    auto& mod = " + M + "::" + M + "::instance();\n";
    } else {
        test += "    " + M + "::" + M + " mod(\"test_module\");\n";
    }
    test += "    REQUIRE_NOTHROW(mod.run());\n";
    test += "}\n\n";
    test += "// TODO: Add domain-specific tests here\n";

    std::string runner = "#!/usr/bin/env bash\n"
        "set -e\n"
        "echo 'Building tests...'\n"
        "g++ -std=c++" + cfg_.cppStd + " -o run_tests " + M + ".test.cpp ../src/" + M + ".cpp\n"
        "echo 'Running tests...'\n"
        "./run_tests\n";

    writeFile(root / "test" / (M + ".test.cpp"), test);
    writeFile(root / "test" / "run_tests.sh",    runner);

    // gen test CMakeLists.txt
    std::string tc;
    tc += "cmake_minimum_required(VERSION 3.10)\n";
    tc += "project(" + M + "_tests LANGUAGES CXX)\n\n";
    tc += "set(CMAKE_CXX_STANDARD " + cfg_.cppStd + ")\n\n";
    tc += "add_executable(run_tests " + M + ".test.cpp ../src/" + M + ".cpp)\n";
    tc += "target_include_directories(run_tests PRIVATE ../src)\n";

    writeFile(root / "test" / "CMakeLists.txt", tc);
}

// ─── Root README ──────────────────────────────────────────────────────────────

inline void Bootstrapper::genRootReadme(const fs::path& root) {
    // Only update / create if NOT inside a flat structure where it's the only root
    // The module-specific README was already generated in genDocs; here we also
    // create a CONTRIBUTING.md and CODE_OF_CONDUCT.md.
    const std::string& M = cfg_.moduleName;

    std::string contrib = "# Contributing to " + M + "\n\n";
    contrib += "Thank you for your interest in contributing!\n\n";
    contrib += "## How to Contribute\n1. Fork the repository.\n2. Create a feature branch.\n3. Commit your changes.\n4. Open a pull request.\n\n";
    contrib += "## Code Style\nFollow the `.clang-format` and `.editorconfig` rules in this repo.\n\n";
    contrib += "## Reporting Issues\nPlease use the issue templates in `.github/ISSUE_TEMPLATE/`.\n";

    std::string coc = "# Code of Conduct\n\n";
    coc += "We follow the [Contributor Covenant](https://www.contributor-covenant.org/) v2.1.\n\n";
    coc += "In summary: be kind, inclusive, and constructive.\n\n";
    coc += "Unacceptable behaviour should be reported to the project maintainers.\n";

    writeFile(root / "CONTRIBUTING.md",    contrib);
    writeFile(root / "CODE_OF_CONDUCT.md", coc);
}

// ─── License ─────────────────────────────────────────────────────────────────

inline void Bootstrapper::genLicense(const fs::path& root) {
    std::string text = Licenses::getText(cfg_.license, authorDisplay(), currentYear());
    writeFile(root / "LICENSE", text);
}

// ─── .gitignore ──────────────────────────────────────────────────────────────

inline void Bootstrapper::genGitignore(const fs::path& root) {
    std::string g = R"(# Build
build/
*.o
*.a
*.so
*.out
run_tests

# Config cache
.quanta_config

# IDE
.vscode/
.idea/
*.swp
*~
.DS_Store

# Generated
quanta_report.txt
)";
    writeFile(root / ".gitignore", g);
}

// ─── CHANGELOG ───────────────────────────────────────────────────────────────

inline void Bootstrapper::genChangelog(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string cl;
    cl += "# Changelog\n\n";
    cl += "All notable changes to " + M + " will be documented here.\n\n";
    cl += "## [v0.1.0] - " + currentYear() + "\n\n";
    cl += "### Added\n- Initial module scaffold generated by QuantaOccipita v" + std::string(QUANTA_VERSION) + "\n";
    writeFile(root / "CHANGELOG.md", cl);
}

// ─── .editorconfig ───────────────────────────────────────────────────────────

inline void Bootstrapper::genEditorconfig(const fs::path& root) {
    std::string ec = R"(root = true

[*]
indent_style = space
indent_size = 4
end_of_line = lf
charset = utf-8
trim_trailing_whitespace = true
insert_final_newline = true

[*.md]
trim_trailing_whitespace = false

[Makefile]
indent_style = tab
)";
    writeFile(root / ".editorconfig", ec);
}

inline void Bootstrapper::genClangFormat(const fs::path& root) {
    std::string cf = R"(BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: Empty
)";
    writeFile(root / ".clang-format", cf);
}

inline void Bootstrapper::genCMakeLists(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string c;
    c += "cmake_minimum_required(VERSION 3.10)\n";
    c += "project(" + M + " VERSION 0.1.0 LANGUAGES CXX)\n\n";
    c += "set(CMAKE_CXX_STANDARD " + cfg_.cppStd + ")\n";
    c += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    c += "add_library(" + M + " src/" + M + ".cpp)\n";
    c += "target_include_directories(" + M + " PUBLIC src)\n\n";
    c += "add_executable(" + M + "_demo src/main.cpp)\n";
    c += "target_link_libraries(" + M + "_demo PRIVATE " + M + ")\n";

    writeFile(root / "CMakeLists.txt", c);
}

inline void Bootstrapper::genDoxyfile(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    std::string d;
    d += "PROJECT_NAME           = \"" + M + "\"\n";
    d += "PROJECT_NUMBER         = 0.1.0\n";
    d += "OUTPUT_DIRECTORY       = docs/doxygen\n";
    d += "INPUT                  = src README.md\n";
    d += "RECURSIVE              = YES\n";
    d += "GENERATE_LATEX         = NO\n";
    d += "USE_MDFILE_AS_MAINPAGE = README.md\n";

    writeFile(root / "Doxyfile", d);
}

// ─── Git ─────────────────────────────────────────────────────────────────────

inline void Bootstrapper::handleGit(const fs::path& root) {
    if (cfg_.dryRun) {
        Console::print("  [dry-run] Would: git init, add, commit, set branch to " + cfg_.initialBranch,
                       Console::Color::Yellow);
        return;
    }

    // Check if already a git repo
    bool isRepo = fs::exists(root / ".git");

    std::string response;
    if (!isRepo) {
        std::cout << "\033[36mInitialise a git repository in " << root.string() << "? (Y/N): \033[0m";
        std::getline(std::cin, response);
        if (response != "Y" && response != "y") return;
    } else {
        Console::info("Git repository already exists. Creating new commit on current branch.");
    }

    auto run = [&](const std::string& cmd) {
        int ret = std::system(("cd '" + root.string() + "' && " + cmd + " 2>/dev/null").c_str());
        return ret == 0;
    };

    if (!isRepo) {
        if (!run("git init")) { Console::error("git init failed."); return; }
        run("git checkout -b '" + cfg_.initialBranch + "'");
    }

    run("git add .");

    std::string msg = cfg_.commitMessage.empty()
        ? "chore: bootstrap " + cfg_.moduleName + " module via QuantaOccipita"
        : cfg_.commitMessage;

    if (!run("git commit -m '" + msg + "'")) {
        Console::warn("git commit failed (is git configured with user.email/user.name?)");
        return;
    }
    Console::success("Git repository initialised with initial commit.");

    std::cout << "\033[36mPush to remote? Enter remote URL (or press Enter to skip): \033[0m";
    std::string remote;
    std::getline(std::cin, remote);
    if (!remote.empty()) {
        run("git remote add origin '" + remote + "'");
        if (run("git push -u origin '" + cfg_.initialBranch + "'"))
            Console::success("Pushed to " + remote);
        else
            Console::warn("Push failed. You can push manually later.");
    }
}

// ─── Report ──────────────────────────────────────────────────────────────────

inline void Bootstrapper::writeReport(const fs::path& root) {
    if (cfg_.dryRun) {
        Console::print("  [dry-run] Would write quanta_report.txt", Console::Color::Yellow);
        return;
    }
    fs::path rp = root / "quanta_report.txt";
    std::ofstream f(rp);
    f << "QuantaOccipita Bootstrap Report\n";
    f << "Module  : " << cfg_.moduleName << "\n";
    f << "Version : " << QUANTA_VERSION << "\n";
    f << "Date    : " << currentYear() << "\n";
    f << "Author  : " << authorDisplay() << "\n\n";
    f << "Files created:\n";
    for (auto& e : log_) f << "  " << e << "\n";
    Console::success("Report written to " + rp.string());
}

// ─── List Templates ──────────────────────────────────────────────────────────

inline void Bootstrapper::listTemplates() const {
    Console::printBanner();
    Console::print("Available module templates:", Console::Color::Cyan);
    Console::print("  standard    Full C++ module with src, test, docs, config, git hooks (default)");
    Console::print("  minimal     Only src/ and a README (combine with --no-tests --no-docs --no-config)");
    Console::print("  singleton   Generate a Singleton class pattern (use --singleton)");
    Console::print("  pimpl       Generate a PIMPL idiom skeleton (use --pimpl)");
    Console::print("  json-cfg    Use JSON instead of YAML for config (use --json)");
    Console::print("");
    Console::print("Select a template by combining flags, e.g.:");
    Console::print("  quanta_occipita -m MyModule --no-tests --no-config   # minimal-ish");
    Console::print("  quanta_occipita -m MyModule --singleton --json       # custom pattern");
}
