
# Ring buffer
Bộ đệm vòng
# Mô tả
Ring buffer là 1 kỹ thuật được sử dụng phổ biến trong khi truyền nhận data. Mục đích của nó là tạo ra 1 buffer để lưu sẵn, trong 1 số trường hợp cpu không xử lý kịp khi data được gửi tới, dữ liệu sẽ tạm thời được lưu vào bộ đệm để xử lý dần khi CPU rảnh.

Ví dụ, Khi đang erase one page flash, có data gửi tới, lúc này nếu không dùng ring buffer, sẽ dẫn tới thất thoát dữ liệu. Ngắt hoặc DMA sẽ được dùng đi kèm với ringbuffer để xử lý các trường hợp này.

Nguyên lý hoạt động của ring buffer dựa trên head và tail, dữ liệu sẽ quay vòng lại vị trí 0 khi bộ đệm đạt max kích thước. Đây là điểm khác biệt so với các buffer thông thường.

# Tài liệu tham khảo
Internet


