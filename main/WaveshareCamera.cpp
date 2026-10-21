#include "WaveshareCamera.hpp"
#include "sdkconfig.h"
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
#include "GPIOAllocator.hpp"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <atomic>
#include <cstdio>
#include <mutex>
#include <vector>

namespace {
std::vector<GPIOAllocator::GPIOLease> leases;
std::atomic<bool> ready{false}, streaming{false};
std::mutex captureMutex;
httpd_handle_t server = nullptr;

esp_err_t unavailable(httpd_req_t* req) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req, "Camera unavailable; check boot log and GPIO assignments");
}
}
#endif

void WaveshareCamera::begin(const espConfig::misc_config_t& config) {
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    if (ready || !config.cameraEnabled) return;
    if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) == 0) {
        ESP_LOGE("Camera", "PSRAM is unavailable; camera disabled");
        return;
    }
    // Board wiring: https://docs.waveshare.com/ESP32-S3-ETH/Development-Environment-Setup-Arduino
    const int inputs[] = {config.cameraVsyncGpio, config.cameraHrefGpio, config.cameraPclkGpio,
        config.cameraD0Gpio, config.cameraD1Gpio, config.cameraD2Gpio, config.cameraD3Gpio, config.cameraD4Gpio, config.cameraD5Gpio, config.cameraD6Gpio, config.cameraD7Gpio};
    const int outputs[] = {config.cameraPowerGpio, config.cameraXclkGpio, config.cameraSdaGpio, config.cameraSclGpio};
    auto claim = [](int pin, gpio_mode_t mode) {
        auto result = GPIOAllocator::instance().acquire(gpio_num_t(pin), mode,
            mode == GPIO_MODE_INPUT ? GPIOAllocator::PinRole::GpioIn : GPIOAllocator::PinRole::GpioOut,
            GPIOAllocator::PinConsumer::DoorbellCamera, "WAVESHARE_CAMERA");
        if (!result || !result->valid()) return false;
        leases.push_back(std::move(result.value()));
        return true;
    };
    for (int pin : inputs) {
        if (!claim(pin, GPIO_MODE_INPUT)) { leases.clear(); return; }
    }
    for (int pin : outputs) {
        if (!claim(pin, GPIO_MODE_OUTPUT)) { leases.clear(); return; }
    }
    gpio_set_level(gpio_num_t(config.cameraPowerGpio), 0); // Active-low regulator enable (Waveshare cam_web.c).
    vTaskDelay(pdMS_TO_TICKS(20));
    camera_config_t c{};
    c.pin_pwdn = -1; c.pin_reset = -1;
    c.pin_xclk = config.cameraXclkGpio; c.pin_sccb_sda = config.cameraSdaGpio; c.pin_sccb_scl = config.cameraSclGpio;
    c.pin_d0 = config.cameraD0Gpio; c.pin_d1 = config.cameraD1Gpio; c.pin_d2 = config.cameraD2Gpio; c.pin_d3 = config.cameraD3Gpio;
    c.pin_d4 = config.cameraD4Gpio; c.pin_d5 = config.cameraD5Gpio; c.pin_d6 = config.cameraD6Gpio; c.pin_d7 = config.cameraD7Gpio;
    c.pin_vsync = config.cameraVsyncGpio; c.pin_href = config.cameraHrefGpio; c.pin_pclk = config.cameraPclkGpio;
    c.xclk_freq_hz = 20000000;
    c.ledc_timer = LEDC_TIMER_3; c.ledc_channel = LEDC_CHANNEL_7;
    c.pixel_format = PIXFORMAT_JPEG; c.frame_size = FRAMESIZE_VGA;
    c.jpeg_quality = 15; c.fb_count = 1;
    c.fb_location = CAMERA_FB_IN_PSRAM; c.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    esp_err_t err = esp_camera_init(&c);
    if (err != ESP_OK) {
        ESP_LOGE("Camera", "Initialization failed: %s", esp_err_to_name(err));
        gpio_set_level(gpio_num_t(config.cameraPowerGpio), 1);
        leases.clear();
        return;
    }
    ready = true;
    ESP_LOGI("Camera", "VGA JPEG ready; snapshot /camera.jpg, MJPEG port 81 /stream");
#endif
}

esp_err_t WaveshareCamera::snapshot(httpd_req_t* req) {
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    if (!ready) return unavailable(req);
    std::unique_lock lock(captureMutex, std::try_to_lock);
    if (!lock.owns_lock()) return unavailable(req);
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return unavailable(req);
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t result = httpd_resp_send(req, reinterpret_cast<const char*>(fb->buf), fb->len);
    esp_camera_fb_return(fb);
    return result;
#else
    return httpd_resp_send_404(req);
#endif
}

esp_err_t WaveshareCamera::stream(httpd_req_t* req) {
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    if (!ready) return unavailable(req);
    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=homekeyframe");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t result = ESP_OK;
    while (streaming) {
        {
            std::unique_lock lock(captureMutex);
            camera_fb_t* fb = esp_camera_fb_get();
            if (!fb) return ESP_FAIL;
            char header[128];
            int size = snprintf(header, sizeof(header),
                "--homekeyframe\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", unsigned(fb->len));
            result = httpd_resp_send_chunk(req, header, size);
            if (result == ESP_OK) result = httpd_resp_send_chunk(req, reinterpret_cast<const char*>(fb->buf), fb->len);
            if (result == ESP_OK) result = httpd_resp_send_chunk(req, "\r\n", 2);
            esp_camera_fb_return(fb);
        }
        if (result != ESP_OK) break;
        // Limit load on Ethernet, PSRAM and HomeKey authentication.
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    return result == ESP_OK ? httpd_resp_send_chunk(req, nullptr, 0) : result;
#else
    return httpd_resp_send_404(req);
#endif
}

void WaveshareCamera::startStream(esp_err_t (*handler)(httpd_req_t*), void* context) {
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    if (server) return;
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    c.server_port = 81; c.ctrl_port = 32769;
    c.max_open_sockets = 1; c.max_uri_handlers = 1;
    c.stack_size = 4096; c.send_wait_timeout = 3; c.recv_wait_timeout = 3;
    if (httpd_start(&server, &c) != ESP_OK) { server = nullptr; return; }
    streaming = true;
    httpd_uri_t uri{};
    uri.uri = "/stream"; uri.method = HTTP_GET; uri.handler = handler; uri.user_ctx = context;
    if (httpd_register_uri_handler(server, &uri) != ESP_OK) stopStream();
#endif
}

void WaveshareCamera::stopStream() {
#ifdef CONFIG_HOMEKEY_WAVESHARE_CAMERA
    streaming = false;
    if (server) { httpd_stop(server); server = nullptr; }
#endif
}
