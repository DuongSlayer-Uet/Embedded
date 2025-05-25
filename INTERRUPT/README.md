# Project Title
Đây là một source code interrupt cho stm32f103c8t6
# Mô tả
Source code này hỗ trợ tạo ngắt ở các line 1,2,3,4, line 5_9 và line 10_15.
Tuy nhiên, chưa xử lý được vấn đề về priority, hiện tại đang xử lý ngắt đơn lẻ. Priority sẽ được đề cập ở các project sau.
Chú ý: Để tạo ngắt cần enable interrupt ở cả ARM cortex (NVIC table) và ở Peripheral.
# Tài liệu tham khảo
RM0008 Reference manual
Cortex-M3 Reference manual


