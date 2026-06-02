#ifndef GITHUB_OTA_H
#define GITHUB_OTA_H

#include <Arduino.h>
const String FIRMWARE_VERSION = "v1.0.0"; // Số phiên bản hiện tại
// Hàm khởi chạy kiểm tra và tự động cập nhật OTA từ GitHub Release
// Tham số truyền vào bao gồm: Tên Wifi, Mật khẩu Wifi, và Phiên bản hiện tại của mạch
void check_and_update_ota(const char* ssid, const char* password, const String current_version);

#endif // GITHUB_OTA_H