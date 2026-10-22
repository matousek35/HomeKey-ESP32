#include "HttpNotifier.hpp"
#include "HttpNotificationPolicy.hpp"
#include "MessageProtocol.hpp"
#include <memory>
#include <new>
#include "eventStructs.hpp"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/task.h"

void HttpNotifier::begin(const espConfig::misc_config_t& config, const espConfig::mqttConfig_t& mqtt) {
    if ((!config.httpNotifyEnabled && !config.httpMirrorEnabled) || queue_) return;
    mirror_ = config.httpMirrorEnabled;
    post_ = config.httpMirrorPost;
    fallback_ = config.httpTopicUrl;
    topics_ = {mqtt.lockStateTopic, mqtt.lockCustomStateTopic, mqtt.hkTopic,
               mqtt.hkAltActionTopic, mqtt.lwtTopic, mqtt.doorbellTopic};
    destinations_ = {config.httpStateUrl, config.httpCustomStateUrl, config.httpAuthUrl,
                     config.httpAltActionUrl, config.httpStatusUrl, config.httpBellTopicUrl};
    urls_ = {config.httpDoorbellUrl, config.httpUnlockedUrl, config.httpLockedUrl};
    username_ = config.httpUsername;
    password_ = config.httpPassword;
    queue_ = xQueueCreate(8, sizeof(Event*));
    if (!queue_) { ESP_LOGE("HTTPNotify", "Cannot allocate event queue"); return; }
    if (xTaskCreate([](void* self) { static_cast<HttpNotifier*>(self)->run(); },
                    "http_notify", 8192, this, 1, nullptr) != pdPASS) {
        vQueueDelete(queue_); queue_ = nullptr;
        ESP_LOGE("HTTPNotify", "Cannot start notification worker"); return;
    }
    if (!config.httpNotifyEnabled) return;
    doorbell_ = AppEventLoop::subscribe(KEYPAD_EVENT, KEYPAD_DOORBELL,
        [this](const uint8_t*, size_t) { enqueue(urls_[0]); });
    lock_ = AppEventLoop::subscribe(LOCK_EVENT, LOCK_STATE_CHANGED,
        [this](const uint8_t* data, size_t size) {
            if (!data || !size) return;
            std::error_code ec;
            std::span<const uint8_t> payload(data, size);
            auto state = alpaca::deserialize<EventLockState>(payload, ec);
            // HomeKit: 0=unlocked, 1=locked. Ignore target-only, jammed and transitional states.
            int kind = HttpNotificationPolicy::lockEvent(state.currentState, state.prevState);
            if (!ec && kind >= 0) enqueue(urls_[kind]);
        });
}

void HttpNotifier::publish(const std::string& topic, const std::string& payload) {
    if (!mirror_ || !queue_) return;
    const std::string* destination = &fallback_;
    for (size_t i = 0; i < topics_.size(); ++i) {
        if (topics_[i] == topic && !destinations_[i].empty()) { destination = &destinations_[i]; break; }
    }
    if (destination->empty()) return;
    if (payload.size() > 8192 || topic.size() > 512) {
        ESP_LOGW("HTTPNotify", "Message exceeds HTTP bridge size limit"); return;
    }
    auto url = MessageProtocol::expand(*destination, topic, payload);
    if (url.size() > 8192) { ESP_LOGW("HTTPNotify", "Expanded URL exceeds size limit"); return; }
    enqueue(url, payload, topic, post_);
}

void HttpNotifier::enqueue(const std::string& url, const std::string& payload,
                           const std::string& topic, bool post) {
    if (url.empty() || !queue_) return;
    const size_t cost = sizeof(Event) + url.size() + payload.size() + topic.size();
    if (queuedBytes_.fetch_add(cost) + cost > 32768) {
        queuedBytes_.fetch_sub(cost);
        ESP_LOGW("HTTPNotify", "HTTP queue memory budget exceeded"); return;
    }
    auto event = new (std::nothrow) Event{url, payload, topic, post, esp_timer_get_time(), cost};
    if (!event) {
        queuedBytes_.fetch_sub(cost);
        ESP_LOGW("HTTPNotify", "Cannot allocate notification"); return;
    }
    if (xQueueSend(queue_, &event, 0) != pdTRUE) {
        queuedBytes_.fetch_sub(cost);
        delete event;
        ESP_LOGW("HTTPNotify", "Queue full; notification dropped");
    }
}

void HttpNotifier::run() {
    for (;;) {
        Event* queued = nullptr;
        if (xQueueReceive(queue_, &queued, portMAX_DELAY) != pdTRUE) continue;
        auto cleanup = [this](Event* event) { queuedBytes_.fetch_sub(event->cost); delete event; };
        std::unique_ptr<Event, decltype(cleanup)> event(queued, cleanup);
        if (HttpNotificationPolicy::expired(esp_timer_get_time(), event->timeUs)) {
            ESP_LOGW("HTTPNotify", "Expired notification dropped"); continue;
        }
        esp_http_client_config_t c{};
        c.url = event->url.c_str();
        c.method = event->post ? HTTP_METHOD_POST : HTTP_METHOD_GET;
        c.timeout_ms = 2000;
        c.disable_auto_redirect = true;
        c.crt_bundle_attach = esp_crt_bundle_attach;
        if (!username_.empty()) {
            c.username = username_.c_str(); c.password = password_.c_str();
            c.auth_type = HTTP_AUTH_TYPE_BASIC;
        }
        auto client = esp_http_client_init(&c);
        if (!client) { ESP_LOGE("HTTPNotify", "Cannot create HTTP client"); continue; }
        if (event->post) {
            esp_http_client_set_header(client, "Content-Type", "text/plain; charset=utf-8");
            auto topic = MessageProtocol::encode(event->topic);
            esp_http_client_set_header(client, "X-MQTT-Topic", topic.c_str());
            esp_http_client_set_post_field(client, event->payload.data(), event->payload.size());
        }
        esp_err_t result = esp_http_client_perform(client);
        int status = esp_http_client_get_status_code(client);
        // No automatic retry: a response can be lost after Loxone already handled a pulse.
        // Never log URLs, which may contain access tokens or other sensitive values.
        if (result != ESP_OK || status < 200 || status >= 300)
            ESP_LOGW("HTTPNotify", "Notification failed: %s, HTTP %d", esp_err_to_name(result), status);
        else ESP_LOGI("HTTPNotify", "Notification delivered (HTTP %d)", status);
        esp_http_client_cleanup(client);
    }
}
