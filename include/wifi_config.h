#pragma once
#include <WiFi.h>

// THÔNG TIN WIFI SỬ DỤNG
const char* ssid     = "Villa_3lau"; // Tên Wi-Fi của bạn
const char* password = "23456778";   // Mật khẩu Wi-Fi của bạn

/// HÀM KẾT NỐI WIFI
void setupWiFi() {
  Serial.print("Đang kết nối Wi-Fi");
  WiFi.begin(ssid, password);
  
  // Chờ kết nối tối đa 20 giây, nếu không có Wi-Fi sẽ bỏ qua để chạy tiếp bằng RTC
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 5) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nĐã kết nối Wi-Fi thành công!");
  } else {
    Serial.println("\nKhông thể kết nối Wi-Fi, chuyển qua sử dụng RTC để chạy tiếp.");
  }
}
