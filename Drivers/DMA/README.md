
# DMA (Direct memory access)
Source code này cấu hình DMA1 channel5, DMA này dành cho nhận RX UART1
# Mô tả
Theo như tài liệu của ST (ref manual), mỗi DMA mỗi channel dành cho 1 vài peripherals nhất định, trong trường hợp này chúng ta đang cần DMA cho bootloader UART, nên sẽ dùng DMA1 channel5 để nhận RX UART1.

Ưu điểm của việc dùng DMA là dữ liệu ghi nhanh hơn rất rất nhiều (x10) so với ngắt RX. Do dữ liệu được ghi trực tiếp vào buffer mà không cần qua ngắt.

# Tài liệu tham khảo
RM0008 Reference manual


