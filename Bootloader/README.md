
# Mini OTA bootloader
Techniques: UART, DMA, Flash layout, Ringbuffer, Github API, Watchdog (IWDG), Lowlevel programming.
# Mô tả
*Thông tin thư mục:*

- Application_Firmware: Đây là folder code app (blink led)
- Factory_Firmware: Đây là folder code factory firmware (Cũng là blink led)
- OTA_Bootloader: Đây là Main folder, chứa các file chi tiết về bootloader.
- Esp32_Python: Đây là mã python cho esp32 để download firmware từ github về

*Ý tưởng*: Xây dựng 1 bootloader tự động boot và nạp code.

*Chức năng*: Người dùng push firmware lên github, esp32 sẽ download firmware về và boot/update firmware đó cho stm32

*Chi tiết*: 

Khi push firmware lên github, cần đi kèm với file version.txt. File version này chứa tên phiên bản, esp32 sẽ tải về và check theo chu kỳ 10s 1 lần, nếu có update, nó sẽ tải file firmware và nạp cho stm32. 

Để bắt đầu nạp, Esp32 gửi chuỗi "START" qua uart, sau đó gửi firmware và kết thúc bằng việc gửi chuỗi "STOPP".

STM32 sẽ set các cờ tương ứng với từng giai đoạn (chi tiết trong code), khi được set các cờ RUN_XX thì nhảy vào vùng nhớ tương ứng để chạy firmware.

## Vấn đề gặp phải: 

### **Fix bug lần I (25/5/2025 - ?)**

Thứ nhất, trong quá trình viết, cần đặc biệt chú ý đến địa chỉ flash origin của application. Chúng ta cần write nó ở 1 địa chỉ hoàn toàn mới, không phải mặc định 0x08000000 trong manual của ST. Việc viết chương trình ở 1 địa chỉ khác, cần phải báo cho linker biết. 

Thứ hai, khi test ngắt ở application, cần chú ý đến flash origin. Bởi vì nếu chúng ta set lại flash origin 0x08002000 (địa chỉ mới cho application), thì interrupt sẽ không hoạt động :). Lý do rất đơn giản, khi bạn build với flash origin như vậy, chương trình sẽ setup ở địa chỉ mới, nhưng bạn chưa nói cho cortex m3 biết điều này, nó vẫn nghĩ offset của vector table ở 0x08000000, và :)) nó sẽ nhảy function pointer đến 1 nơi không có địa chỉ vì vectortable lúc này nằm ở địa chỉ 0x08002000.

Thứ ba, ngoài nút nhấn reset vật lý, ARM cortex m3 có thanh ghi hỗ trợ SOFT RESET (có cơ chế nhập key để tránh reset nhầm). Ngoài ra còn có 1 số thông tin về thanh ghi offset của vector table,... được đề cập trong tài liệu ARMv7 Arch ref manual.

Thứ tư, về việc pull firmware ở trên github về. Không biết là do mạng chậm hay có limit về response time. Mình test thì push lên khoảng 3-5p sau esp32 mới nhận được sự thay đổi file version và down application về. Hiện tại chưa fix được lỗi này, nhanh hơn thì có thể sử dụng nền tảng khác ngoài github.

### **Fix bug lần II (29/6/2025 - ?)**

Thứ nhất, bug về việc respond time limit trên github đã được fix, bằng cách sử dụng Github API. Vì github thông thường sẽ cache cho các file mới được gửi lên trong khoảng 3-5 phút. Do đó để có thể respond trong khoảng 10s thì cần phải tạo token ID trên github, gia tăng tốc độ phản hồi lên 5000 requests/hour.

Thứ hai, trong quá trình thử nghiệm, thay vì việc sử dụng ngắt ngoài để tạo boot signal, mình đã sử dụng UART. Bằng cách gửi các chuỗi "START" và "STOPP" đi kèm với tập tin, sẽ cho STM biết được khi nào cần nhảy vào bootloader nhận firmware. Thực tế, khi cần nạp firmware mới, mình đã sử dụng thêm 1 kí tự 'A' để tạo Rx Interrupt bên application, sau đó reset về bootloader nhận firmware.

Thứ ba, một bug liên quan đến chuỗi kí tự. Với "START", mình tạo buff và chèn data vào từ trái qua phải, cái này ok. Nhưng với "STOPP", nó không hoạt động. Khi chạy debug, mình phát hiện ra có rất nhiều trường hợp nó nhận về "PPSTO", điều này làm cho việc so sánh chuỗi bị sai. Do đó mình này ra ý tưởng cho nó chèn data từ phải qua trái, lúc này các ký tự cuối sẽ luôn là "STOPP".

### **Fix bug + Update lần III (11/7/2025 - ??)**

Sau lần update trước, có 1 vấn đề là, để nạp được code mà không dùng chân boot và reset, thì sẽ phải sửa firmware và dùng ngắt UART. Điều này gây bất tiện và phải sửa firmware. 

Lần này mình đã fix lại, không dùng ngắt UART nữa. Thay vào đó là boot pin và RST pin. Esp32 sẽ điều khiển 2 pin này bằng gpio. Khi cần update, esp32 kéo rst pin của stm và nó sẽ nhảy về bootloader, lúc này stm check boot pin, nếu đang kéo gnd thì nó nhảy vào receive firmware. 

Như vậy, Firmware sẽ không phải sửa gì cả, hoàn toàn tự động.

Có 1 bug phát sinh đó là soft reset của stm sẽ kéo pin nrst về 0. Trong khi nrst pin kết nối với GPIO của esp và nó đang kéo high. Điều này gây mâu thuẫn khiến cho stm không reset được. Bug này đã được fix bằng cách sau khi stm nhảy vào receive firmware, esp cho gpio về dạng input (floating).

## Kết luận 

Đã push firmware lên github, sau đó ESP32 boot/update firmware cho stm32 thành công, hoàn toàn tự động.
# Tài liệu tham khảo
Reference manual (RM0008)

Flash memory manual (PM0075)

Datasheet stm32f103xx 

Cortex m3 technical reference manual

Arm®
v7-M Architecture
Reference Manual
