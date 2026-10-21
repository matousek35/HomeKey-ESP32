#include "HttpNotifier.hpp"
#include "HttpNotificationPolicy.hpp"
#include "eventStructs.hpp"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/task.h"

void HttpNotifier::begin(const espConfig::misc_config_t& config) {
    if (!config.httpNotifyEnabled || queue_) return;
    urls_ = {config.httpDoorbellUrl, config.httpUnlockedUrl, config.httpLockedUrl};
    username_ = config.httpUsername;
    password_ = config.httpPassword;
    queue_ = xQueueCreate(8, sizeof(Event));
    if (!queue_) { ESP_LOGE("HTTPNotify", "Cannot allocate event queue"); return; }
    if (xTaskCreate([](void* self) { static_cast<HttpNotifier*>(self)->run(); },
                    "http_notify", 6144, this, 1, nullptr) != pdPASS) {
        vQueueDelete(queue_); queue_ = nullptr;
        ESP_LOGE("HTTPNotify", "Cannot start notification worker"); return;
    }
    doorbell_ = AppEventLoop::subscribe(KEYPAD_EVENT, KEYPAD_DOORBELL,
        [this](const uint8_t*, size_t) { enqueue(0); });
    lock_ = AppEventLoop::subscribe(LOCK_EVENT, LOCK_STATE_CHANGED,
        [this](const uint8_t* data, size_t size) {
            if (!data || !size) return;
            std::error_code ec;
            auto state = alpaca::deserialize<EventLockState>(std::span<const uint8_t>(data, size), ec);
            // HomeKit: 0=unlocked, 1=locked. Ignore target-only, jammed and transitional states.
            int kind = HttpNotificationPolicy::lockEvent(state.currentState, state.prevState);
            if (!ec && kind >= 0) enqueue(static_cast<uint8_t>(kind));
        });
}

void HttpNotifier::enqueue(uint8_t kind) {
    if (urls_[kind].empty()) return;
    Event event{kind, esp_timer_get_time()};
    if (xQueueSend(queue_, &event, 0) != pdTRUE)
        ESP_LOGW("HTTPNotify", "Queue full; notification dropped");
}

void HttpNotifier::run() {
    Event event{};
    for (;;) {
        if (xQueueReceive(queue_, &event, portMAX_DELAY) != pdTRUE) continue;
        // Never replay an old doorbell pulse or old lock state after a network outage.
        if (HttpNotificationPolicy::expired(esp_timer_get_time(), event.timeUs)) {
            ESP_LOGW("HTTPNotify", "Expired notification dropped"); continue;
        }
        esp_http_client_config_t c{};
        c.url = urls_[event.kind].c_str();
        c.method = HTTP_METHOD_GET;
        c.timeout_ms = 2000;
        c.disable_auto_redirect = true;
        c.crt_bundle_attach = esp_crt_bundle_attach;
        if (!username_.empty()) {
            c.username = username_.c_str(); c.password = password_.c_str();
            c.auth_type = HTTP_AUTH_TYPE_BASIC;
        }
        auto client = esp_http_client_init(&c);
        if (!client) { ESP_LOGE("HTTPNotify", "Cannot create HTTP client"); continue; }
        esp_err_t result = esp_http_client_perform(client);
        int status = esp_http_client_get_status_code(client);
        // No automatic retry: a response can be lost after Loxone already handled a pulse.
        // Never log URLs, which may contain access tokens or other sensitive values.
        if (result != ESP_OK || status < 200 || status >= 300)
            ESP_LOGW("HTTPNotify", "Event %u failed: %s, HTTP %d", event.kind, esp_err_to_name(result), status);
        else ESP_LOGI("HTTPNotify", "Event %u delivered (HTTP %d)", event.kind, status);
        esp_http_client_cleanup(client);
    }
}
