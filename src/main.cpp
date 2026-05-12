// QuantaOccipita - QuantaSoft Module Bootstrapper
// C++ Console Application | MIT License

#include "cli.hpp"
#include "bootstrapper.hpp"
#include "console.hpp"
#include <iostream>
#include <csignal>
#include <atomic>

namespace {
    std::atomic<bool> g_terminated{false};
}

void signalHandler(int signum) {
    (void)signum;
    g_terminated = true;
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

    if (cfg.dryRun) {
        Console::warn("--- Dry Run Mode: No files will be written ---");
    }

    bootstrapper.run();

    if (g_terminated) {
        bootstrapper.cleanup();
        return 1;
    }

    if (!cfg.quiet) {
        Console::success("QuantaOccipita bootstrap complete!");
    }

    return 0;
}
