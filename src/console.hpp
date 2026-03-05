#pragma once
#include <string>
#include <iostream>

#define QUANTA_VERSION "1.1.0"

namespace Console {

    enum class Color { Default, Red, Green, Yellow, Cyan, Magenta, Bold };

    inline std::string colorCode(Color c) {
        switch (c) {
            case Color::Red:     return "\033[31m";
            case Color::Green:   return "\033[32m";
            case Color::Yellow:  return "\033[33m";
            case Color::Cyan:    return "\033[36m";
            case Color::Magenta: return "\033[35m";
            case Color::Bold:    return "\033[1m";
            default:             return "\033[0m";
        }
    }

    inline void print(const std::string& msg, Color c = Color::Default) {
        std::cout << colorCode(c) << msg << "\033[0m" << "\n";
    }

    inline void success(const std::string& msg) {
        std::cout << "\033[32m[✓] " << msg << "\033[0m\n";
    }

    inline void warn(const std::string& msg) {
        std::cout << "\033[33m[!] " << msg << "\033[0m\n";
    }

    inline void error(const std::string& msg) {
        std::cerr << "\033[31m[✗] " << msg << "\033[0m\n";
    }

    inline void info(const std::string& msg) {
        std::cout << "\033[36m[→] " << msg << "\033[0m\n";
    }

    inline void printBanner() {
        std::cout << "\033[35m\033[1m";
        std::cout << R"(
  ___                  _        ___           _       _ _
 / _ \ _   _  __ _ _ _| |_ __ _/ _ \ ___ ___(_)_ __ (_) |_ __ _
| | | | | | |/ _` | '_ \ __/ _` | | | / __/ __| | '_ \| | __/ _` |
| |_| | |_| | (_| | | | | || (_| | |_| | (_| (__| | |_) | | || (_| |
 \__\_\\__,_|\__,_|_| |_|\__\__,_|\___/ \___\___|_| .__/|_|\__\__,_|
                                                   |_|
)" << "\033[0m";
        std::cout << "\033[36m  QuantaSoft Module Bootstrapper v" << QUANTA_VERSION << "\033[0m\n\n";
    }
}
