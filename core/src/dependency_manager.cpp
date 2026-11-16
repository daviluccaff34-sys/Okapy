#include <okapy/dependency_manager.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace okapy {
namespace {

struct Version {
    std::uint64_t major = 0;
    std::uint64_t minor = 0;
    std::uint64_t patch = 0;
};

bool parseNumber(const std::string& text, std::size_t& position,
                 std::uint64_t& value) {
    const auto start = position;
    while (position < text.size() && text[position] >= '0' &&
           text[position] <= '9') {
        ++position;
    }
    if (start == position || (position - start > 1 && text[start] == '0')) {
        return false;
    }

    value = 0;
    for (std::size_t i = start; i < position; ++i) {
        const auto digit = static_cast<std::uint64_t>(text[i] - '0');
        if (value > (UINT64_MAX - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    return true;
}

bool parseVersion(const std::string& text, Version& version) {
    std::size_t position = 0;
    if (!parseNumber(text, position, version.major) ||
        position >= text.size() || text[position++] != '.' ||
        !parseNumber(text, position, version.minor) ||
        position >= text.size() || text[position++] != '.' ||
        !parseNumber(text, position, version.patch)) {
        return false;
    }
    return position == text.size();
}

bool versionAtLeast(const Version& actual, const Version& minimum) {
    if (actual.major != minimum.major) return actual.major > minimum.major;
    if (actual.minor != minimum.minor) return actual.minor > minimum.minor;
    return actual.patch >= minimum.patch;
}

} // namespace

bool DependencyReport::hasErrors() const noexcept {
    return !issues.empty();
}

bool DependencyReport::hasCycles() const noexcept {
    return std::any_of(issues.begin(), issues.end(),
        [](const DependencyIssue& issue) {
            return issue.code == DependencyIssueCode::DependencyCycle;
        });
}

DependencyReport DependencyManager::resolve(
    const ModRegistry& registry,
    const std::vector<ModManifest>& manifests) const {
    DependencyReport report;
    const auto allMods = registry.list();

    std::unordered_map<std::string, ModInfo> enabled;
    for (const auto& mod : allMods) {
        if (mod.state == ModState::Enabled) {
            enabled.emplace(mod.id, mod);
        }
    }

    std::unordered_map<std::string, const ModManifest*> manifestById;
    for (const auto& manifest : manifests) {
        if (manifestById.find(manifest.id) != manifestById.end()) {
            report.issues.push_back({DependencyIssueCode::DuplicateManifest,
                manifest.id, {}, "More than one manifest was supplied for this mod."});
            continue;
        }
        manifestById.emplace(manifest.id, &manifest);
    }

    // Validate dependencies and build a deterministic graph.
    std::unordered_map<std::string, std::vector<std::string>> edges;
    for (const auto& entry : enabled) {
        const auto manifestIt = manifestById.find(entry.first);
        if (manifestIt == manifestById.end()) continue;

        for (const auto& dependency : manifestIt->second->dependencies) {
            const auto target = enabled.find(dependency.id);
            if (target == enabled.end()) {
                if (!dependency.optional) {
                    report.issues.push_back({DependencyIssueCode::MissingDependency,
                        entry.first, dependency.id,
                        "Required dependency is not enabled or installed."});
                }
                continue;
            }

            if (!dependency.minimumVersion.empty()) {
                Version minimum;
                Version actual;
                if (!parseVersion(dependency.minimumVersion, minimum) ||
                    !parseVersion(target->second.version, actual)) {
                    report.issues.push_back({DependencyIssueCode::InvalidVersion,
                        entry.first, dependency.id,
                        "Versions must use MAJOR.MINOR.PATCH without leading zeroes."});
                    continue;
                }
                if (!versionAtLeast(actual, minimum)) {
                    report.issues.push_back({DependencyIssueCode::VersionMismatch,
                        entry.first, dependency.id,
                        "Installed dependency version is below the inclusive minimum."});
                    continue;
                }
            }
            edges[entry.first].push_back(dependency.id);
        }
    }

    // Conflicts are checked in either direction, once per pair.
    std::unordered_set<std::string> checkedPairs;
    for (const auto& entry : enabled) {
        const auto manifestIt = manifestById.find(entry.first);
        if (manifestIt == manifestById.end()) continue;
        for (const auto& conflictId : manifestIt->second->conflicts) {
            if (enabled.find(conflictId) == enabled.end()) continue;
            const auto first = std::min(entry.first, conflictId);
            const auto second = std::max(entry.first, conflictId);
            const auto key = first + "\n" + second;
            if (checkedPairs.insert(key).second) {
                report.issues.push_back({DependencyIssueCode::DeclaredConflict,
                    first, second, "Both mods are enabled but declare a conflict."});
            }
        }
    }

    for (auto& edge : edges) {
        std::sort(edge.second.begin(), edge.second.end());
        edge.second.erase(std::unique(edge.second.begin(), edge.second.end()),
                          edge.second.end());
    }

    // DFS over sorted IDs gives stable dependency-first ordering.
    enum class Visit { Unvisited, Visiting, Visited };
    std::unordered_map<std::string, Visit> visits;
    for (const auto& entry : enabled) visits.emplace(entry.first, Visit::Unvisited);

    bool cycleFound = false;
    std::function<void(const std::string&)> visit = [&](const std::string& id) {
        auto& state = visits[id];
        if (state == Visit::Visited) return;
        if (state == Visit::Visiting) {
            cycleFound = true;
            return;
        }
        state = Visit::Visiting;
        const auto edgeIt = edges.find(id);
        if (edgeIt != edges.end()) {
            for (const auto& dependencyId : edgeIt->second) {
                if (visits[dependencyId] == Visit::Visiting) {
                    cycleFound = true;
                    report.issues.push_back({DependencyIssueCode::DependencyCycle,
                        id, dependencyId, "Dependency graph contains a cycle."});
                } else if (visits[dependencyId] == Visit::Unvisited) {
                    visit(dependencyId);
                }
            }
        }
        state = Visit::Visited;
        report.loadOrder.push_back(id);
    };

    std::vector<std::string> ids;
    ids.reserve(enabled.size());
    for (const auto& entry : enabled) ids.push_back(entry.first);
    std::sort(ids.begin(), ids.end());
    for (const auto& id : ids) {
        if (visits[id] == Visit::Unvisited) visit(id);
    }

    if (cycleFound || report.hasErrors()) report.loadOrder.clear();
    return report;
}

} // namespace okapy
