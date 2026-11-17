#include <okapy/package_manifest.hpp>

#include <okapy/mod_registry.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>

namespace okapy {
namespace {

struct JsonNumber { std::string text; };
struct JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;
struct JsonValue {
    using Data = std::variant<std::nullptr_t, bool, JsonNumber, std::string, JsonArray, JsonObject>;
    Data data;
};

class JsonParser final {
public:
    explicit JsonParser(std::string_view input) : m_input(input) {}

    bool parse(JsonValue& output, std::string& error) {
        skipWhitespace();
        if (!parseValue(output, 0)) {
            error = m_error.empty() ? "Malformed JSON." : m_error;
            return false;
        }
        skipWhitespace();
        if (m_position != m_input.size()) {
            error = "Unexpected data after the JSON value.";
            return false;
        }
        return true;
    }

private:
    bool fail(const char* message) {
        if (m_error.empty()) m_error = message;
        return false;
    }

    void skipWhitespace() {
        while (m_position < m_input.size()) {
            const char c = m_input[m_position];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
            ++m_position;
        }
    }

    bool consume(char expected) {
        if (m_position >= m_input.size() || m_input[m_position] != expected) return false;
        ++m_position;
        return true;
    }

    static void appendUtf8(std::string& output, std::uint32_t codepoint) {
        if (codepoint <= 0x7f) {
            output.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7ff) {
            output.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0xffff) {
            output.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            output.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
    }

    bool hex4(std::uint32_t& value) {
        if (m_input.size() - m_position < 4) return false;
        value = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = m_input[m_position++];
            value <<= 4;
            if (c >= '0' && c <= '9') value |= static_cast<std::uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<std::uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<std::uint32_t>(c - 'A' + 10);
            else return false;
        }
        return true;
    }

    bool parseString(std::string& output) {
        if (!consume('"')) return false;
        while (m_position < m_input.size()) {
            const unsigned char c = static_cast<unsigned char>(m_input[m_position++]);
            if (c == '"') return true;
            if (c < 0x20) return fail("Unescaped control character in JSON string.");
            if (c != '\\') {
                output.push_back(static_cast<char>(c));
                continue;
            }
            if (m_position >= m_input.size()) return fail("Incomplete JSON escape.");
            const char escaped = m_input[m_position++];
            switch (escaped) {
                case '"': output.push_back('"'); break;
                case '\\': output.push_back('\\'); break;
                case '/': output.push_back('/'); break;
                case 'b': output.push_back('\b'); break;
                case 'f': output.push_back('\f'); break;
                case 'n': output.push_back('\n'); break;
                case 'r': output.push_back('\r'); break;
                case 't': output.push_back('\t'); break;
                case 'u': {
                    std::uint32_t codepoint = 0;
                    if (!hex4(codepoint)) return fail("Invalid Unicode escape.");
                    if (codepoint >= 0xd800 && codepoint <= 0xdbff) {
                        if (m_input.size() - m_position < 6 ||
                            m_input[m_position] != '\\' || m_input[m_position + 1] != 'u') {
                            return fail("Unpaired high surrogate in JSON string.");
                        }
                        m_position += 2;
                        std::uint32_t low = 0;
                        if (!hex4(low) || low < 0xdc00 || low > 0xdfff) {
                            return fail("Invalid low surrogate in JSON string.");
                        }
                        codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (low - 0xdc00);
                    } else if (codepoint >= 0xdc00 && codepoint <= 0xdfff) {
                        return fail("Unpaired low surrogate in JSON string.");
                    }
                    appendUtf8(output, codepoint);
                    break;
                }
                default: return fail("Unknown JSON escape.");
            }
        }
        return fail("Unterminated JSON string.");
    }

