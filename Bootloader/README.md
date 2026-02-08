
# OTA bootloader
Techniques: DMA, Flash layout, Ringbuffer, Watchdog (IWDG), Lowlevel programming, dual firmware, auto rollback,...
# Mô tả
*Thông tin thư mục:*

- ESP32: Thư mục này chứa code arduino IDE cho ESP32
- Source: Thư mục này chứa source code bootloader của stm32
- Header: Thư mục này chứa header code của STM32

*Ý tưởng*: Xây dựng 1 bootloader tự động boot và nạp code cho STM32F1.

*Chức năng*: Người dùng push firmware lên webserver chạy local của ESP32 (Access point mode), esp32 sẽ download firmware về và boot/update firmware đó cho stm32

*Chi tiết*: 

Người dùng build file firmware với địa chỉ flash được chỉ định (một trong hai vùng 0x08002000 hoặc 0x08007000), sau khi build được file.bin thì thực hiện tạo một file version theo đúng tiêu chuẩn sau:

APP_ID: 2
VERSION: 2.0.31
ENTRY_ADDR: 0x08007000

Trong đó APP_ID là ID của firmware, ID 1 tương ứng với địa chỉ 0x08002000 và ID 2 tương ứng với địa chỉ 0x08007000.
VERSION là tên phiên bản của firmware vừa build, nó có dạng xxx.xxx.xxx. ENTRY_ADDR là địa chỉ CPU sẽ nhảy tới thực thi sau khi nạp xong firmware vào flash, nó có thể là một trong hai địa chỉ vừa nêu ở trên.

Sau khi đã có 2 file: một file main.bin và một file version.txt theo đúng format.
Người dùng thực hiện cấp nguồn cho board bằng cách kết nối với adapter 12V trên cổng header. Sau đó kết nối vào wifi nội bộ của ESP có tên là ESP32. Sau đó truy cập vào địa chỉ 192.168.4.1 để up các file vừa tạo lên đó, các file này sẽ được ESP32 pull về và nạp cho STM32 trên board (Hoàn toàn tự động, nạp từ xa, không cần cắm dây nạp gì cả, tất cả chỉ cần cấp nguồn 12V cho board).
Tốc độ nạp siêu nhanh.

## Kết quả demo

Đã nạp thành công.

## Vấn đề gặp phải: 

### **Fix bug lần I (25/5/2025)**

Thứ nhất, trong quá trình viết, cần đặc biệt chú ý đến địa chỉ flash origin của application. Chúng ta cần write nó ở 1 địa chỉ hoàn toàn mới, không phải mặc định 0x08000000 trong manual của ST. Việc viết chương trình ở 1 địa chỉ khác, cần phải báo cho linker biết. 

Thứ hai, khi test ngắt ở application, cần chú ý đến flash origin. Bởi vì nếu chúng ta set lại flash origin 0x08002000 (địa chỉ mới cho application), thì interrupt sẽ không hoạt động :). Lý do rất đơn giản, khi bạn build với flash origin như vậy, chương trình sẽ setup ở địa chỉ mới, nhưng bạn chưa nói STM biết, nó vẫn nghĩ offset của vector table ở 0x08000000, và :)) nó sẽ nhảy function pointer đến 1 nơi không có địa chỉ vì vectortable lúc này nằm ở địa chỉ 0x08002000.

Thứ ba, ngoài nút nhấn reset vật lý, ARM cortex m3 có thanh ghi hỗ trợ SOFT RESET (có cơ chế nhập key để tránh reset nhầm). Ngoài ra còn có 1 số thông tin về thanh ghi offset của vector table,... được đề cập trong tài liệu ARMv7 Arch ref manual.

Thứ tư, về việc pull firmware ở trên github về. Không biết là do mạng chậm hay có limit về response time. Mình test thì push lên khoảng 3-5p sau esp32 mới nhận được sự thay đổi file version và down application về. Hiện tại chưa fix được lỗi này, nhanh hơn thì có thể sử dụng nền tảng khác ngoài github.

### **Fix bug lần II (29/6/2025)**

Thứ nhất, bug về việc respond time limit trên github đã được fix, bằng cách sử dụng Github API. Vì github thông thường sẽ cache cho các file mới được gửi lên trong khoảng 3-5 phút. Do đó để có thể respond trong khoảng 10s thì cần phải tạo token ID trên github, gia tăng tốc độ phản hồi lên 5000 requests/hour.

Thứ hai, trong quá trình thử nghiệm, thay vì việc sử dụng ngắt ngoài để tạo boot signal, mình đã sử dụng UART. Bằng cách gửi các chuỗi "START" và "STOPP" đi kèm với tập tin, sẽ cho STM biết được khi nào cần nhảy vào bootloader nhận firmware. Thực tế, khi cần nạp firmware mới, mình đã sử dụng thêm 1 kí tự 'A' để tạo Rx Interrupt bên application, sau đó reset về bootloader nhận firmware.

Thứ ba, một bug liên quan đến chuỗi kí tự. Với "START", mình tạo buff và chèn data vào từ trái qua phải, cái này ok. Nhưng với "STOPP", nó không hoạt động. Khi chạy debug, mình phát hiện ra có rất nhiều trường hợp nó nhận về "PPSTO", điều này làm cho việc so sánh chuỗi bị sai. Do đó mình này ra ý tưởng cho nó chèn data từ phải qua trái, lúc này các ký tự cuối sẽ luôn là "STOPP".

### **Fix bug + Update lần III (11/7/2025)**

Sau lần update trước, có 1 vấn đề là, để nạp được code mà không dùng chân boot và reset, thì sẽ phải sửa firmware và dùng ngắt UART. Điều này gây bất tiện và phải sửa firmware. 

Lần này mình đã fix lại, không dùng ngắt UART nữa. Thay vào đó là boot pin và RST pin. Esp32 sẽ điều khiển 2 pin này bằng gpio. Khi cần update, esp32 kéo rst pin của stm và nó sẽ nhảy về bootloader, lúc này stm check boot pin, nếu đang kéo gnd thì nó nhảy vào receive firmware. 

Như vậy, Firmware sẽ không phải sửa gì cả, hoàn toàn tự động.

Có 1 bug phát sinh đó là soft reset của stm sẽ kéo pin nrst về 0. Trong khi nrst pin kết nối với GPIO của esp và nó đang kéo high. Điều này gây mâu thuẫn khiến cho stm không reset được. Bug này đã được fix bằng cách sau khi stm nhảy vào receive firmware, esp cho gpio về dạng input (floating).

### **Fix bug + Big update lần IV (8/2/2026)**
Hoàn thiện project, thiết kế board PCB hoàn chỉnh, test board và nạp thành công. 
Nạp một file blink.bin 2kb với tốc độ 0.099s (đã đo bằng timer1)

# Tài liệu tham khảo
Reference manual (RM0008)

Flash memory manual (PM0075)

Datasheet stm32f103xx 

Cortex m3 technical reference manual

Arm®
v7-M Architecture
Reference Manual
