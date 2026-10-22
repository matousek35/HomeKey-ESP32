#pragma once
#include "config.hpp"
#include "app_event_loop.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <array>
#include <atomic>

// Boot-time snapshot: the HTTP worker never reads mutable web configuration.
class HttpNotifier {
public:
    void begin(const espConfig::misc_config_t& config, const espConfig::mqttConfig_t& mqtt);
    void publish(const std::string& topic, const std::string& payload);
private:
    struct Event { std::string url, payload, topic; bool post; int64_t timeUs; size_t cost; };
    void enqueue(const std::string& url, const std::string& payload = {},
                 const std::string& topic = {}, bool post = false);
    void run();
    std::atomic<size_t> queuedBytes_{0};
    QueueHandle_t queue_ = nullptr;
    std::array<std::string, 3> urls_;
    std::array<std::string, 6> topics_, destinations_;
    std::string fallback_, username_, password_;
    bool mirror_ = false, post_ = false;
    AppEventLoop::SubscriptionHandle doorbell_, lock_;
};
