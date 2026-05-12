// QuantaOccipita - QuantaSoft Module Bootstrapper
// C++ Console Application | MIT License

#include "cli.hpp"
#include "bootstrapper.hpp"
#include "console.hpp"
#include <iostream>
#include <csignal>

namespace {
    Bootstrapper* g_bootstrapper = nullptr;
}

void signalHandler(int signum) {
    Console::error("\nInterrupt received (signal " + std::to_string(signum) + ").");
    if (g_bootstrapper) {
        g_bootstrapper->cleanup();
    }
    std::exit(signum);
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    CLI cli;
    Config cfg = cli.parse(argc, argv);

    Console::setVerbosity(cfg.verbose);

    if (cfg.showHelp) {
        cli.printHelp();
        return 0;
    }
    if (cfg.showVersion) {
        Console::print(QUANTA_VERSION, Console::Color::Cyan);
        return 0;
    }
    if (cfg.listTemplates) {
        Bootstrapper bootstrapper(cfg);
        bootstrapper.run();
        return 0;
    }

    if (!cfg.noBanner) {
        Console::printBanner(cfg.bannerText);
    }

    if (cfg.moduleName.empty()) {
        cfg.moduleName = cli.promptModuleName();
    }

    if (!cli.validateModuleName(cfg.moduleName)) {
        Console::error("Invalid module name. Use alphanumeric characters and underscores only.");
        return 1;
    }

    Bootstrapper bootstrapper(cfg);
    g_bootstrapper = &bootstrapper;

    if (cfg.dryRun) {
        Console::warn("--- Dry Run Mode: No files will be written ---");
    }

    bootstrapper.run();

    if (!cfg.quiet) {
        Console::success("QuantaOccipita bootstrap complete!");
    }

    return 0;
}
