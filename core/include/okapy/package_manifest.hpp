#pragma once

#include <okapy/dependency_manager.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace okapy {

struct ParsedPackageManifest {
    bool valid = false;
    std::string error;
    ModInfo mod;
    ModManifest dependencies;
    std::string description;
    std::string gameId;
    std::vector<std::string> gameVersions;
    std::unordered_map<std::string, std::string> androidEntrypoints;
};

// Parses the UTF-8 root okapy.json document. This validates manifest structure
// and package-relative paths; it does not inspect ZIP contents or load code.
[[nodiscard]] ParsedPackageManifest parsePackageManifest(
    std::string_view json, std::string packagePath = {});

} // namespace okapy
