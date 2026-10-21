#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace MessageProtocol {
enum class Command { State, Current, Target, Custom, Battery };
enum class Operation { Override, Current, Target, Battery };
struct Request { Operation operation; uint8_t current = 6, target = 6, battery = 0; };
inline std::optional<uint8_t> number(std::string_view text) {
    if (text.empty() || text.size() > 3) return {};
    unsigned n = 0;
    for (char c : text) {
        if (c < '0' || c > '9') return {};
        n = n * 10 + (c - '0');
    }
    return n <= 255 ? std::optional<uint8_t>(n) : std::nullopt;
}
inline std::optional<Request> decode(Command command, std::string_view payload,
                                     const std::array<uint8_t, 6>& custom) {
    auto n = number(payload);
    if (!n) return {};
    auto v = *n;
    switch (command) {
    case Command::State:
        if (v <= 1) return Request{Operation::Override, v, v};
        break;
    case Command::Current:
        if (v <= 3) return Request{Operation::Current, v};
        break;
    case Command::Target:
        if (v <= 1) return Request{Operation::Target, 6, v};
        break;
    case Command::Battery:
        if (v <= 100) return Request{Operation::Battery, 6, 6, v};
        break;
    case Command::Custom:
        // Preserve the existing MQTT precedence for duplicate custom mappings.
        if (v == custom[1]) return Request{Operation::Target, 6, 0};
        if (v == custom[3]) return Request{Operation::Target, 6, 1};
        if (v == custom[2]) return Request{Operation::Override, 0, 0};
        if (v == custom[0]) return Request{Operation::Override, 1, 1};
        if (v == custom[4]) return Request{Operation::Override, 2, 6};
        if (v == custom[5]) return Request{Operation::Override, 3, 6};
    }
    return {};
}
inline unsigned stateValue(unsigned current, unsigned target) {
    return current == target ? current : target == 0 ? 4 : 5;
}
inline std::string encode(std::string_view input) {
    constexpr char hex[] = "0123456789ABCDEF";
    std::string result;
    for (unsigned char c : input) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') result += c;
        else { result += '%'; result += hex[c >> 4]; result += hex[c & 15]; }
    }
    return result;
}
inline bool decodeUrl(std::string_view input, std::string& output) {
    auto hex = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    output.clear();
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = input[i];
        if (c == '%') {
            if (i + 2 >= input.size()) return false;
            int a = hex(input[i + 1]), b = hex(input[i + 2]);
            if (a < 0 || b < 0) return false;
            c = (a << 4) | b; i += 2;
        } else if (c == '+') c = ' ';
        if (c < 32 || c == 127) return false;
        output += c;
    }
    return true;
}
inline std::string expand(std::string_view pattern, std::string_view topic, std::string_view payload) {
    std::string result;
    // Single pass: braces contained in payloads are data, never nested templates.
    for (size_t i = 0; i < pattern.size();) {
        if (pattern.substr(i, 7) == "{topic}") { result += encode(topic); i += 7; }
        else if (pattern.substr(i, 9) == "{payload}") { result += encode(payload); i += 9; }
        else result += pattern[i++];
    }
    return result;
}
}
