
# Mini OTA bootloader
Techniques: UART, DMA, Flash layout, Ringbuffer, github, Lowlevel programming (register)
# Mô tả
*Ý tưởng*: Xây dựng 1 bootloader tự động boot và nạp code.

*Chức năng*: Người dùng push firmware lên github, esp32 sẽ download firmware về và boot/update firmware đó cho stm32 

*Chi tiết*: 

Khi push firmware lên github, cần đi kèm với file version.txt. File version này chứa tên phiên bản, esp32 sẽ tải về và check theo chu kỳ 1 phút 1 lần, nếu có update, nó sẽ tải file firmware và nạp cho stm32. 

Để nạp cho stm32, esp cần kéo pin PD4, PD5 xuống để ra tín hiệu bắt đầu nạp (falling edge trigger bên stm), sau khi nạp xong thì nhấc 2 pin kia lên (rising edge trigger). Tại sao phải tách ra 2 ngắt? Vì app và bootloader sử dụng vectortable ở 2 vị trí khác nhau, do đó ngắt ở mỗi bên sẽ sử dụng 1 vectortable riêng, nên không sử dụng chung ngắt được, 1 ngắt cho bootloader và 1 ngắt cho application.

*Vấn đề gặp phải*: 

Thứ nhất, trong quá trình viết, cần đặc biệt chú ý đến địa chỉ flash origin của application. Chúng ta cần write nó ở 1 địa chỉ hoàn toàn mới, không phải mặc định 0x08000000 trong manual của ST. Việc viết chương trình ở 1 địa chỉ khác, cần phải báo cho linker biết. 

Thứ hai, khi test ngắt ở application, cần chú ý đến flash origin. Bởi vì nếu chúng ta set lại flash origin 0x08002000 (địa chỉ mới cho application), thì interrupt độc lập sẽ không hoạt động :). Lý do rất đơn giản, khi bạn build với flash origin như vậy, chương trình sẽ setup ở địa chỉ mới, nhưng bạn chưa nói cho cortex m3 biết điều này, nó vẫn nghĩ offset của vector table ở 0x08000000, và :)) nó sẽ nhảy function pointer đến 1 nơi không có địa chỉ vì vectortable lúc này nằm ở địa chỉ 0x08002000.

Thứ ba, ngoài nút nhấn reset vật lý, ARM cortex m3 có thanh ghi hỗ trợ SOFT RESET (có cơ chế nhập key để tránh reset nhầm). Ngoài ra còn có 1 số thông tin về thanh ghi offset của vector table,... được đề cập trong tài liệu ARMv7 Arch ref manual.

Thứ tư, về việc pull firmware ở trên github về. Không biết là do mạng chậm hay có limit về response time. Mình test thì push lên khoảng 3-5p sau esp32 mới nhận được sự thay đổi file version và down application về. Hiện tại chưa fix được lỗi này, nhanh hơn thì có thể sử dụng nền tảng khác ngoài github.

*Kết luận* 

Đã push thử firmware lên github và ESP32 boot/update firmware cho stm32 thành công, hoàn toàn tự động, nhưng vẫn còn nhiều thiếu sót.

# Tài liệu tham khảo
Reference manual (RM0008)

Flash memory manual

ARMv7-M Arch Ref Manual

Cortex M3 manual

