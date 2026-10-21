#include "DoorbellButton.hpp"
#include "app_event_loop.hpp"
#include "app_events.hpp"
#include "esp_timer.h"

bool DoorbellButton::begin() {
#ifdef CONFIG_HOMEKEY_DOORBELL
    auto result = GPIOAllocator::instance().acquire(
        gpio_num_t(CONFIG_HOMEKEY_DOORBELL_GPIO), GPIO_MODE_INPUT,
        GPIOAllocator::PinRole::GpioIn, GPIOAllocator::PinConsumer::Hardware, "DOORBELL_BUTTON");
    if (!result || !result->valid()) {
        ESP_LOGE("Doorbell", "Cannot reserve doorbell GPIO; button disabled");
        return false;
    }
    pin_ = std::move(result.value());
    pin_.set_pulldown(false);
    pin_.set_pullup(true);
    ESP_LOGI("Doorbell", "Active-low button on GPIO %d", pin_.get_pin());
    return true;
#else
    return false;
#endif
}

void DoorbellButton::poll() {
#ifdef CONFIG_HOMEKEY_DOORBELL
    if (pin_.valid() && debounce_.update(!pin_.get_level(),
            uint32_t(esp_timer_get_time() / 1000), CONFIG_HOMEKEY_DOORBELL_DEBOUNCE_MS)) {
        // Same event as the keypad: existing automation triggers work unchanged.
        AppEventLoop::publish(KEYPAD_EVENT, KEYPAD_DOORBELL, nullptr, 0);
    }
#endif
}
