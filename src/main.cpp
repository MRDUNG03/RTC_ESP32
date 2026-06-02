#include "wifi_config.h"
#include "rtc.h"

void setup()
{
  // 1. Khởi tạo Baudrate cho Serial Monitor
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=============================================");
  Serial.println("--- HỆ THỐNG KHỞI ĐỘNG ---");
  Serial.println("=============================================");

  // 2. Khởi tạo phần cứng I2C và kiểm tra  IC DS3231
  if (!initRTC())
  {
    Serial.println("Lỗi khởi tạo RTC, không tìm thấy DS3231. Vui lòng kiểm tra kết nối phần cứng.");
    while (1)
    {
      delay(1000);
    }
  }

  setupWiFi();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Chế độ: ONLINE (Ưu tiên NTP Internet)");

    // Lấy thời gian từ NTP Server và ghi thẳng xuống thanh ghi của DS3231 để cập nhật
    setRtcTimeFromNtp();

    // Đồng bộ thời gian chính xác vừa lưu trong RTC vào bộ Timer (nhân nội) của ESP32
    syncRtcToEsp32Internal();
  }
  else
  {
    Serial.println("Chế độ: OFFLINE (Sử dụng thời gian lưu trữ từ chip RTC)");

    // Không có Internet -> Bỏ qua bước NTP, lấy thẳng thời gian của DS3231 nạp vào ESP32
    syncRtcToEsp32Internal();
  }

  Serial.println("=============================================");
  Serial.println("Chế độ: Bắt đầu tiến trình loop()...");
}

void loop()
{
  printInternalTime();

  delay(1000);
}