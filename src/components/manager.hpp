#pragma once
#include "../cli.hpp"
#include "../console.hpp"
#include "generators.hpp"
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

class Manager {
public:
    Manager(Config& cfg, Generators& gens) : cfg_(cfg), generators_(gens) {}

    void loadProfile() {
        Console::info("Loading profile: " + cfg_.profile);
        fs::path p = fs::path(".quanta_profiles") / (cfg_.profile + ".quanta_config");
        if (fs::exists(p)) {
            std::ifstream f(p);
            std::string line;
            while (std::getline(f, line)) {
                if (line == "cpp_std=20") cfg_.cppStd = "20";
                if (line == "no_tests=true") cfg_.noTests = true;
            }
            Console::success("Applied profile settings from " + p.string());
        } else {
            Console::warn("Profile " + cfg_.profile + " not found in .quanta_profiles/");
        }
    }

    void runPostGenHook(const fs::path& root) {
        Console::info("Running post-generation hook...");
        std::string hook = "post_gen.sh";
        if (fs::exists(root / hook)) {
            Console::success("Executing " + hook);
            std::system(("cd '" + root.string() + "' && bash ./" + hook).c_str());
        }
    }

    void updateModule(const fs::path& root, const std::string& author, const std::string& year) {
        Console::info("Updating module: " + cfg_.moduleName);
        generators_.genSourceFiles(root, author, year);
        generators_.genCMakeLists(root);
        Console::success("Module boilerplate updated.");
    }

    void ejectModule(const fs::path& root) {
        Console::info("Ejecting module: " + cfg_.moduleName);
        fs::path config = root / ".quanta_config";
        fs::path report = root / "quanta_report.txt";
        if (fs::exists(config)) fs::remove(config);
        if (fs::exists(report)) fs::remove(report);
        Console::success("Module ejected.");
    }

    void checkNewVersion() {
        std::string latest = "1.1.1";
        if (std::string(QUANTA_VERSION) < latest) {
            Console::info("New version available: v" + latest);
        }
    }

    void handleSnippets(const fs::path& root) {
        fs::path dir = ".quanta_snippets";
        if (!fs::exists(dir)) return;

        const std::string& h_ext = (cfg_.extension == "cc") ? "hh" : "hpp";
        fs::path target = root / "src" / (cfg_.moduleName + "." + h_ext);

        for (auto& p : fs::directory_iterator(dir)) {
            if (p.path().extension() == "." + h_ext) {
                std::ifstream src(p.path());
                std::ofstream dst(target, std::ios::app);
                dst << "\n// Snippet from " << p.path().filename() << "\n" << src.rdbuf() << "\n";
                Console::info("Injected snippet: " + p.path().filename().string());
            }
        }
    }

    void handleCustomFiles(const fs::path& root) {
        fs::path dir = ".quanta_custom_files";
        if (fs::exists(dir)) {
            for (auto& p : fs::directory_iterator(dir)) {
                fs::copy(p.path(), root / p.path().filename(), fs::copy_options::overwrite_existing);
                Console::info("Copied custom file: " + p.path().filename().string());
            }
        }
    }

    void handlePlugins(const fs::path& root) {
        fs::path dir = ".quanta_plugins";
        if (fs::exists(dir)) {
            for (auto& p : fs::directory_iterator(dir)) {
                Console::success("Executing plugin: " + p.path().filename().string());
                std::system(("cd '" + root.string() + "' && bash " + p.path().string()).c_str());
            }
        }
    }

private:
    Config& cfg_;
    Generators& generators_;
};
