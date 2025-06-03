
# Uart config
Thư viện này cấu hình UART1 cho vi xử lý stm32f103
# Mô tả
Để sử dụng được UART, cần kết hợp cấu hình GPIO (input cho RX và output cho TX).

Uart trong chương trình được tính sẵn BAUDRate theo công thức trong manual (Baudrate 9600). Nếu có sự thay đổi về clock hoặc sử dụng baudrate khác, hãy tính toán lại giá trị thanh ghi BRR.

# Tài liệu tham khảo
RM0008 Reference manual


