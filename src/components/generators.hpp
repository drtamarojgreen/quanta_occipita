#pragma once
#include "../cli.hpp"
#include "../console.hpp"
#include "../licenses.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

namespace fs = std::filesystem;

class Generators {
public:
    Generators(const Config& cfg, std::vector<std::string>& log) : cfg_(cfg), log_(log) {}

    void genSourceFiles(const fs::path& root, const std::string& author, const std::string& year);
    std::string getHeaderString(const std::string& author, const std::string& year);
    void genConfigYaml(const fs::path& root, const std::string& author);
    void genConfigParser(const fs::path& root);
    void genConfigJson(const fs::path& root, const std::string& author);
    void genDocs(const fs::path& root);
    void genIssueTemplates(const fs::path& root);
    void genTests(const fs::path& root);
    void genGTestSkeleton(const fs::path& root);
    void genRootReadme(const fs::path& root);
    void updateRootReadme(const fs::path& root);
    void genLicense(const fs::path& root, const std::string& author, const std::string& year);
    void genGitignore(const fs::path& root);
    void genChangelog(const fs::path& root, const std::string& year);
    void genEditorconfig(const fs::path& root);
    void genClangFormat(const fs::path& root);
    void genCMakeLists(const fs::path& root);
    void genDoxyfile(const fs::path& root);
    void genPythonSkeleton(const fs::path& root);
    void genRustSkeleton(const fs::path& root);
    void genGoSkeleton(const fs::path& root);

private:
    void writeFile(const fs::path& p, const std::string& content);
    void action(const std::string& msg, Console::Color c = Console::Color::Green);
    const Config& cfg_;
    std::vector<std::string>& log_;
};

inline void Generators::writeFile(const fs::path& p, const std::string& content) {
    if (cfg_.dryRun) {
        Console::print("  [dry-run] Would write: " + p.string(), Console::Color::Yellow);
        log_.push_back("[dry-run] " + p.string());
        return;
    }
    if (fs::exists(p) && !cfg_.force) {
        Console::warn("Skipping (exists): " + p.string() + "  (use --force to overwrite)");
        return;
    }
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    if (!f) throw std::runtime_error("Cannot write: " + p.string());
    f << content;
    action("  [+] " + p.string());
}

inline void Generators::action(const std::string& msg, Console::Color c) {
    if (!cfg_.quiet) Console::print(msg, c);
    log_.push_back(msg);
}

inline std::string Generators::getHeaderString(const std::string& author, const std::string& year) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    std::string guard = M;
    for (auto& ch : guard) ch = std::toupper(ch);
    guard += (ext == "cc") ? "_HH" : "_HPP";
    std::string licHdr = Licenses::headerComment(cfg_.license, author, year);
    std::string h = licHdr + "\n";
    if (cfg_.headerGuard) h += "#ifndef " + guard + "\n#define " + guard + "\n\n";
    else h += "#pragma once\n\n";
    h += "#include <string>\n";
    if (cfg_.pimpl) h += "#include <memory>\n";
    h += "\nnamespace " + M + " {\nclass " + M + " {\npublic:\n";
    if (cfg_.singleton) {
        h += "    static " + M + "& instance();\n    " + M + "(const " + M + "&) = delete;\n    " + M + "& operator=(const " + M + "&) = delete;\n\n";
    } else {
        h += "    explicit " + M + "(const std::string& name);\n";
    }
    h += "    virtual ~" + M + "() = default;\n    std::string name() const;\n    void run();\nprivate:\n";
    if (cfg_.singleton) h += "    " + M + "();\n";
    if (cfg_.pimpl) h += "    struct Impl;\n    std::unique_ptr<Impl> impl_;\n";
    else h += "    std::string name_;\n";
    h += "};\n} // namespace " + M + "\n";
    if (cfg_.headerGuard) h += "\n#endif // " + guard + "\n";
    return h;
}

