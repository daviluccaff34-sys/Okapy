#pragma once

#include <okapy/mod_registry.hpp>

#include <string>
#include <vector>

namespace okapy {

struct ModDependency {
    std::string id;
    // Inclusive minimum in MAJOR.MINOR.PATCH form. Empty means any version.
    std::string minimumVersion;
    bool optional = false;
};

struct ModManifest {
    std::string id;
    std::vector<ModDependency> dependencies;
    std::vector<std::string> conflicts;
};

enum class DependencyIssueCode {
    MissingDependency,
    VersionMismatch,
    DeclaredConflict,
    DependencyCycle,
    InvalidVersion,
    DuplicateManifest
};

struct DependencyIssue {
    DependencyIssueCode code;
    std::string modId;
    std::string relatedModId;
    std::string detail;
};

struct DependencyReport {
    // Enabled mods in dependency-first load order. Empty if a cycle exists.
    std::vector<std::string> loadOrder;
    std::vector<DependencyIssue> issues;

    [[nodiscard]] bool hasErrors() const noexcept;
    [[nodiscard]] bool hasCycles() const noexcept;
};

// Resolves metadata only. It never loads libraries, executes mods or changes registry state.
class DependencyManager final {
public:
    [[nodiscard]] DependencyReport resolve(
        const ModRegistry& registry,
        const std::vector<ModManifest>& manifests) const;
};

} // namespace okapy
