#include "../../src/components/generators.hpp"
#include "../mini_test.hpp"
#include <vector>
#include <string>

TEST(SDDVirtualDestructor) {
    Config cfg;
    cfg.moduleName = "TestMod";
    std::vector<std::string> log;
    Generators gen(cfg, log);

    std::string header = gen.getHeaderString("Author", "2024");
    // Verify mandatory virtual destructor (Architectural Restriction)
    EXPECT_TRUE(header.find("virtual ~TestMod() = default;") != std::string::npos);
}

TEST(SDDPimplIdiom) {
    Config cfg;
    cfg.moduleName = "TestMod";
    cfg.pimpl = true;
    std::vector<std::string> log;
    Generators gen(cfg, log);

    std::string header = gen.getHeaderString("Author", "2024");
    // Verify PIMPL structure (Architectural Restriction)
    EXPECT_TRUE(header.find("struct Impl;") != std::string::npos);
    EXPECT_TRUE(header.find("std::unique_ptr<Impl> impl_;") != std::string::npos);
}

TEST(SDDHeaderGuard) {
    Config cfg;
    cfg.moduleName = "TestMod";
    cfg.headerGuard = true;
    cfg.extension = "cc";
    std::vector<std::string> log;
    Generators gen(cfg, log);

    std::string header = gen.getHeaderString("Author", "2024");
    // Verify custom header guard naming convention
    EXPECT_TRUE(header.find("#ifndef TESTMOD_HH") != std::string::npos);
    EXPECT_TRUE(header.find("#define TESTMOD_HH") != std::string::npos);
}

int main() {
    return MiniTest::runAll();
}