inline void Generators::genSourceFiles(const fs::path& root, const std::string& author, const std::string& year) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";
    std::string h = getHeaderString(author, year);
    std::string licHdr = Licenses::headerComment(cfg_.license, author, year);
    std::string cpp = licHdr + "\n// Item 23: Explanatory comments\n#include \"" + M + "." + h_ext + "\"\n#include <iostream>\n\nnamespace " + M + " {\n";
    if (cfg_.pimpl) cpp += "struct " + M + "::Impl { std::string name; };\n\n";
    if (cfg_.singleton) {
        cpp += M + "& " + M + "::instance() { static " + M + " inst; return inst; }\n" + M + "::" + M + "() ";
        if (cfg_.pimpl) cpp += ": impl_(std::make_unique<Impl>(Impl{\"Singleton " + M + "\"}))";
        else cpp += ": name_(\"Singleton " + M + "\")";
        cpp += " {}\n";
    } else {
        cpp += M + "::" + M + "(const std::string& name) ";
        if (cfg_.pimpl) cpp += ": impl_(std::make_unique<Impl>(Impl{name}))";
        else cpp += ": name_(name)";
        cpp += " {}\n";
    }
    cpp += "std::string " + M + "::name() const { return " + (cfg_.pimpl ? "impl_->name" : "name_") + "; }\n";
    cpp += "void " + M + "::run() { std::cout << \"[\" << name() << \"] running...\\n\"; }\n} // namespace " + M + "\n";
    std::string maincpp = licHdr + "\n#include \"" + M + "." + h_ext + "\"\nint main() {\n";
    if (cfg_.singleton) maincpp += "    auto& module = " + M + "::" + M + "::instance();\n";
    else maincpp += "    " + M + "::" + M + " module(\"" + M + "\");\n";
    maincpp += "    module.run(); return 0;\n}\n";
    fs::path src = root / "src";
    if (cfg_.headerOnly) writeFile(src / (M + "." + h_ext), h + "\n" + cpp);
    else { writeFile(src / (M + "." + h_ext), h); writeFile(src / (M + "." + ext), cpp); }
    writeFile(src / ("main." + ext), maincpp);
}

inline void Generators::genConfigYaml(const fs::path& root, const std::string& author) {
    const std::string& M = cfg_.moduleName;
    std::string y = "# config.yaml\nmetadata:\n  module: \"" + M + "\"\n  author: \"" + author + "\"\n";
    if (!cfg_.authorEmail.empty()) y += "  email: \"" + cfg_.authorEmail + "\"\n";
    if (!cfg_.copyrightHolder.empty()) y += "  copyright: \"" + cfg_.copyrightHolder + "\"\n";
    y += "\nnested_config:\n  sub_setting: 42\n  enabled_nodes:\n    - \"node_1\"\n    - \"node_2\"\n";
    writeFile(root / "config.yaml", y);
}

inline void Generators::genConfigParser(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& h_ext = (cfg_.extension == "cc") ? "hh" : "hpp";
    std::string h = "#pragma once\n#include <string>\nnamespace " + M + " {\nstruct Config { std::string logLevel = \"info\"; };\nclass ConfigParser { public: static Config parse(const std::string& f); };\n}\n";
    writeFile(root / "src" / ("ConfigParser." + h_ext), h);
}

inline void Generators::genConfigJson(const fs::path& root, const std::string& author) {
    std::string j = "{\n  \"metadata\": {\n    \"module\": \"" + cfg_.moduleName + "\",\n    \"author\": \"" + author + "\"\n  }\n}\n";
    writeFile(root / "config.json", j);
}

inline void Generators::genDocs(const fs::path& root) {
    writeFile(root / "docs" / "plan.md", "# Plan\n");
    writeFile(root / "docs" / "enhancements.md", "# Enhancements\n");
    writeFile(root / "README.md", "# " + cfg_.moduleName + "\n");
}

