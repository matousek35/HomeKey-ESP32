#pragma once
#include <cstdint>
#include <string_view>
namespace HttpNotificationPolicy {
inline int lockEvent(uint8_t current, uint8_t previous) {
    return current <= 1 && current != previous ? current + 1 : -1;
}
inline bool expired(int64_t nowUs, int64_t queuedUs) {
    return nowUs - queuedUs > 5000000;
}
inline bool validUrl(std::string_view url) {
    if (url.empty()) return true; // Disabled endpoint.
    size_t prefix = url.substr(0, 8) == "https://" ? 8 : url.substr(0, 7) == "http://" ? 7 : 0;
    if (!prefix || url.size() <= prefix || url.size() > 512 ||
        url.find_first_of("\r\n\t #") != std::string_view::npos) return false;
    auto authority = url.substr(prefix, url.find_first_of("/?", prefix) - prefix);
    return !authority.empty() && authority.find('@') == std::string_view::npos;
}
}
