#pragma once
#include "GPIOAllocator.hpp"
#include "DoorbellDebouncer.hpp"
class DoorbellButton {
public:
    bool begin(const espConfig::misc_config_t& config);
    void poll();
private:
    GPIOAllocator::GPIOLease pin_;
    DoorbellDebouncer debounce_;
};
