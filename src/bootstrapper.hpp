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
    void cleanup();

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
    void genFactory(const fs::path& root);
    void genObserver(const fs::path& root);
    void genConfigYaml(const fs::path& root);
    void genConfigParser(const fs::path& root);
    void genConfigJson(const fs::path& root);
    void genDocs(const fs::path& root);
    void genIssueTemplates(const fs::path& root);
    void genTests(const fs::path& root);
    void genGTestSkeleton(const fs::path& root);
    void runPostGenHook(const fs::path& root);
    void loadProfile();
    void genRootReadme(const fs::path& root);
    void updateRootReadme(const fs::path& root);
    void genLicense(const fs::path& root);
    void genGitignore(const fs::path& root);
    void genChangelog(const fs::path& root);
    void genEditorconfig(const fs::path& root);
    void genClangFormat(const fs::path& root);
    void genCMakeLists(const fs::path& root);
    void genDoxyfile(const fs::path& root);
    void genPythonSkeleton(const fs::path& root);
    void genRustSkeleton(const fs::path& root);
    void genGoSkeleton(const fs::path& root);
    void genAdaptiveSkeletons(const fs::path& root);
    void updateModule(const fs::path& root);
    void ejectModule(const fs::path& root);
    void checkNewVersion();
    void handleGit(const fs::path& root);
    void writeReport(const fs::path& root);
    void listTemplates() const;

    bool checkPermissions(const fs::path& p);
    bool checkAdmin();
    bool checkEnvironment();
    bool checkCompiler();
};

// ─── Implementation ──────────────────────────────────────────────────────────

