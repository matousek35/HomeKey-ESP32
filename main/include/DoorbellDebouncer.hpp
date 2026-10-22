#pragma once
#include <cstdint>

// Stable release is required after boot and between presses.
class DoorbellDebouncer {
public:
    bool update(bool pressed, uint32_t now, uint32_t debounceMs) {
        if (!initialized_) {
            initialized_ = true;
            candidate_ = pressed;
            since_ = now;
        }
        if (pressed != candidate_) {
            candidate_ = pressed;
            since_ = now;
        }
        if (uint32_t(now - since_) < debounceMs) return false;
        if (!candidate_) { armed_ = true; return false; }
        if (!armed_) return false;
        armed_ = false;
        return true;
    }
private:
    bool initialized_ = false, candidate_ = false, armed_ = false;
    uint32_t since_ = 0;
};
