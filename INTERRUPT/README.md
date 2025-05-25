# STM32F103C8T6 - Interrupt Source Code
Đây là một source code để cấu hình và xử lý ngắt

# Mô tả
Thư viện này hỗ trợ xử lý ngắt trên các line sau của EXTI:
- Line 1, 2, 3, 4
- Line 5 đến 9 (EXTI9_5)
- Line 10 đến 15 (EXTI15_10)
Lưu ý:
- Hiện tại mới xử lý ngắt đơn lẻ, chưa xử lý priority (độ ưu tiên).
- Việc thiết lập priority sẽ được bổ sung trong các project tiếp theo.

Chú ý: Để tạo ngắt cần enable interrupt ở cả ARM cortex (NVIC table) và ở Peripheral.
  
# Tài liệu tham khảo
RM0008 Reference manual
Cortex-M3 Reference manual


