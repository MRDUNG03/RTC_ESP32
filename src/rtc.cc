#include "rtc.h"

// Cấu hình NTP Server
const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600; // GIỜ VIỆT NAM (GMT+7) = 7 giờ * 3600 giây/giờ
const int DAYLIGHT_OFFSET_SEC = 0;    // CẤU HÌNH
// HÀM CHUYỂN ĐỔI GIỮA DECIMAL VÀ BCD (DS3231 SỬ DỤNG DẠNG BCD)
// vì ds3231 lưu tữ thời gian dưới dạng bcd nên cần chuyển đổi từ dec sang binary
uint8_t decToBcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10); // Ý NGHĨA CỦA CÔNG THỨC NÀY CHIA GIÁ TRỊ CHO 10 ĐỂ LẤY PHẦN CHỤC
                                           // SAU ĐÓ DỊCH TRÁI 4 BIT ĐỂ ĐẶT NÓ VÀO VỊ TRÍ HÀNG CHỤC TRONG BCD, CUỐI CÙNG LẤY PHẦN ĐƠN VỊ CỦA GIÁ TRỊ VÀ CỘNG VỚI PHẦN CHỤC ĐÃ DỊCH TRÁI
                                           /// VÌ SAO % 10  VIỆC LẤY PHẦN ĐƠN VỊ CỦA GIÁ TRỊ, VÌ BCD LƯU TRỮ MỖI CHỮ SỐ DƯỚI DẠNG 4 BIT, NÊN PHẦN ĐƠN VỊ SẼ Ở 4 BIT THẤP VÀ PHẦN CHỤC SẼ Ở 4 BIT CAO
}
// hàm chuyển đổi từ bcd sang dec để đọc dữ liệu từ ds3231 về dạng dec dễ hiểu hơn
uint8_t bcdToDec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F); // 0X0F là mặc định để lấy 4 bit thấp trong bcd
}
// SỬ DỤNG BOOL VÌ Ở ĐÂY NÓ SẼ KIỂM TRA KẾT LỖI VỚI DS3231 , NẾU MÀ THÀNH CÔNG THÌ TRUE, KH THÌ FALSE
bool initRTC()
{
    Wire.begin();
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    if (Wire.endTransmission() == 0)
    {
        Serial.println("[Driver RTC] Tìm thấy DS3231 thành công!");
        return true;
    }
    else
    {
        Serial.println("[Driver RTC] LỖI: Không tìm thấy DS3231!");
        return false;
    }
}
// HÀM NÀY SẼ GHI THỜI GIAN LẤY TỪ NTP XUỐNG RTC
bool writeTimeToRTC(CustomDateTime dt) // truyền tham chiếu tới cấu trúc dt để nhận dữ liệu về struct gồm năm, tháng, ngày, giờ, phút, giây để ghi xuống RTC
{
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0x00);

    Wire.write(decToBcd(dt.second));
    Wire.write(decToBcd(dt.minute));
    Wire.write(decToBcd(dt.hour));
    Wire.write(decToBcd(1));
    Wire.write(decToBcd(dt.day));
    Wire.write(decToBcd(dt.month));
    Wire.write(decToBcd(dt.year % 100));

    if (Wire.endTransmission() == 0)
    {
        return true;
    }
    return false;
}
// HÀM ĐỌC THỜI GIAN TỪ RTC
bool readTimeFromRTC(CustomDateTime &dt) // truyền tham chiếu tới cấu trúc dt để nhận dữ liệu từ hàm này
{
    Wire.beginTransmission(DS3231_I2C_ADDRESS); // bắt đầu giao tiếp với DS3231
    Wire.write(0x00);                           // ghi địa chỉ bắt đầu với địa chỉ là 0x00 để đọc từ thanh ghi giây đầu tiên
    if (Wire.endTransmission() != 0)
        return false; // nếu không thể kết nối được thì trả về false và kết thúc

    Wire.requestFrom(DS3231_I2C_ADDRESS, 7); // yêu cầu dữ liệu đọc từ DS3231, gồm  7 byte  giây, phút, giờ, ngày, tháng, năm
    if (Wire.available() >= 7)
    { // kiểm tra số byte nhận được , nếu đủ thì tiến hành chuyển sang binary và lưu vào cấu trúc dt
        // dt có mục đích ở đây là truyền tham chiếu để nhận dữ liệu từ hàm này, sau khi đọc được dữ liệu từ ds3231 thì sẽ chuyển đổi từ bcd sang dec và lưu vào dt để sử dụng cho các hàm khác
        dt.second = bcdToDec(Wire.read() & 0x7F); // 7 BIT
        dt.minute = bcdToDec(Wire.read());        // 8 BIT
        dt.hour = bcdToDec(Wire.read() & 0x3F);   // 6 BIT
        Wire.read();                              //
        dt.day = bcdToDec(Wire.read());           // 8 BIT
        dt.month = bcdToDec(Wire.read() & 0x1F);  // 5 BIT
        dt.year = bcdToDec(Wire.read()) + 2000;   // 8 BIT SẼ COCONJG VỚI NĂM 2000 ĐỂ RA NĂM HIỆN TẠI

        return true;
    }
    return false;
}
// HÀM ĐỒNG BỘ RTC VÀO TIMER CỦA ESP32  . DÙNG ĐỂ LẤY THỜI GIAN NẾU KHÔNG CÓ NTP HOẶC LẤY THỜI GIAN TỪ RTC ĐỂ ĐỒNG BỘ VỚI TIMER NỘI CỦA ESP32
void syncRtcToEsp32Internal()
{
    CustomDateTime rtcTime; // GỌI CẤU TRÚC DỮ LIỆU ĐỂ NHẬN THỜI GIAN TỪ RTC GẮN VỚI BIẾN rtcTime
    if (readTimeFromRTC(rtcTime))
    {                                    // NẾU ĐỌC THÀNH CÔNG , NÓ SẼ TIẾN HÀNH ĐỒNG HỒ THỜI GIAN VÀO TIMER NỘI CỦA ESP32
        struct tm t;                     // CẤU TRÚC THỜI GIAN CHUẨN CỦA C ĐỂ DỄ DÀNG CHUYỂN SANG EPOCH TIME
        t.tm_sec = rtcTime.second;       // TẠI ĐÂY THỜI GIAN CỦA TIMER NỘI SẼ BẰNG THỜI GIAN LẤY TỪ RTC
        t.tm_min = rtcTime.minute;       // THỜI GIAN CỦA TIMER NỘI ( PHÚT) SẼ BẰNG VỚI SỐ PHÚT CỦA RTC
        t.tm_hour = rtcTime.hour;        // THỜI GIAN CỦA TIMER NỘI (GIỜ) SẼ BẰNG VỚI SỐ GIỜ CỦA RTC
        t.tm_mday = rtcTime.day;         // THỜI GIAN CỦA TIMER NỘI (NGÀY) SẼ BẰNG VỚI SỐ NGÀY CỦA RTC
        t.tm_mon = rtcTime.month - 1;    // VÌ TRONG C TIMER NỘI CỦA NÓ BẰNG 0 NÊN TA SẼ TRỪ ĐI 1 ĐỂ RA THÁNG HIỆN TẠI VÀ GHI VÀO TIMER NỘI
        t.tm_year = rtcTime.year - 1900; // VÌ TRONG C TIMER NỘI CỦA NÓ BẰNG 1900 NÊN TA SẼ TRỪ ĐI 1900 ĐỂ RA NĂM HIỆN TẠI
        t.tm_isdst = -1;                 // CHO PHÉP HỆ THỐNG XÁC ĐỊNH GIỜ DST NẾU CÓ

        time_t epochTime = mktime(&t);      // TẠO 1 BIẾN TIME_T ĐỂ CHUYỂN THỜI GIAN CỦA TIMER NỘI SANG DẠNG EPOCH TIME (SỐ GIÂY TỪ 1/1/1970)
        struct timeval tv = {epochTime, 0}; // TẠO 1 BIẾN TIMEVAL ĐỂ SỬ DỤNG HÀM settimeofday() CHO ESP32 ĐỒNG BỘ THỜI GIAN VỚI RTC
        settimeofday(&tv, NULL);            // ĐỒNG BỘ THỜI GIAN CỦA ESP32 VỚI THỜI GIAN LẤY TỪ RTC

        Serial.printf("[Driver RTC] Đã đồng bộ sang ESP32 thành công. Unix: %ld\n", epochTime);
    }
    else
    {
        Serial.println("[Driver RTC] Lỗi không đọc được dữ liệu để đồng bộ!");
    }
}
// HÀM GHI THỜI GIAN LẤY TỪ SERVER NTP XUỐNG RTC
void setRtcTimeFromNtp()
{
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER); // CẤU HÌNH NTP
    Serial.println("[NTP] Đang đồng bộ từ Internet...");

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10000))
    {
        CustomDateTime ntpData;
        ntpData.year = timeinfo.tm_year + 1900;
        ntpData.month = timeinfo.tm_mon + 1;
        ntpData.day = timeinfo.tm_mday;
        ntpData.hour = timeinfo.tm_hour;
        ntpData.minute = timeinfo.tm_min;
        ntpData.second = timeinfo.tm_sec;

        // Khi gọi hàm này, ntpData (CustomDateTime) truyền vào sẽ khớp với hàm nhận tham trị
        if (writeTimeToRTC(ntpData))
        {
            Serial.println("[Driver RTC] Đã ghi thời gian NTP mới xuống thanh ghi DS3231!");
        }
    }
    else
    {
        Serial.println("[NTP] Gặp lỗi, không lấy được giờ từ Server.");
    }
}
// HÀM IN THỜI  GIAN HIỆN TẠI
void printInternalTime()
{
    time_t now; // lấy thời gian hiện tại là thời gian unix time
    struct tm timeinfo; // cấu trúc thời gian chuẩn của C để dễ dàng chuyển đổi và in ra định dạng 
    time(&now); // LẤY THỜI GIAN HIỆN TẠI DƯỚI DẠNG UNIXTIME 
    localtime_r(&now, &timeinfo);

    Serial.printf("[TIME] ESP32: %04d/%02d/%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.printf("Unix Time: %ld\n", now);
}