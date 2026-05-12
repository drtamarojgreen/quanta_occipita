#include "../../src/cli.hpp"
#include "../mini_test.hpp"
#include <vector>

TEST(CLIBasicParsing) {
    CLI cli;
    char* argv[] = {(char*)"quanta", (char*)"--module", (char*)"TestMod", (char*)"--dry-run"};
    Config cfg = cli.parse(4, argv);

    EXPECT_STR_EQ(cfg.moduleName, "TestMod");
    EXPECT_TRUE(cfg.dryRun);
}

TEST(CLIHelpFlag) {
    CLI cli;
    char* argv[] = {(char*)"quanta", (char*)"--help"};
    Config cfg = cli.parse(2, argv);
    EXPECT_TRUE(cfg.showHelp);
}

TEST(CLIVersionFlag) {
    CLI cli;
    char* argv[] = {(char*)"quanta", (char*)"-v"};
    Config cfg = cli.parse(2, argv);
    EXPECT_TRUE(cfg.showVersion);
}

TEST(CLIValidation) {
    CLI cli;
    EXPECT_TRUE(cli.validateModuleName("Valid_Name_123"));
    EXPECT_TRUE(!cli.validateModuleName("123Invalid"));
    EXPECT_TRUE(!cli.validateModuleName("Invalid-Name"));
}

int main() {
    return MiniTest::runAll();
}
