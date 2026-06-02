#include "ota.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <httpUpdate.h>
#include <ArduinoJson.h>

// Đường dẫn API GitHub lấy bản Release mới nhất của bạn
static const char* github_api_url = "https://api.github.com/repos/MRDUNG03/RTC_ESP32/releases/latest";

void check_and_update_ota(const char* ssid, const char* password, const String current_version) {
    // 1. Kết nối Wi-Fi nếu chưa kết nối
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Đang kết nối Wi-Fi để check OTA...");
        WiFi.begin(ssid, password);
        int counter = 0;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            counter++;
            if (counter > 20) { // Quá 10 giây không kết nối được thì thoát để chạy code chính
                Serial.println("\n[OTA] Kết nối Wi-Fi thất bại, bỏ qua check OTA.");
                return;
            }
        }
        Serial.println("\n[OTA] Wi-Fi đã kết nối thành công!");
    }

    // 2. Gọi API GitHub
    HTTPClient http;
    http.begin(github_api_url);
    http.setUserAgent("ESP32-OTA"); // Bắt buộc phải có User-Agent với GitHub API

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Khởi tạo bộ giải mã JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            String latest_version = doc["tag_name"].as<String>();
            Serial.print("[OTA] Phiên bản hiện tại: "); Serial.println(current_version);
            Serial.print("[OTA] Phiên bản mới nhất trên GitHub: "); Serial.println(latest_version);
            
            // Nếu phát hiện có phiên bản mới hơn trên GitHub
            if (latest_version != current_version) {
                Serial.println("[OTA] Phát hiện phiên bản mới! Đang lấy link tải...");
                
                // Lấy link tải file từ asset đầu tiên (file .bin)
                String download_url = doc["assets"][0]["browser_download_url"].as<String>();
                Serial.print("[OTA] Link tải: "); Serial.println(download_url);
                
                Serial.println("[OTA] Bắt đầu tải và nạp phần mềm tự động... Vui lòng giữ nguồn!");
                
                // Tự động khởi động lại sau khi nạp xong thành công
                httpUpdate.rebootOnUpdate(true);
                t_httpUpdate_return ret = httpUpdate.update(http, download_url);
                
                switch (ret) {
                    case HTTP_UPDATE_FAILED:
                        Serial.printf("[OTA] Cập nhật thất bại (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                        break;
                    case HTTP_UPDATE_NO_UPDATES:
                        Serial.println("[OTA] Không có bản cập nhật mới nào.");
                        break;
                    case HTTP_UPDATE_OK:
                        Serial.println("[OTA] Thành công! Đang khởi động lại chip...");
                        break;
                }
            } else {
                Serial.println("[OTA] Thiết bị đang ở phiên bản mới nhất.");
            }
        } else {
            Serial.println("[OTA] Lỗi giải mã dữ liệu JSON từ GitHub.");
        }
    } else {
        Serial.printf("[OTA] Không thể kết nối tới API GitHub, Mã lỗi: %d\n", httpCode);
    }
    http.end();
}