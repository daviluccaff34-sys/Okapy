#include <okapy/dependency_manager.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void addEnabled(okapy::ModRegistry& registry, std::string id,
                std::string version = "1.0.0") {
    okapy::ModInfo mod;
    mod.id = std::move(id);
    mod.name = mod.id;
    mod.version = std::move(version);
    mod.state = okapy::ModState::Enabled;
    expect(registry.add(std::move(mod)) == okapy::RegistryResult::Added,
           "test mod should be added");
}

bool hasIssue(const okapy::DependencyReport& report,
              okapy::DependencyIssueCode code) {
    for (const auto& issue : report.issues) {
        if (issue.code == code) return true;
    }
    return false;
}

} // namespace

int main() {
    using namespace okapy;
    const DependencyManager manager;

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        addEnabled(registry, "library");
        const auto report = manager.resolve(registry, {
            {"app", {{"library", "1.0.0", false}}, {}},
            {"library", {}, {}}
        });
        expect(!report.hasErrors(), "valid dependency graph should have no errors");
        expect(report.loadOrder.size() == 2, "both enabled mods should be ordered");
        expect(report.loadOrder[0] == "library" && report.loadOrder[1] == "app",
               "dependencies should load before dependants");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        const auto report = manager.resolve(registry, {
            {"app", {{"missing", "", false}}, {}}
        });
        expect(hasIssue(report, DependencyIssueCode::MissingDependency),
               "missing required dependency should be reported");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        addEnabled(registry, "library", "1.2.0");
        const auto report = manager.resolve(registry, {
            {"app", {{"library", "2.0.0", false}}, {}}
        });
        expect(hasIssue(report, DependencyIssueCode::VersionMismatch),
               "too-old dependency should be reported");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        addEnabled(registry, "library");
        const auto report = manager.resolve(registry, {
            {"app", {}, {"library"}}
        });
        expect(hasIssue(report, DependencyIssueCode::DeclaredConflict),
               "declared conflict should be reported");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "alpha");
        addEnabled(registry, "beta");
        const auto report = manager.resolve(registry, {
            {"alpha", {{"beta", "", false}}, {}},
            {"beta", {{"alpha", "", false}}, {}}
        });
        expect(hasIssue(report, DependencyIssueCode::DependencyCycle),
               "dependency cycle should be reported");
        expect(report.loadOrder.empty(), "cyclic graph must not produce load order");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        const auto report = manager.resolve(registry, {
            {"app", {{"optional-lib", "", true}}, {}}
        });
        expect(!report.hasErrors(), "missing optional dependency should not block");
        expect(report.loadOrder.size() == 1, "app should remain loadable");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        addEnabled(registry, "library", "1.10.0");
        const auto report = manager.resolve(registry, {
            {"app", {{"library", "1.9.0", false}}, {}}
        });
        expect(!report.hasErrors(), "numeric version comparison must not be lexical");
    }

    {
        ModRegistry registry;
        addEnabled(registry, "app");
        const auto report = manager.resolve(registry, {
            {"app", {{"library", "01.0.0", false}}, {}}
        });
        expect(hasIssue(report, DependencyIssueCode::InvalidVersion),
               "malformed semantic version should be reported");
    }

    std::cout << "All DependencyManager tests passed.\n";
    return EXIT_SUCCESS;
}
