#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <time.h>
#include <sys/time.h>

// Địa chỉ I2C của DS3231
#define DS3231_I2C_ADDRESS 0x68

// Cấu trúc để lưu trữ thời gian đọc từ RTC
typedef struct {
  uint16_t year; // Năm
  uint8_t  month; // Tháng
  uint8_t  day; // Ngày
  uint8_t  hour; // Giờ
  uint8_t  minute; // Phút
  uint8_t  second; // Giây
} CustomDateTime; // cấu trúc dữ liệu thay vì dùng struct như trên chỉ cần gắn biến customDateTime là kiểu struct đã định nghĩa

// Khai báo các hàm chức năng trong thư viện tự viết
bool initRTC();
bool writeTimeToRTC(CustomDateTime dt); // truyền tham chiếu cấu trúc thời gian
bool readTimeFromRTC(CustomDateTime &dt); // truyền tham chiếu cấu trúc thời gian để nhận dữ liệu

void syncRtcToEsp32Internal(); // Đồng bộ thời gian từ RTC vào bộ Timer nội của ESP32
void setRtcTimeFromNtp(); // Lấy thời gian từ NTP Server và ghi xuống RTC
void printInternalTime(); // in thời gian hiện tại của ESP32

#endif