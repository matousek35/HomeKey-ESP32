#pragma once
#include "GPIOAllocator.hpp"
#include "cJSON.h"
#include "HttpNotificationPolicy.hpp"
#include <cmath>
#include <string>

// Validate the complete proposed settings, even when only an enable switch changed.
inline std::string validateDoorbellCameraSettings(cJSON* saved, cJSON* changes) {
    auto value = [&](const char* key) {
        auto item = cJSON_GetObjectItemCaseSensitive(changes, key);
        return item ? item : cJSON_GetObjectItemCaseSensitive(saved, key);
    };
    auto enabled = [&](const char* key) { return value(key)->valueint != 0; };
#ifndef CONFIG_HOMEKEY_DOORBELL
    if (enabled("doorbellEnabled")) return "This firmware does not include the GPIO doorbell";
#endif
#ifndef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    if (enabled("cameraEnabled")) return "Install the camera firmware variant to enable the camera";
#endif
    for (const char* key : {"httpDoorbellUrl", "httpLockedUrl", "httpUnlockedUrl", "httpTopicUrl", "httpStateUrl", "httpCustomStateUrl", "httpAuthUrl", "httpAltActionUrl", "httpStatusUrl", "httpBellTopicUrl"}) {
        std::string url = value(key)->valuestring;
        if (url.empty()) continue;
        if (!HttpNotificationPolicy::validUrl(url))
            return std::string(key) + ": use an HTTP(S) URL (max 512 characters), with credentials in the separate fields";
    }
    for (const char* key : {"httpUsername", "httpPassword"})
        if (std::string(value(key)->valuestring).size() > 128) return std::string(key) + ": maximum 128 characters";
    if (enabled("httpNotifyEnabled") && !*value("httpDoorbellUrl")->valuestring &&
        !*value("httpLockedUrl")->valuestring && !*value("httpUnlockedUrl")->valuestring)
        return "Enter at least one HTTP notification URL";

    if (enabled("httpApiEnabled") && !enabled("webAuthEnabled"))
        return "Enable web authentication in Security before enabling HTTP commands";
    if (enabled("httpMirrorEnabled")) {
        bool destination = false;
        for (const char* key : {"httpTopicUrl", "httpStateUrl", "httpCustomStateUrl", "httpAuthUrl", "httpAltActionUrl", "httpStatusUrl", "httpBellTopicUrl"}) {
            std::string url = value(key)->valuestring;
            destination |= !url.empty();
            if (!url.empty() && !enabled("httpMirrorPost") && url.find("{payload}") == std::string::npos)
                return std::string(key) + ": GET mapping requires {payload}; use POST for the original body";
        }
        if (!destination) return "Enter at least one topic mapping URL";
    }

    struct Pin { const char* key; bool output; bool camera; };
    const Pin pins[] = {
        {"doorbellGpio", false, false}, {"cameraPowerGpio", true, true},
        {"cameraXclkGpio", true, true}, {"cameraSdaGpio", true, true}, {"cameraSclGpio", true, true},
        {"cameraD0Gpio", false, true}, {"cameraD1Gpio", false, true}, {"cameraD2Gpio", false, true},
        {"cameraD3Gpio", false, true}, {"cameraD4Gpio", false, true}, {"cameraD5Gpio", false, true},
        {"cameraD6Gpio", false, true}, {"cameraD7Gpio", false, true},
        {"cameraVsyncGpio", false, true}, {"cameraHrefGpio", false, true}, {"cameraPclkGpio", false, true}
    };
    std::array<bool, 256> used{};
    for (const auto& pin : pins) {
        auto item = value(pin.key);
        if (item->valuedouble != std::floor(item->valuedouble) || item->valuedouble < 0 || item->valuedouble > 48)
            return std::string(pin.key) + ": GPIO must be an integer from 0 to 48";
        if (!enabled(pin.camera ? "cameraEnabled" : "doorbellEnabled")) continue;
        const auto n = static_cast<uint8_t>(item->valueint);
        if (used[n]) return "Doorbell/camera GPIO assignments must be unique";
        used[n] = true;
        auto verdict = GPIOAllocator::instance().validate(gpio_num_t(n),
            pin.output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
            pin.output ? GPIOAllocator::PinRole::GpioOut : GPIOAllocator::PinRole::GpioIn,
            GPIOAllocator::PinConsumer::DoorbellCamera, GPIOAllocator::PinConsumer::DoorbellCamera);
        if (!verdict) return std::string(pin.key) + " " + GPIOAllocator::error_str(verdict.error());
    }
    return {};
}
