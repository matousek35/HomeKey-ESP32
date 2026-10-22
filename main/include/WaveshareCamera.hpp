#pragma once
#include "esp_http_server.h"
#include "config.hpp"
namespace WaveshareCamera {
void begin(const espConfig::misc_config_t& config);
esp_err_t snapshot(httpd_req_t* req);
// Dedicated HTTP task: a slow MJPEG client must not occupy the UI/OTA server.
void startStream(esp_err_t (*handler)(httpd_req_t*), void* context);
void stopStream();
esp_err_t stream(httpd_req_t* req);
}
