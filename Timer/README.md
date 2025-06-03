
# Timer config
Thư viện này cấu hình Timer1 cho vi xử lý stm32f103
# Mô tả
Thư viện này cấu hình Timer 1 16 bit 
Source clock được lấy từ APB2Bus (8Mhz), có thể thay đổi clock cho APB2 bằng cách dùng thư viện RCC đã up trước đó.

Hàm delay 1s sử dụng phương pháp "đếm tràn", check bit overflow. Có vài phương pháp khác là sử dụng ngắt tràn timer, ngắt so sánh timer,... có thể phát triển thêm.

Hạn chế: Hiện tại mới chỉ config cho Timer1, hàm chưa viết được theo kiểu truyền param cho từng timer. Định nghĩa các struct và enum cho 3 timer còn lại để hoàn thiện.

# Tài liệu tham khảo
RM0008 Reference manual