inline void Generators::genIssueTemplates(const fs::path& root) {
    fs::path tpl = root / ".github" / "ISSUE_TEMPLATE";
    writeFile(tpl / "bug_report.md", "name: Bug\n");
}

inline void Generators::genTests(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";
    std::string t = "#define CATCH_CONFIG_MAIN\n#include \"catch2/catch.hpp\"\n#include \"../src/" + M + "." + h_ext + "\"\n";
    t += "TEST_CASE(\"" + M + " TDD check\") { REQUIRE(false); }\n";
    writeFile(root / "test" / (M + ".test." + ext), t);

    std::string runner = "#!/usr/bin/env bash\nset -e\ng++ -std=c++17 -o run_tests " + M + ".test." + ext + " ../src/" + M + "." + ext + "\n./run_tests\n";
    writeFile(root / "test" / "run_tests.sh", runner);

    std::string tc = "cmake_minimum_required(VERSION 3.10)\nproject(" + M + "_tests)\nadd_executable(run_tests " + M + ".test." + ext + " ../src/" + M + "." + ext + ")\n";
    writeFile(root / "test" / "CMakeLists.txt", tc);
}

inline void Generators::genGTestSkeleton(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";
    std::string t = "#include <gtest/gtest.h>\n#include \"../src/" + M + "." + h_ext + "\"\nTEST(" + M + "Test, Basic) { EXPECT_TRUE(true); }\n";
    writeFile(root / "test" / (M + ".gtest." + ext), t);
}

inline void Generators::genRootReadme(const fs::path& root) {
    writeFile(root / "CONTRIBUTING.md", "# Contributing\n");
    writeFile(root / "CODE_OF_CONDUCT.md", "# CoC\n");
}

inline void Generators::updateRootReadme(const fs::path& root) {
    fs::path rr = fs::path(cfg_.outputPath) / "README.md";
    if (fs::exists(rr) && !cfg_.dryRun) {
        std::ofstream f(rr, std::ios::app);
        f << "\n- [" << cfg_.moduleName << "](" << cfg_.moduleName << "/README.md)\n";
    }
}

inline void Generators::genLicense(const fs::path& root, const std::string& author, const std::string& year) {
    writeFile(root / "LICENSE", Licenses::getText(cfg_.license, author, year));
}

inline void Generators::genGitignore(const fs::path& root) {
    writeFile(root / ".gitignore", "*.o\nbuild/\n");
}

inline void Generators::genChangelog(const fs::path& root, const std::string& year) {
    writeFile(root / "CHANGELOG.md", "# Changelog\n## [v0.1.0] - " + year + "\n");
}

inline void Generators::genEditorconfig(const fs::path& root) {
    writeFile(root / ".editorconfig", "root = true\n[*]\nindent_style = space\n");
}

inline void Generators::genClangFormat(const fs::path& root) {
    writeFile(root / ".clang-format", "BasedOnStyle: LLVM\n");
}

inline void Generators::genCMakeLists(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    std::string c = "cmake_minimum_required(VERSION 3.10)\nproject(" + M + ")\n";
    if (cfg_.headerOnly) c += "add_library(" + M + " INTERFACE)\n";
    else c += "add_library(" + M + " " + (cfg_.shared ? "SHARED" : "STATIC") + " src/" + M + "." + ext + ")\n";
    writeFile(root / "CMakeLists.txt", c);
}

inline void Generators::genDoxyfile(const fs::path& root) {
    writeFile(root / "Doxyfile", "PROJECT_NAME = " + cfg_.moduleName + "\n");
}

inline void Generators::genPythonSkeleton(const fs::path& root) {
    writeFile(root / "main.py", "print('hello')\n");
}

inline void Generators::genRustSkeleton(const fs::path& root) {
    writeFile(root / "src/main.rs", "fn main() {}\n");
}

inline void Generators::genGoSkeleton(const fs::path& root) {
    writeFile(root / "main.go", "package main\nfunc main() {}\n");
}
