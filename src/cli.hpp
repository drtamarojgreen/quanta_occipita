#pragma once
#include <string>
#include <vector>

// ─── Config ──────────────────────────────────────────────────────────────────

struct Config {
    std::string moduleName;
    std::string outputPath   = ".";          // custom output directory
    std::string license      = "mit";        // license type
    std::string authorName;
    std::string authorEmail;
    std::string initialBranch = "main";
    std::string commitMessage;               // custom git commit message
    std::string cppStd        = "17";        // C++ standard version

    bool noTests      = false;
    bool dryRun       = false;
    bool force        = false;
    bool quiet        = false;
    bool noBanner     = false;
    bool skipGit      = false;
    bool noConfig     = false;
    bool noDocs       = false;
    bool report       = false;
    bool timestamp    = false;
    bool showHelp     = false;
    bool showVersion  = false;
    bool listTemplates= false;
    bool headerGuard  = false;     // use #ifndef guard instead of #pragma once
    bool flatStructure= false;     // no module subdirectory
};

// ─── CLI ─────────────────────────────────────────────────────────────────────

#include "console.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

class CLI {
public:
    Config parse(int argc, char* argv[]);
    void printHelp() const;
    std::string promptModuleName() const;
    bool validateModuleName(const std::string& name) const;

private:
    std::string getArg(const std::vector<std::string>& args, const std::string& flag, const std::string& def = "") const;
    bool hasFlag(const std::vector<std::string>& args, const std::string& flag) const;
    Config loadUserConfig() const;
    void saveUserConfig(const Config& cfg) const;

    static constexpr const char* CONFIG_FILE = ".quanta_config";
};

// ─── Inline Implementation ────────────────────────────────────────────────────

inline Config CLI::loadUserConfig() const {
    Config cfg;
    if (!fs::exists(CONFIG_FILE)) return cfg;
    std::ifstream f(CONFIG_FILE);
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key == "author_name")  cfg.authorName  = val;
        if (key == "author_email") cfg.authorEmail = val;
        if (key == "license")      cfg.license     = val;
        if (key == "output_path")  cfg.outputPath  = val;
        if (key == "initial_branch") cfg.initialBranch = val;
    }
    return cfg;
}

inline void CLI::saveUserConfig(const Config& cfg) const {
    std::ofstream f(CONFIG_FILE);
    if (!cfg.authorName.empty())   f << "author_name="   << cfg.authorName  << "\n";
    if (!cfg.authorEmail.empty())  f << "author_email="  << cfg.authorEmail << "\n";
    if (!cfg.license.empty())      f << "license="       << cfg.license     << "\n";
    if (cfg.outputPath != ".")     f << "output_path="   << cfg.outputPath  << "\n";
    if (!cfg.initialBranch.empty())f << "initial_branch="<< cfg.initialBranch << "\n";
}

inline bool CLI::hasFlag(const std::vector<std::string>& args, const std::string& flag) const {
    return std::find(args.begin(), args.end(), flag) != args.end();
}

inline std::string CLI::getArg(const std::vector<std::string>& args, const std::string& flag, const std::string& def) const {
    for (size_t i = 0; i + 1 < args.size(); ++i)
        if (args[i] == flag) return args[i + 1];
    return def;
}

inline Config CLI::parse(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    // Load persisted defaults first
    Config cfg = loadUserConfig();

    if (hasFlag(args, "--help")    || hasFlag(args, "-h"))  { cfg.showHelp    = true; return cfg; }
    if (hasFlag(args, "--version") || hasFlag(args, "-v"))  { cfg.showVersion = true; return cfg; }
    if (hasFlag(args, "--list-templates"))                  { cfg.listTemplates = true; }

    cfg.moduleName    = getArg(args, "--module",      getArg(args, "-m", cfg.moduleName));
    cfg.outputPath    = getArg(args, "--output",      getArg(args, "-o", cfg.outputPath));
    cfg.license       = getArg(args, "--license",     cfg.license);
    cfg.authorName    = getArg(args, "--author",      cfg.authorName);
    cfg.authorEmail   = getArg(args, "--email",       cfg.authorEmail);
    cfg.initialBranch = getArg(args, "--branch",      cfg.initialBranch);
    cfg.commitMessage = getArg(args, "--message",     getArg(args, "-M", ""));
    cfg.cppStd        = getArg(args, "--cpp-std",     cfg.cppStd);

    cfg.noTests       = hasFlag(args, "--no-tests");
    cfg.dryRun        = hasFlag(args, "--dry-run");
    cfg.force         = hasFlag(args, "--force")    || hasFlag(args, "-f");
    cfg.quiet         = hasFlag(args, "--quiet")    || hasFlag(args, "--silent");
    cfg.noBanner      = hasFlag(args, "--no-banner");
    cfg.skipGit       = hasFlag(args, "--skip-git");
    cfg.noConfig      = hasFlag(args, "--no-config");
    cfg.noDocs        = hasFlag(args, "--no-docs");
    cfg.report        = hasFlag(args, "--report");
    cfg.timestamp     = hasFlag(args, "--timestamp");
    cfg.headerGuard   = hasFlag(args, "--header-guard");
    cfg.flatStructure = hasFlag(args, "--flat");

    // Persist any user-supplied identity settings
    if (!cfg.authorName.empty() || !cfg.authorEmail.empty())
        saveUserConfig(cfg);

    return cfg;
}

inline std::string CLI::promptModuleName() const {
    std::string name;
    while (name.empty()) {
        std::cout << "\033[36mEnter module name (e.g. QuantaCortex): \033[0m";
        std::getline(std::cin, name);
        if (name.empty()) Console::warn("Module name cannot be empty.");
    }
    return name;
}

inline bool CLI::validateModuleName(const std::string& name) const {
    std::regex valid(R"([A-Za-z][A-Za-z0-9_]*)");
    return std::regex_match(name, valid);
}

inline void CLI::printHelp() const {
    Console::printBanner();
    std::cout << R"(
USAGE
  quanta_occipita [OPTIONS]

IDENTITY
  --module,  -m  <name>    Module name (prompted if omitted)
  --author       <name>    Author name (saved to .quanta_config)
  --email        <email>   Author email (saved to .quanta_config)

OUTPUT
  --output,  -o  <path>    Target directory (default: current dir)
  --flat                   No module subdirectory (flat structure)
  --timestamp              Append timestamp to module directory name

GENERATION
  --no-tests               Skip test/ directory and Catch2 skeletons
  --no-config              Skip config.yaml generation
  --no-docs                Skip docs/ directory generation
  --header-guard           Use #ifndef guards instead of #pragma once
  --license      <type>    License type: mit (default), apache2, gpl3, none
  --cpp-std      <ver>     C++ standard version (default: 17)

GIT
  --skip-git               Disable all git actions
  --branch       <name>    Initial branch name (default: main)
  --message, -M  <msg>     Custom initial commit message

BEHAVIOUR
  --dry-run                Preview actions without writing files
  --force,   -f            Overwrite existing files
  --quiet / --silent       Suppress non-essential output
  --no-banner              Hide ASCII banner
  --report                 Generate quanta_report.txt after bootstrapping
  --list-templates         Show available module templates

INFO
  --version, -v            Print version
  --help,    -h            Print this help

EXAMPLES
  quanta_occipita --module QuantaCortex
  quanta_occipita -m QuantaThalamus --no-tests --license apache2
  quanta_occipita -m QuantaGlia --dry-run
  quanta_occipita -m QuantaSensa --author "Ada Lovelace" --email ada@quanta.dev
)";
}
