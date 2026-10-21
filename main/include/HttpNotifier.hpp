#pragma once
#include "config.hpp"
#include "app_event_loop.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <array>

// Owns a boot-time configuration snapshot. Never accesses mutable web settings.
class HttpNotifier {
public:
    void begin(const espConfig::misc_config_t& config);
private:
    struct Event { uint8_t kind; int64_t timeUs; };
    void enqueue(uint8_t kind);
    void run();
    QueueHandle_t queue_ = nullptr;
    std::array<std::string, 3> urls_;
    std::string username_, password_;
    AppEventLoop::SubscriptionHandle doorbell_, lock_;
};
