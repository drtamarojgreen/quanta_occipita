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
    std::string extension     = "cpp";       // item 28
    std::string templatePath;                // item 91
    std::string copyrightHolder;             // item 94
    std::string profile;                     // item 97
    std::string bannerText;                  // item 98
    std::string lang          = "cpp";       // support more languages
    std::string gitignorePath;               // item 99
    std::string remoteName    = "origin";    // item 55

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
    bool singleton    = false;     // generate singleton class
    bool pimpl        = false;     // use PIMPL idiom
    bool jsonConfig   = false;     // use JSON instead of YAML for config

    bool verbose      = false;               // log toggles
    bool progress     = false;               // item 12
    bool update       = false;               // item 13
    bool eject        = false;               // item 14
    bool factory      = false;               // item 22
    bool observer     = false;               // item 32
    bool shared       = false;               // item 34
    bool headerOnly   = false;               // item 30
    bool newBranch    = false;               // item 47
    bool push         = false;               // item 49
    bool tag          = false;               // item 53
    bool noUpdateCheck= false;               // item 100
    bool snippets     = false;               // item 86
    bool hooks        = false;               // item 87
    bool issueTemplates= false;              // item 90
    bool customFiles  = false;               // item 95
    bool plugins      = false;               // item 96
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
    if (hasFlag(args, "--list-templates"))                  { cfg.listTemplates = true; return cfg; }

    cfg.moduleName    = getArg(args, "--module",      getArg(args, "-m", cfg.moduleName));
    cfg.outputPath    = getArg(args, "--output",      getArg(args, "-o", cfg.outputPath));
    cfg.license       = getArg(args, "--license",     cfg.license);
    cfg.authorName    = getArg(args, "--author",      cfg.authorName);
    cfg.authorEmail   = getArg(args, "--email",       cfg.authorEmail);
    cfg.initialBranch = getArg(args, "--branch",      cfg.initialBranch);
    cfg.commitMessage = getArg(args, "--message",     getArg(args, "-M", ""));
    cfg.cppStd        = getArg(args, "--cpp-std",     cfg.cppStd);
    cfg.extension     = getArg(args, "--extension",   cfg.extension);
    cfg.templatePath  = getArg(args, "--template-path", cfg.templatePath);
    cfg.copyrightHolder = getArg(args, "--copyright", cfg.copyrightHolder);
    cfg.profile       = getArg(args, "--profile",     cfg.profile);
    cfg.bannerText    = getArg(args, "--banner-text", cfg.bannerText);
    cfg.lang          = getArg(args, "--lang",        cfg.lang);
    cfg.gitignorePath = getArg(args, "--gitignore-path", cfg.gitignorePath);
    cfg.remoteName    = getArg(args, "--remote-name", cfg.remoteName);

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
    cfg.singleton     = hasFlag(args, "--singleton");
    cfg.pimpl         = hasFlag(args, "--pimpl");
    cfg.jsonConfig    = hasFlag(args, "--json");

    cfg.verbose       = hasFlag(args, "--verbose");
    cfg.progress      = hasFlag(args, "--progress");
    cfg.update        = hasFlag(args, "--update");
    cfg.eject         = hasFlag(args, "--eject");
    cfg.factory       = hasFlag(args, "--factory");
    cfg.observer      = hasFlag(args, "--observer");
    cfg.shared        = hasFlag(args, "--shared");
    cfg.headerOnly    = hasFlag(args, "--header-only");
    cfg.newBranch     = hasFlag(args, "--new-branch");
    cfg.push          = hasFlag(args, "--push");
    cfg.tag           = hasFlag(args, "--tag");
    cfg.noUpdateCheck = hasFlag(args, "--no-update-check");
    cfg.snippets      = hasFlag(args, "--snippets");
    cfg.hooks         = hasFlag(args, "--hooks");
    cfg.issueTemplates= hasFlag(args, "--issue-templates");
    cfg.customFiles   = hasFlag(args, "--custom-files");
    cfg.plugins       = hasFlag(args, "--plugins");

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
  --singleton              Generate singleton class pattern
  --pimpl                  Generate PIMPL idiom skeleton
  --factory                Generate factory pattern skeleton
  --observer               Generate observer pattern skeleton
  --header-only            Generate header-only library
  --shared                 Generate shared library in CMakeLists.txt
  --json                   Generate config.json instead of config.yaml
  --license      <type>    License type: mit (default), apache2, gpl3, none
  --cpp-std      <ver>     C++ standard version (default: 17)
  --extension    <ext>     File extension: cpp (default) or cc
  --lang         <lang>    Language skeleton: cpp (default), python, rust, go

GIT
  --skip-git               Disable all git actions
  --branch       <name>    Initial branch name (default: main)
  --message, -M  <msg>     Custom initial commit message
  --new-branch             Automatically create a new branch
  --push                   Automatically push to remote
  --tag                    Automatically tag the initial commit
  --remote-name  <name>    Git remote name (default: origin)
  --gitignore-path <path>  Path or URL to a custom .gitignore

EXTENSIBILITY
  --template-path <path>   Path to custom templates
  --snippets               Use custom code snippets
  --hooks                  Run post-generation hooks
  --issue-templates        Use custom issue templates
  --custom-files           Copy custom files into the module
  --plugins                Enable plugin system
  --profile      <name>    Use a specific project profile
  --copyright    <name>    Custom copyright holder name

BEHAVIOUR
  --dry-run                Preview actions without writing files
  --force,   -f            Overwrite existing files
  --quiet / --silent       Suppress non-essential output
  --verbose                Show detailed output
  --progress               Show progress indicators
  --no-banner              Hide ASCII banner
  --banner-text  <text>    Custom banner text
  --report                 Generate quanta_report.txt after bootstrapping
  --list-templates         Show available module templates
  --update                 Update an existing module
  --eject                  Remove QuantaOccipita-specific files
  --no-update-check        Disable online version check

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
