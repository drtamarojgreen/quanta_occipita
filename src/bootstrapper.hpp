#pragma once

#include "cli.hpp"
#include "console.hpp"
#include "licenses.hpp"
#include "components/validator.hpp"
#include "components/git_manager.hpp"
#include "components/generators.hpp"
#include "components/skeletons.hpp"
#include "components/manager.hpp"

#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

class Bootstrapper {
public:
    explicit Bootstrapper(Config& cfg)
        : cfg_(cfg),
          validator_(cfg),
          git_manager_(cfg),
          generators_(cfg, log_),
          skeletons_(cfg),
          manager_(cfg, generators_) {}

    void run();
    void cleanup();

private:
    Config& cfg_;
    Validator validator_;
    GitManager git_manager_;
    Generators generators_;
    Skeletons skeletons_;
    Manager manager_;

    std::vector<std::string> log_;

    fs::path rootPath() const;
    std::string currentYear() const;
    std::string authorDisplay() const;

    void makeDir(const fs::path& p);
    void action(const std::string& msg, Console::Color c = Console::Color::Green);
    void listTemplates() const;
};

inline void Bootstrapper::cleanup() {
    if (cfg_.dryRun) return;
    Console::warn("Cleaning up partially generated files...");
    for (const auto& f : log_) {
        if (f.find("  [+] ") == 0) {
            std::string path = f.substr(6);
            if (fs::exists(path)) {
                fs::remove(path);
                Console::debug("Removed: " + path);
            }
        }
    }
}

inline void Bootstrapper::run() {
    if (cfg_.listTemplates) { listTemplates(); return; }
    if (!cfg_.profile.empty()) manager_.loadProfile();
    if (!validator_.checkEnvironment()) return;

    fs::path root = rootPath();
    if (!cfg_.dryRun) {
        if (fs::exists(root) && !cfg_.force) {
            bool nonEmpty = false;
            if (fs::is_directory(root)) {
                for ([[maybe_unused]] auto& e : fs::directory_iterator(root)) { nonEmpty = true; break; }
            }
            if (nonEmpty) {
                if (cfg_.quiet) { Console::error("Directory non-empty. Use --force."); return; }
                std::cout << "\033[33m[!] Directory not empty. Continue? (Y/N): \033[0m";
                std::string res; std::getline(std::cin, res);
                if (res != "Y" && res != "y") { Console::info("Aborted."); return; }
            }
        }
        makeDir(root);
    }

    generators_.genSourceFiles(root, authorDisplay(), currentYear());
    if (cfg_.factory) skeletons_.genFactory(root);
    if (cfg_.observer) skeletons_.genObserver(root);

    if (!cfg_.noConfig) {
        if (cfg_.jsonConfig) generators_.genConfigJson(root, authorDisplay());
        else {
            generators_.genConfigYaml(root, authorDisplay());
            generators_.genConfigParser(root);
        }
    }

    if (!cfg_.noDocs) generators_.genDocs(root);
    generators_.genIssueTemplates(root);
    if (!cfg_.noTests) {
        generators_.genTests(root);
        generators_.genGTestSkeleton(root);
    }

    generators_.genRootReadme(root);
    generators_.updateRootReadme(root);
    if (cfg_.license != "none") generators_.genLicense(root, authorDisplay(), currentYear());

    generators_.genGitignore(root);
    generators_.genChangelog(root, currentYear());
    generators_.genEditorconfig(root);
    generators_.genClangFormat(root);
    generators_.genCMakeLists(root);
    generators_.genDoxyfile(root);

    if (cfg_.lang == "python") generators_.genPythonSkeleton(root);
    else if (cfg_.lang == "rust") generators_.genRustSkeleton(root);
    else if (cfg_.lang == "go") generators_.genGoSkeleton(root);

    skeletons_.genAdaptiveSkeletons(root);

    if (!cfg_.skipGit) git_manager_.handleGit(root);
    if (cfg_.hooks) manager_.runPostGenHook(root);

    if (cfg_.update) manager_.updateModule(root, authorDisplay(), currentYear());
    if (cfg_.eject) manager_.ejectModule(root);
    if (!cfg_.noUpdateCheck) manager_.checkNewVersion();

    if (cfg_.snippets) manager_.handleSnippets(root);
    if (cfg_.customFiles) manager_.handleCustomFiles(root);
    if (cfg_.plugins) manager_.handlePlugins(root);

    if (cfg_.report) {
        fs::path rp = root / "quanta_report.txt";
        std::ofstream f(rp);
        f << "Report for " << cfg_.moduleName << "\nFiles:\n";
        for (auto& l : log_) f << l << "\n";
        Console::success("Report written.");
    }
}

inline fs::path Bootstrapper::rootPath() const {
    std::string name = cfg_.moduleName;
    if (cfg_.timestamp) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        char buf[16]; std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
        name += "_" + std::string(buf);
    }
    if (cfg_.flatStructure) return fs::path(cfg_.outputPath);
    return fs::path(cfg_.outputPath) / name;
}

inline std::string Bootstrapper::currentYear() const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buf[8]; std::strftime(buf, sizeof(buf), "%Y", &tm);
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

inline void Bootstrapper::action(const std::string& msg, Console::Color c) {
    if (!cfg_.quiet) Console::print(msg, c);
    log_.push_back(msg);
}

inline void Bootstrapper::listTemplates() const {
    Console::printBanner();
    Console::print("Templates:", Console::Color::Cyan);
    Console::print("  standard, minimal, singleton, pimpl, json-cfg");
}