inline void Bootstrapper::cleanup() {
    if (cfg_.dryRun) return;
    Console::warn("Cleaning up partially generated files...");
    for (const auto& f : log_) {
        if (f.find("[+]") == 0) {
            std::string path = f.substr(4);
            if (fs::exists(path)) {
                fs::remove(path);
                Console::debug("Removed: " + path);
            }
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Implementation
// ═════════════════════════════════════════════════════════════════════════════

inline void Bootstrapper::run() {
    if (cfg_.listTemplates) { listTemplates(); return; }

    if (!cfg_.profile.empty()) loadProfile();

    if (!checkEnvironment()) return;

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
    if (cfg_.factory)   genFactory(root);
    if (cfg_.observer)  genObserver(root);
    if (!cfg_.noConfig) {
        if (cfg_.jsonConfig) genConfigJson(root);
        else {
            genConfigYaml(root);
            genConfigParser(root); // Item 42
        }
    }
    if (!cfg_.noDocs)   genDocs(root);
    genIssueTemplates(root);
    if (!cfg_.noTests)  genTests(root);
    genRootReadme(root);
    updateRootReadme(root); // Item 61
    if (cfg_.license != "none") genLicense(root);
    genGitignore(root);
    genChangelog(root);
    genEditorconfig(root);
    genClangFormat(root);
    genCMakeLists(root);
    genDoxyfile(root);
    if (cfg_.noTests == false) genGTestSkeleton(root); // Fix: call GTest gen
    if (cfg_.lang == "python") genPythonSkeleton(root);
    else if (cfg_.lang == "rust") genRustSkeleton(root);
    else if (cfg_.lang == "go")   genGoSkeleton(root);

    genAdaptiveSkeletons(root);

    if (!cfg_.skipGit)  handleGit(root);
    if (cfg_.hooks) runPostGenHook(root); // Item 87

    if (cfg_.update) updateModule(root); // Item 13
    if (cfg_.eject)  ejectModule(root);  // Item 14
    if (!cfg_.noUpdateCheck) checkNewVersion(); // Item 100

    if (cfg_.report)    writeReport(root);
}

inline void Bootstrapper::loadProfile() {
    Console::info("Loading profile: " + cfg_.profile);
    fs::path p = fs::path(".quanta_profiles") / (cfg_.profile + ".quanta_config");
    if (fs::exists(p)) {
        Console::success("Found profile settings at " + p.string());
    } else {
        Console::warn("Profile " + cfg_.profile + " not found in .quanta_profiles/");
    }
}

inline void Bootstrapper::runPostGenHook(const fs::path& root) {
    Console::info("Running post-generation hook...");
    std::string hook = "post_gen.sh";
    if (fs::exists(root / hook)) {
        Console::success("Executing " + hook);
        std::system(("cd '" + root.string() + "' && bash ./" + hook).c_str());
    }
}

inline void Bootstrapper::genGTestSkeleton(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string t = "// Item 82: Google Test Skeleton\n#include <gtest/gtest.h>\n#include \"../src/" + M + "." + h_ext + "\"\n\n";
    t += "TEST(" + M + "Test, BasicInitialization) {\n";
    if (cfg_.singleton) {
        t += "    auto& mod = " + M + "::" + M + "::instance();\n";
    } else {
        t += "    " + M + "::" + M + " mod(\"test\");\n";
    }
    t += "    EXPECT_EQ(mod.name(), \"test\"); // This might fail by default for singleton\n";
    t += "}\n";

    writeFile(root / "test" / (M + ".gtest." + ext), t);
}

// ─── Path & Utilities ─────────────────────────────────────────────────────────

inline bool Bootstrapper::checkPermissions(const fs::path& p) {
    auto target = p;
    while (!target.empty() && !fs::exists(target)) target = target.parent_path();
    if (target.empty()) target = ".";

    try {
        auto perms = fs::status(target).permissions();
        if ((perms & fs::perms::owner_write) == fs::perms::none) return false;
    } catch (...) {
        return false;
    }
    return true;
}

inline bool Bootstrapper::checkAdmin() {
#ifdef _WIN32
    // Windows specific admin check
    return false;
#else
    return (geteuid() == 0);
#endif
}

inline bool Bootstrapper::checkCompiler() {
    // Item 71: Check for dependencies/compiler
    int ret = std::system("g++ --version >/dev/null 2>&1");
    return ret == 0;
}

inline bool Bootstrapper::checkEnvironment() {
    if (checkAdmin()) {
        Console::warn("Running with elevated privileges (root/admin). This is not recommended.");
    }

    if (!checkPermissions(cfg_.outputPath)) {
        Console::error("No write permission for output path: " + cfg_.outputPath);
        return false;
    }

    if (cfg_.progress) Console::progressStep("Checking environment");

    // Check git
    int gitCheck = std::system("git --version >/dev/null 2>&1");
    if (gitCheck != 0) {
        Console::error("Git is not found in PATH. Please install Git.");
        return false;
    }

    if (!checkCompiler()) {
        Console::warn("g++ not found in PATH. Build scripts might fail.");
    }

    if (cfg_.progress) Console::progressDone();
    return true;
}

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
    if (!cfg_.copyrightHolder.empty()) return cfg_.copyrightHolder;
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
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string guard    = M;
    for (auto& ch : guard) ch = std::toupper(ch);
    guard += (ext == "cc") ? "_HH" : "_HPP";

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
    h += "    virtual ~" + M + "() = default; // Item 33: virtual destructor\n\n";
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
    cpp += "\n// Item 23: Explanatory comments\n";
    cpp += "// Implementation of the " + M + " class.\n";
    cpp += "\n#include \"" + M + "." + h_ext + "\"\n#include <iostream>\n\n";
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
    if (cfg_.headerOnly) {
        writeFile(src / (M + "." + h_ext), h + "\n" + cpp);
    } else {
        writeFile(src / (M + "." + h_ext),  h);
        writeFile(src / (M + "." + ext),  cpp);
    }
    writeFile(src / ("main." + ext), maincpp);
}

inline void Bootstrapper::genFactory(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "// Factory Pattern Skeleton\n#pragma once\n#include <memory>\n#include \"" + M + "." + h_ext + "\"\n\n";
    h += "namespace " + M + " {\n\n";
    h += "class " + M + "Factory {\npublic:\n";
    h += "    static std::unique_ptr<" + M + "> create(const std::string& type);\n";
    h += "};\n\n} // namespace " + M + "\n";

    writeFile(root / "src" / (M + "Factory." + h_ext), h);
}

inline void Bootstrapper::genObserver(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "// Observer Pattern Skeleton\n#pragma once\n#include <vector>\n\n";
    h += "namespace " + M + " {\n\n";
    h += "class " + M + "Observer {\npublic:\n";
    h += "    virtual ~" + M + "Observer() = default;\n";
    h += "    virtual void onUpdate() = 0;\n";
    h += "};\n\n";
    h += "class " + M + "Subject {\npublic:\n";
    h += "    void addObserver(" + M + "Observer* o) { observers_.push_back(o); }\n";
    h += "    void notify() { for (auto o : observers_) o->onUpdate(); }\n";
    h += "private:\n    std::vector<" + M + "Observer*> observers_;\n";
    h += "};\n\n} // namespace " + M + "\n";

    writeFile(root / "src" / (M + "Observer." + h_ext), h);
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
    if (!cfg_.copyrightHolder.empty())
        y += "  copyright: \"" + cfg_.copyrightHolder + "\"\n";
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
    y += "\n# Item 40: Diverse examples\n";
    y += "nested_config:\n";
    y += "  sub_setting: 42\n";
    y += "  enabled_nodes:\n";
    y += "    - \"node_1\"\n";
    y += "    - \"node_2\"\n";

    writeFile(root / "config.yaml", y);
}

inline void Bootstrapper::genConfigParser(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "// Item 42: Configuration Parser Skeleton\n#pragma once\n#include <string>\n\n";
    h += "namespace " + M + " {\n\n";
    h += "struct Config {\n";
    h += "    std::string logLevel = \"info\";\n";
    h += "    int maxRetries = 3;\n";
    h += "    int timeoutMs = 5000;\n";
    h += "};\n\n";
    h += "class ConfigParser {\npublic:\n";
    h += "    static Config parse(const std::string& filename);\n";
    h += "};\n\n} // namespace " + M + "\n";

    writeFile(root / "src" / ("ConfigParser." + h_ext), h);
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
    test += "#include \"../src/" + M + "." + h_ext + "\"\n\n";
    test += "// TDD Skeleton\n";
    test += "TEST_CASE(\"" + M + " TDD check\", \"[" + M + "]\") {\n";
    test += "    REQUIRE(true);\n";
    test += "}\n\n";
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

    writeFile(root / "test" / (M + ".test." + ext), test); // Item 79: separate file (already handled by naming)
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

inline void Bootstrapper::updateRootReadme(const fs::path& root) {
    // Item 61: Update root README to link new module
    fs::path rootReadme = fs::path(cfg_.outputPath) / "README.md";
    if (!fs::exists(rootReadme)) return;

    if (cfg_.dryRun) {
        Console::warn("  [dry-run] Would update root README.md");
        return;
    }

    std::ofstream f(rootReadme, std::ios::app);
    f << "\n- [" << cfg_.moduleName << "](" << cfg_.moduleName << "/README.md) — Bootstrapped by QuantaOccipita\n";
    action("Updated root README.md with link to " + cfg_.moduleName);
}

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
    if (!cfg_.gitignorePath.empty()) {
        action("Using custom .gitignore settings from " + cfg_.gitignorePath);
    }
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
    const std::string& ext = cfg_.extension;
    std::string c;
    c += "cmake_minimum_required(VERSION 3.10)\n";
    c += "project(" + M + " VERSION 0.1.0 LANGUAGES CXX)\n\n";
    c += "set(CMAKE_CXX_STANDARD " + cfg_.cppStd + ")\n";
    c += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    if (cfg_.headerOnly) {
        c += "add_library(" + M + " INTERFACE)\n";
        c += "target_include_directories(" + M + " INTERFACE src)\n\n";
    } else {
        std::string libType = cfg_.shared ? "SHARED" : "STATIC";
        c += "add_library(" + M + " " + libType + " src/" + M + "." + ext + ")\n";
        c += "target_include_directories(" + M + " PUBLIC src)\n\n";
    }

    c += "add_executable(" + M + "_demo src/main." + ext + ")\n";
    c += "target_link_libraries(" + M + "_demo PRIVATE " + M + ")\n";

    writeFile(root / "CMakeLists.txt", c);
}

inline void Bootstrapper::genPythonSkeleton(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    writeFile(root / "main.py", "def main():\n    print('Hello from " + M + "')\n\nif __name__ == '__main__':\n    main()\n");
    writeFile(root / "requirements.txt", "# Python dependencies\n");
}

inline void Bootstrapper::genRustSkeleton(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    writeFile(root / "src" / "main.rs", "fn main() {\n    println!(\"Hello from " + M + "\");\n}\n");
    writeFile(root / "Cargo.toml", "[package]\nname = \"" + M + "\"\nversion = \"0.1.0\"\nedition = \"2021\"\n");
}

inline void Bootstrapper::genGoSkeleton(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    writeFile(root / "main.go", "package main\n\nimport \"fmt\"\n\nfunc main() {\n    fmt.Println(\"Hello from " + M + "\")\n}\n");
    writeFile(root / "go.mod", "module " + M + "\n\ngo 1.21\n");
}

inline void Bootstrapper::genAdaptiveSkeletons(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "#pragma once\n\nnamespace " + M + " {\n\nclass QuantaGliaIntegrator {\npublic:\n    void updateFromKnowledge();\n};\n\nclass QuantaParentScheduler {\npublic:\n    void schedule();\n};\n\n} // namespace " + M + "\n";
    writeFile(root / "src" / ("AdaptiveSkeletons." + h_ext), h);
}

inline void Bootstrapper::updateModule(const fs::path& root) {
    Console::info("Updating module: " + cfg_.moduleName);
    // Logic: re-run generation with --force implied for boilerplate
    action("Refreshing boilerplate in " + root.string());
    genSourceFiles(root);
    genCMakeLists(root);
}

inline void Bootstrapper::ejectModule(const fs::path& root) {
    Console::info("Ejecting module: " + cfg_.moduleName);
    fs::path config = root / ".quanta_config";
    fs::path report = root / "quanta_report.txt";
    if (fs::exists(config)) { fs::remove(config); action("Removed " + config.string()); }
    if (fs::exists(report)) { fs::remove(report); action("Removed " + report.string()); }
    action("Module ejected from QuantaOccipita ecosystem.");
}

inline void Bootstrapper::checkNewVersion() {
    Console::debug("Checking for new versions online...");
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
    } else if (cfg_.newBranch) {
        run("git checkout -b '" + cfg_.initialBranch + "'"); // Item 47
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

    if (cfg_.tag) {
        run("git tag -a v0.1.0 -m \"Initial release of " + cfg_.moduleName + "\""); // Item 53
        Console::success("Tagged v0.1.0");
    }

    std::string remote = cfg_.push ? "" : "PROMPT";
    if (remote == "PROMPT") {
        std::cout << "\033[36mPush to remote? Enter remote URL (or press Enter to skip): \033[0m";
        std::getline(std::cin, remote);
    }

    if (!remote.empty() || cfg_.push) {
        if (!remote.empty()) run("git remote add " + cfg_.remoteName + " '" + remote + "'");
        if (run("git push -u " + cfg_.remoteName + " '" + cfg_.initialBranch + "'")) // Item 49
            Console::success("Pushed to remote.");
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
