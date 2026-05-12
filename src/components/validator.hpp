#pragma once
#include "../cli.hpp"
#include "../console.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs = std::filesystem;

class Validator {
public:
    explicit Validator(const Config& cfg) : cfg_(cfg) {}

    bool checkPermissions(const fs::path& p) {
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

    bool checkAdmin() {
#ifdef _WIN32
        return false;
#else
        return (geteuid() == 0);
#endif
    }

    bool checkCompiler() {
        int ret = std::system("g++ --version >/dev/null 2>&1");
        return ret == 0;
    }

    bool checkEnvironment() {
        if (checkAdmin()) {
            Console::warn("Running with elevated privileges (root/admin). This is not recommended.");
        }

        if (!checkPermissions(cfg_.outputPath)) {
            Console::error("No write permission for output path: " + cfg_.outputPath);
            return false;
        }

        if (cfg_.progress) Console::progressStep("Checking environment");

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

private:
    const Config& cfg_;
};
