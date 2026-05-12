#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>

namespace MiniTest {
    struct Test {
        std::string name;
        std::function<void()> func;
    };

    inline std::vector<Test>& getTests() {
        static std::vector<Test> tests;
        return tests;
    }

    inline void registerTest(const std::string& name, std::function<void()> func) {
        getTests().push_back({name, func});
    }

    inline int runAll() {
        int passed = 0;
        int failed = 0;
        for (const auto& t : getTests()) {
            std::cout << "[ RUN      ] " << t.name << "\n";
            try {
                t.func();
                std::cout << "[       OK ] " << t.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cerr << "[  FAILED  ] " << t.name << ": " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cerr << "[  FAILED  ] " << t.name << ": Unknown error\n";
                failed++;
            }
        }
        std::cout << "\nPASSED: " << passed << "\nFAILED: " << failed << "\n";
        return failed == 0 ? 0 : 1;
    }
}

#define TEST(name) \
    void name(); \
    static int dummy_##name = (MiniTest::registerTest(#name, name), 0); \
    void name()

#define EXPECT_TRUE(cond) if (!(cond)) throw std::runtime_error("Condition failed: " #cond)
#define EXPECT_EQ(a, b) if ((a) != (b)) throw std::runtime_error("Equality failed: " + std::to_string(a) + " != " + std::to_string(b))
#define EXPECT_STR_EQ(a, b) if (std::string(a) != std::string(b)) throw std::runtime_error("String equality failed: " + std::string(a) + " != " + std::string(b))
