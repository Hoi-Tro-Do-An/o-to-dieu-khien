# 🚗 Xe Robot / Ô Tô Điều Khiển Từ Xa Bằng ESP32 Qua WiFi (Web App)

Dự án xe ô tô / robot điều khiển từ xa thông qua giao diện Web trực quan, chạy trên vi điều khiển **ESP32** kết hợp mạch điều khiển động cơ **L298N**. Không cần cài app phức tạp, chỉ cần kết nối WiFi và mở trình duyệt trên điện thoại hoặc máy tính là có thể điều khiển trực tiếp.

[![Zalo Contact](https://img.shields.io/badge/Zalo-0394.241.597-blue?style=for-the-badge&logo=zalo)](https://zalo.me/0394241597)
[![Support](https://img.shields.io/badge/H%E1%BB%97%20Tr%E1%BB%A3%20%C4%90%E1%BB%93%20%C3%81n-Web%20%7C%20App%20%7C%20IoT-orange?style=for-the-badge)](https://zalo.me/0394241597)

---

## 🌟 Tính Năng Nổi Bật

- **Giao diện Web điều khiển trực quan**: Bàn phím điều hướng mượt mà, hỗ trợ nhấn giữ (touch/pointer events), tự động dừng khi nhả tay.
- **Điều chỉnh tốc độ linh hoạt (PWM)**: Thanh trượt thay đổi vận tốc tức thì (0% - 100%).
- **Cơ chế an toàn (Failsafe Timeout)**: Tự động ngắt động cơ sau 1.2s nếu mất kết nối hoặc rớt mạng để tránh xe chạy mất kiểm soát.
- **2 Chế độ hoạt động WiFi**:
  - **Station Mode (Kết nối WiFi có sẵn)**: Kết nối vào mạng gia đình/phòng lab, hỗ trợ truy cập qua địa chỉ mDNS `http://robotcar.local`.
  - **Access Point Mode (AP - Tự phát WiFi)**: Khi không có WiFi cấu hình hoặc kết nối thất bại, xe tự phát sóng WiFi `RobotCar` (Mật khẩu: `12345678`) với IP mặc định `192.168.4.1`.
- **Tối ưu phần cứng**: Điều khiển PWM trực tiếp trên 4 chân tín hiệu logic, giữ nguyên jumper ENA/ENB của L298N.

---

## 🛠 Sơ Đồ Đấu Nối Phần Cứng

### 1. Kết nối ESP32 với Driver L298N

| Chân ESP32 | Chân L298N | Chức Năng |
| :---: | :---: | :--- |
| **GPIO 25** | **IN1** | Điều khiển Động cơ Trái (Tiến/Lùi PWM) |
| **GPIO 26** | **IN2** | Điều khiển Động cơ Trái (Tiến/Lùi PWM) |
| **GPIO 32** | **IN3** | Điều khiển Động cơ Phải (Tiến/Lùi PWM) |
| **GPIO 33** | **IN4** | Điều khiển Động cơ Phải (Tiến/Lùi PWM) |
| **GND** | **GND** | Nối chung mass (Bắt buộc) |

> ⚠️ **Lưu ý**: Giữ nguyên jumper 5V-EN tại 2 chân ENA và ENB trên mạch L298N.

### 2. Nguồn Cấp
- Pin 2S / 3S (7.4V - 12V) hoặc 2 cell 18650 cấp vào cọc 12V và GND của L298N.
- Cấp nguồn cho ESP32 qua cổng Micro-USB/Type-C hoặc từ chân 5V của L298N (nếu jumper 5V-EN bật và nguồn cấp pin phù hợp).

---

## 🚀 Hướng Dẫn Cài Đặt & Nạp Code

### Chuẩn bị
1. Cài đặt **Visual Studio Code** và extension **PlatformIO IDE**.
2. Clone hoặc tải project về máy:
   ```bash
   git clone https://github.com/Hoi-Tro-Do-An/o-to-dieu-khien.git
   ```

### Cấu hình WiFi (Tùy chọn)
Mở file `src/main.cpp`, tìm đoạn cấu hình:
```cpp
const char *WIFI_SSID = "TEN_WIFI_CUA_BAN";
const char *WIFI_PASS = "MAT_KHAU_WIFI";
```
- Nếu giữ nguyên mặc định, xe sẽ tự phát WiFi `RobotCar` / `12345678`.
- Nếu nhập đúng tên và mật khẩu WiFi của bạn, xe sẽ kết nối vào mạng nội bộ.

### Nạp Code
1. Cắm ESP32 vào máy tính qua cáp nạp dữ liệu.
2. Trên PlatformIO, nhấn biểu tượng **Upload** (hoặc dùng phím tắt `Ctrl + Alt + U`).
3. Mở **Serial Monitor** với tốc độ baud `115200` để xem thông tin IP.

---

## 📱 Hướng Dẫn Sử Dụng

1. **Kết nối WiFi**:
   - Nếu ở chế độ AP: Dùng điện thoại kết nối vào WiFi `RobotCar`, mật khẩu `12345678`.
   - Nếu ở chế độ STA: Đảm bảo điện thoại cùng mạng WiFi với ESP32.
2. **Mở trình duyệt Web**:
   - Truy cập địa chỉ IP hiển thị trên Serial Monitor (VD: `http://192.168.4.1` hoặc `http://robotcar.local`).
3. **Điều khiển**:
   - Nhấn giữ các nút mũi tên để di chuyển (Tiến, Lùi, Rẽ Trái, Rẽ Phải).
   - Nút đỏ ở giữa dùng để dừng khẩn cấp.
   - Kéo thanh trượt để tăng/giảm tốc độ động cơ.

---

## 🤝 DỊCH VỤ HỖ TRỢ ĐỒ ÁN - WEB, APP, IOT CÁC LOẠI

> 💡 **Bạn đang cần làm đồ án tốt nghiệp, bài tập lớn, hoặc dự án thực tế?**  
> Đội ngũ nhận tư vấn, thiết kế, lập trình và hỗ trợ trọn gói từ A - Z:

- 🔌 **Hệ thống IoT & Nhúng**: 
  - Vi điều khiển: ESP32, ESP8266, STM32, Arduino, Raspberry Pi, nRF52...
  - Giao thức & Nền tảng: MQTT, HTTP REST, WebSocket, Blynk IoT, Firebase, AWS IoT.
  - Ứng dụng: Xe tự hành, Smart Home, trạm quan trắc môi trường, máy điểm danh vân tay/RFID, định vị GPS, điều khiển qua LoRa khoảng cách xa.
- 🌐 **Web Application**:
  - Fullstack Web: NodeJS, React, Vue, Python (Django, FastAPI), PHP (Laravel).
  - Web Dashboard giám sát và điều khiển thiết bị phần cứng thời gian thực.
- 📱 **Mobile App**:
  - Ứng dụng Android & iOS (Flutter, React Native) kết nối Bluetooth BLE, WiFi, Cloud.
- ⚙️ **Thiết kế phần cứng & Mạch in**:
  - Thiết kế sơ đồ nguyên lý & Layout PCB trên Altium Designer, Proteus.
  - Gia công mạch in, hàn linh kiện và lắp ráp mô hình hoàn chỉnh.

### 🌟 Quyền Lợi Khi Được Hỗ Trợ:
✅ Bàn giao đầy đủ mã nguồn (Full source code), sơ đồ nguyên lý, file thiết kế.  
✅ Hướng dẫn nạp code, chạy thử và giải thích chi tiết từng dòng code.  
✅ Hỗ trợ chỉnh sửa và bổ sung tính năng theo yêu cầu của Giảng viên hướng dẫn.  
✅ Hỗ trợ chuẩn bị slide thuyết trình, tài liệu báo cáo đồ án.

---

### 📞 THÔNG TIN LIÊN HỆ:
- 📱 **Zalo / Hotline**: [**0394.241.597**](https://zalo.me/0394241597) *(Nhấp vào để chat Zalo ngay)*
- 🌐 **Trang giới thiệu chi tiết**: Xem file [gioi-thieu.html](file:///c:/Users/fpt/Documents/PlatformIO/Projects/phaocuusinh/gioi-thieu.html) hoặc mở trên trình duyệt.
- 💻 **GitHub**: [Hội Trợ Đồ Án](https://github.com/Hoi-Tro-Do-An)
