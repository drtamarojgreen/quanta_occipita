#pragma once
#include "../cli.hpp"
#include "../console.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

class Skeletons {
public:
    explicit Skeletons(const Config& cfg) : cfg_(cfg) {}

    void genFactory(const fs::path& root);
    void genObserver(const fs::path& root);
    void genAdaptiveSkeletons(const fs::path& root);

private:
    void writeFile(const fs::path& p, const std::string& content);
    const Config& cfg_;
};

inline void Skeletons::writeFile(const fs::path& p, const std::string& content) {
    if (cfg_.dryRun) return;
    if (fs::exists(p) && !cfg_.force) return;
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    if (f) f << content;
}

inline void Skeletons::genFactory(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "// Factory Pattern Skeleton\n#pragma once\n#include <memory>\n#include \"" + M + "." + h_ext + "\"\n\n";
    h += "namespace " + M + " {\n\n";
    h += "class " + M + "Factory {\npublic:\n";
    h += "    static std::unique_ptr<" + M + "> create(const std::string& type);\n";
    h += "};\n\n} // namespace " + M + "\n";

    writeFile(root / "src" / (M + "Factory." + h_ext), h);
}

inline void Skeletons::genObserver(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "// Observer Pattern Skeleton\n#pragma once\n#include <vector>\n\n";
    h += "namespace " + M + " {\n\n";
    h += "class " + M + "Observer {\npublic:\n";
    h += "    virtual ~" + M + "Observer() = default;\n";
    h += "    virtual void onUpdate() = 0;\n";
    h += "};\n\n";
    h += "class " + M + "Subject {\npublic:\n";
    h += "    void addObserver(" + M + "Observer* o) { observers_.push_back(o); }\n";
    h += "    void notify() { for (auto o : observers_) o->onUpdate(); }\n";
    h += "private:\n    std::vector<" + M + "Observer*> observers_;\n";
    h += "};\n\n} // namespace " + M + "\n";

    writeFile(root / "src" / (M + "Observer." + h_ext), h);
}

inline void Skeletons::genAdaptiveSkeletons(const fs::path& root) {
    const std::string& M = cfg_.moduleName;
    const std::string& ext = cfg_.extension;
    const std::string& h_ext = (ext == "cc") ? "hh" : "hpp";

    std::string h = "#pragma once\n#include <string>\n\nnamespace " + M + " {\n\n";
    h += "/**\n * @brief Integrates with QuantaGlia for knowledge updates.\n */\n";
    h += "class QuantaGliaIntegrator {\npublic:\n";
    h += "    void updateFromKnowledge() {\n        // Logic to poll QuantaGlia for domain updates\n    }\n";
    h += "};\n\n";
    h += "/**\n * @brief Handles scheduling via QuantaParent.\n */\n";
    h += "class QuantaParentScheduler {\npublic:\n";
    h += "    void schedule(const std::string& task) {\n        // Integration logic with QuantaParent scheduler\n    }\n";
    h += "};\n\n} // namespace " + M + "\n";
    writeFile(root / "src" / ("AdaptiveSkeletons." + h_ext), h);
}
