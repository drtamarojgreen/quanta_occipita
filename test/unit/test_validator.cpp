#include "../../src/components/validator.hpp"
#include "../mini_test.hpp"

TEST(ValidatorCompilerCheck) {
    Config cfg;
    Validator val(cfg);
    EXPECT_TRUE(val.checkCompiler());
}

TEST(ValidatorAdminCheck) {
    Config cfg;
    Validator val(cfg);
    // Should be false in most CI environments
    EXPECT_TRUE(!val.checkAdmin());
}

int main() {
    return MiniTest::runAll();
}
