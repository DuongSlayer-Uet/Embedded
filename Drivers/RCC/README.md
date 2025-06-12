
# RCC Config
Thư viện này cấu hình clock cho vi xử lý stm32f103
# Mô tả
Chi tiết hơn về địa chỉ và vị trí các bit được đề cập trong manual.
Thư viện config clock cho 4 loại clock thường gặp: 8, 48, 64,72mhz.
Do các bit nằm không theo thứ tự, nên việc cấu hình clock theo kiểu truyền param cho func gặp 1 chút khó khăn.
Thư viện viết theo kiểu bare-metal, chỉ cần file linker và file startup để chạy. Không cần dùng hal hay cmsis.
# Tài liệu tham khảo
RM0008 Reference manual


