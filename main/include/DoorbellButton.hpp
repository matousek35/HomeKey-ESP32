#pragma once
#include "GPIOAllocator.hpp"
#include "DoorbellDebouncer.hpp"
class DoorbellButton {
public:
    bool begin();
    void poll();
private:
    GPIOAllocator::GPIOLease pin_;
    DoorbellDebouncer debounce_;
};
