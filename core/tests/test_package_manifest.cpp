#include <okapy/package_manifest.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

const std::string validManifest = R"json({
  "formatVersion": 1,
  "id": "author.example-mod",
  "name": "Example Mod",
  "version": "1.0.0",
  "developer": "Example Author",
  "description": "A mod with \u00e7 and \ud83d\ude80",
  "game": {"id": "com.robtopx.geometryjump", "versions": ["2.2"]},
  "entrypoints": {"android": {"arm64-v8a": "libs/arm64-v8a/libexample.so"}},
  "dependencies": ["author.library"],
  "conflicts": ["author.incompatible"]
})json";
}

int main() {
    using namespace okapy;
    const auto parsed = parsePackageManifest(validManifest, "/mods/author.example-mod");
    expect(parsed.valid, "valid package manifest should parse");
    expect(parsed.mod.id == "author.example-mod", "mod ID should be mapped");
    expect(parsed.mod.name == "Example Mod", "mod name should be mapped");
    expect(parsed.mod.packagePath == "/mods/author.example-mod", "package path should be retained");
    expect(parsed.description.find("🚀") != std::string::npos, "surrogate pair should decode to UTF-8");
    expect(parsed.dependencies.dependencies.size() == 1, "dependency ID should be mapped");
    expect(parsed.dependencies.dependencies.front().id == "author.library", "dependency ID should match");
    expect(parsed.dependencies.conflicts.front() == "author.incompatible", "conflict ID should be mapped");
    expect(parsed.gameVersions.size() == 1 && parsed.gameVersions.front() == "2.2",
           "game versions should be mapped");
    expect(parsed.androidEntrypoints.at("arm64-v8a") == "libs/arm64-v8a/libexample.so",
           "Android entrypoint should be mapped");

    expect(!parsePackageManifest("{").valid, "malformed JSON should fail");
    expect(!parsePackageManifest(R"({"formatVersion":2})").valid,
           "unsupported format version should fail");
    expect(!parsePackageManifest(R"({"formatVersion":1,"formatVersion":1})").valid,
           "duplicate JSON keys should fail");
    expect(!parsePackageManifest(R"({"formatVersion":1,"id":"../escape"})").valid,
           "invalid mod ID should fail");
    expect(!parsePackageManifest(R"({"formatVersion":1,"id":"author.mod","name":"x","version":"1","developer":"x","game":{"id":"g","versions":["1"]},"entrypoints":{"android":{"arm64-v8a":"../evil.so"}}})").valid,
           "entrypoint traversal should fail");
    expect(!parsePackageManifest(R"({"formatVersion":1,"id":"author.mod","name":"x","version":"1","developer":"x","game":{"id":"g","versions":["1"]},"entrypoints":{"android":{"arm64-v8a":"lib.so"}},"dependencies":["../bad"]})").valid,
           "invalid dependency IDs should fail");
    expect(!parsePackageManifest(R"({"formatVersion":1,"id":"author.mod","name":"x","version":"1","developer":"x","game":{"id":"g","versions":["1"]},"entrypoints":{"android":{"arm64-v8a":"lib.so"}},"dependencies":["author.mod"]})").valid,
           "self dependency should fail");
    expect(!parsePackageManifest(std::string(65 * 1024, ' ')).valid,
           "manifest size limit should be enforced");

    std::cout << "All package manifest parser tests passed.\n";
    return EXIT_SUCCESS;
}