    bool parseNumber(JsonValue& output) {
        const auto start = m_position;
        consume('-');
        if (m_position >= m_input.size()) return fail("Invalid JSON number.");
        if (m_input[m_position] == '0') {
            ++m_position;
            if (m_position < m_input.size() && std::isdigit(static_cast<unsigned char>(m_input[m_position]))) {
                return fail("Leading zero in JSON number.");
            }
        } else {
            if (m_input[m_position] < '1' || m_input[m_position] > '9') return fail("Invalid JSON number.");
            while (m_position < m_input.size() && std::isdigit(static_cast<unsigned char>(m_input[m_position]))) ++m_position;
        }
        if (consume('.')) {
            const auto fraction = m_position;
            while (m_position < m_input.size() && std::isdigit(static_cast<unsigned char>(m_input[m_position]))) ++m_position;
            if (fraction == m_position) return fail("Invalid JSON fraction.");
        }
        if (m_position < m_input.size() && (m_input[m_position] == 'e' || m_input[m_position] == 'E')) {
            ++m_position;
            if (m_position < m_input.size() && (m_input[m_position] == '+' || m_input[m_position] == '-')) ++m_position;
            const auto exponent = m_position;
            while (m_position < m_input.size() && std::isdigit(static_cast<unsigned char>(m_input[m_position]))) ++m_position;
            if (exponent == m_position) return fail("Invalid JSON exponent.");
        }
        output.data = JsonNumber{std::string(m_input.substr(start, m_position - start))};
        return true;
    }

    bool parseValue(JsonValue& output, unsigned depth) {
        if (depth > 32) return fail("JSON nesting limit exceeded.");
        skipWhitespace();
        if (m_position >= m_input.size()) return fail("Unexpected end of JSON.");
        const char c = m_input[m_position];
        if (c == '"') {
            std::string value;
            if (!parseString(value)) return false;
            output.data = std::move(value);
            return true;
        }
        if (c == '{') {
            ++m_position;
            JsonObject object;
            skipWhitespace();
            if (consume('}')) { output.data = std::move(object); return true; }
            while (true) {
                skipWhitespace();
                std::string key;
                if (!parseString(key)) return fail("Expected an object key.");
                skipWhitespace();
                if (!consume(':')) return fail("Expected ':' after object key.");
                JsonValue value;
                if (!parseValue(value, depth + 1)) return false;
                if (!object.emplace(std::move(key), std::move(value)).second) {
                    return fail("Duplicate JSON object key.");
                }
                skipWhitespace();
                if (consume('}')) break;
                if (!consume(',')) return fail("Expected ',' or '}' in object.");
            }
            output.data = std::move(object);
            return true;
        }
        if (c == '[') {
            ++m_position;
            JsonArray array;
            skipWhitespace();
            if (consume(']')) { output.data = std::move(array); return true; }
            while (true) {
                JsonValue value;
                if (!parseValue(value, depth + 1)) return false;
                array.push_back(std::move(value));
                skipWhitespace();
                if (consume(']')) break;
                if (!consume(',')) return fail("Expected ',' or ']' in array.");
            }
            output.data = std::move(array);
            return true;
        }
        if (m_input.substr(m_position, 4) == "true") {
            m_position += 4; output.data = true; return true;
        }
        if (m_input.substr(m_position, 5) == "false") {
            m_position += 5; output.data = false; return true;
        }
        if (m_input.substr(m_position, 4) == "null") {
            m_position += 4; output.data = nullptr; return true;
        }
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(output);
        return fail("Invalid JSON value.");
    }

