#pragma once
#include "../cli.hpp"
#include "../console.hpp"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

class GitManager {
public:
    explicit GitManager(const Config& cfg) : cfg_(cfg) {}

    void handleGit(const fs::path& root) {
        if (cfg_.dryRun) {
            Console::print("  [dry-run] Would: git init, add, commit, set branch to " + cfg_.initialBranch,
                           Console::Color::Yellow);
            return;
        }

        bool isRepo = fs::exists(root / ".git");
        std::string response;
        if (!isRepo) {
            std::cout << "\033[36mInitialise a git repository in " << root.string() << "? (Y/N): \033[0m";
            std::getline(std::cin, response);
            if (response != "Y" && response != "y") return;
        } else {
            Console::info("Git repository already exists. Creating new commit on current branch.");
        }

        auto runCmd = [&](const std::string& cmd) {
            int ret = std::system(("cd '" + root.string() + "' && " + cmd + " 2>/dev/null").c_str());
            return ret == 0;
        };

        if (!isRepo) {
            if (!runCmd("git init")) { Console::error("git init failed."); return; }
            runCmd("git checkout -b '" + cfg_.initialBranch + "'");
        } else if (cfg_.newBranch) {
            runCmd("git checkout -b '" + cfg_.initialBranch + "'");
        }

        runCmd("git add .");

        std::string msg = cfg_.commitMessage.empty()
            ? "chore: bootstrap " + cfg_.moduleName + " module via QuantaOccipita"
            : cfg_.commitMessage;

        if (!runCmd("git commit -m '" + msg + "'")) {
            Console::warn("git commit failed (is git configured with user.email/user.name?)");
            return;
        }
        Console::success("Git repository initialised with initial commit.");

        if (cfg_.tag) {
            runCmd("git tag -a v0.1.0 -m \"Initial release of " + cfg_.moduleName + "\"");
            Console::success("Tagged v0.1.0");
        }

        std::string remote = cfg_.push ? "" : "PROMPT";
        if (remote == "PROMPT") {
            std::cout << "\033[36mPush to remote? Enter remote URL (or press Enter to skip): \033[0m";
            std::getline(std::cin, remote);
        }

        if (!remote.empty() || cfg_.push) {
            if (!remote.empty()) runCmd("git remote add " + cfg_.remoteName + " '" + remote + "'");
            if (runCmd("git push -u " + cfg_.remoteName + " '" + cfg_.initialBranch + "'"))
                Console::success("Pushed to remote.");
            else
                Console::warn("Push failed. You can push manually later.");
        }
    }

private:
    const Config& cfg_;
};