    std::string_view m_input;
    std::size_t m_position = 0;
    std::string m_error;
};

const JsonValue* member(const JsonObject& object, const char* key) {
    const auto found = object.find(key);
    return found == object.end() ? nullptr : &found->second;
}

const std::string* stringValue(const JsonValue* value) {
    return value ? std::get_if<std::string>(&value->data) : nullptr;
}

const JsonObject* objectValue(const JsonValue* value) {
    return value ? std::get_if<JsonObject>(&value->data) : nullptr;
}

const JsonArray* arrayValue(const JsonValue* value) {
    return value ? std::get_if<JsonArray>(&value->data) : nullptr;
}

bool validPackageRelativePath(const std::string& path) {
    if (path.empty() || path.front() == '/' || path.front() == '\\' ||
        path.find('\\') != std::string::npos || path.find(':') != std::string::npos) return false;
    std::size_t start = 0;
    while (start <= path.size()) {
        const auto end = path.find('/', start);
        const auto part = path.substr(start, end == std::string::npos ? end : end - start);
        if (part.empty() || part == "." || part == "..") return false;
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return true;
}

bool readStringArray(const JsonValue* value, std::vector<std::string>& output,
                     const std::string& field, std::string& error) {
    if (!value) return true;
    const auto* array = arrayValue(value);
    if (!array) { error = field + " must be an array of strings."; return false; }
    for (const auto& item : *array) {
        const auto* text = stringValue(&item);
        if (!text || !ModRegistry::isValidId(*text)) {
            error = field + " contains an invalid mod ID.";
            return false;
        }
        output.push_back(*text);
    }
    std::sort(output.begin(), output.end());
    if (std::adjacent_find(output.begin(), output.end()) != output.end()) {
        error = field + " contains duplicate IDs.";
        return false;
    }
    return true;
}

} // namespace

ParsedPackageManifest parsePackageManifest(std::string_view json, std::string packagePath) {
    ParsedPackageManifest result;
    if (json.empty() || json.size() > 64 * 1024) {
        result.error = "Manifest is empty or exceeds the 64 KiB limit.";
        return result;
    }

    JsonValue root;
    JsonParser parser(json);
    if (!parser.parse(root, result.error)) return result;
    const auto* object = objectValue(&root);
    if (!object) { result.error = "Manifest root must be a JSON object."; return result; }

    const auto* format = member(*object, "formatVersion");
    const auto* formatNumber = format ? std::get_if<JsonNumber>(&format->data) : nullptr;
    if (!formatNumber || formatNumber->text != "1") {
        result.error = "Unsupported or missing package formatVersion (expected integer 1).";
        return result;
    }

    const auto* id = stringValue(member(*object, "id"));
    const auto* name = stringValue(member(*object, "name"));
    const auto* version = stringValue(member(*object, "version"));
    const auto* developer = stringValue(member(*object, "developer"));
    if (!id || !ModRegistry::isValidId(*id)) { result.error = "Mod ID is missing or invalid."; return result; }
    if (!name || name->empty()) { result.error = "Mod name is missing."; return result; }
    if (!version || version->empty()) { result.error = "Mod version is missing."; return result; }
    if (!developer || developer->empty()) { result.error = "Developer name is missing."; return result; }

    const auto* game = objectValue(member(*object, "game"));
    if (!game) { result.error = "Game compatibility information is missing."; return result; }
    const auto* gameId = stringValue(member(*game, "id"));
    const auto* gameVersions = arrayValue(member(*game, "versions"));
    if (!gameId || gameId->empty() || !gameVersions || gameVersions->empty()) {
        result.error = "Game ID and at least one supported game version are required.";
        return result;
    }
    for (const auto& item : *gameVersions) {
        const auto* text = stringValue(&item);
        if (!text || text->empty()) { result.error = "Game versions must be non-empty strings."; return result; }
        result.gameVersions.push_back(*text);
    }

    const auto* entrypoints = objectValue(member(*object, "entrypoints"));
    const auto* android = entrypoints ? objectValue(member(*entrypoints, "android")) : nullptr;
    if (!android || android->empty()) { result.error = "Android entrypoints are missing."; return result; }
    for (const auto& entry : *android) {
        const auto* path = stringValue(&entry.second);
        if ((entry.first != "arm64-v8a" && entry.first != "armeabi-v7a" &&
             entry.first != "x86" && entry.first != "x86_64") ||
            !path || !validPackageRelativePath(*path)) {
            result.error = "Android entrypoint ABI or package-relative path is invalid.";
            return result;
        }
        result.androidEntrypoints.emplace(entry.first, *path);
    }

    result.dependencies.id = *id;
    std::vector<std::string> dependencyIds;
    if (!readStringArray(member(*object, "dependencies"), dependencyIds,
                         "dependencies", result.error)) return result;
    for (const auto& dependencyId : dependencyIds) {
        if (dependencyId == *id) { result.error = "A mod cannot depend on itself."; return result; }
        result.dependencies.dependencies.push_back({dependencyId, {}, false});
    }
    if (!readStringArray(member(*object, "conflicts"), result.dependencies.conflicts,
                         "conflicts", result.error)) return result;
    for (const auto& conflict : result.dependencies.conflicts) {
        if (conflict == *id) { result.error = "A mod cannot conflict with itself."; return result; }
    }

    const auto* description = stringValue(member(*object, "description"));
    if (member(*object, "description") && !description) {
        result.error = "Description must be a string when present.";
        return result;
    }
    result.description = description ? *description : std::string{};
    result.gameId = *gameId;
    result.mod = ModInfo{*id, *name, *version, *developer,
                         std::move(packagePath), ModState::Discovered};
    result.valid = true;
    return result;
}

} // namespace okapy
