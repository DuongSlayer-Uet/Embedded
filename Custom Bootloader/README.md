
# Mini bootloader
Techniques: UART, DMA, Flash layout, Ringbuffer, Lowlevel programming (register)
# Mô tả
*Ý tưởng*: Xây dựng 1 bootloader, giúp cho việc nạp code thông qua UART, không cần dùng đến STlink.

*Chức năng*: Bootloader này chịu trách nhiệm đọc trạng thái bootpin (custom bootpin PA5). 

Có 2 boot mode:
- Nếu PA5 == 0 (nhấn PA5) thì Bootloader sẽ vào trạng thái nạp new firmware application. Lúc này chương trình thực hiện nhận RX và đẩy data trực tiếp vào buffer bằng DMA. Cùng lúc đó, buffer của DMA sẽ được copy sang ringbuffer để thực hiện đọc data nhận được. Data này sau đó được write đến vùng nhớ của application (0x08002000) 
- Nếu PA5 == 1 (nhả PA5) thì Bootloader sẽ vào trạng thái chạy firmware hiện tại nó đang có. Chương trình sẽ setup MSP, con trỏ hàm resethandler, offset của vector table,... sau đó nhảy tới app bằng con trỏ hàm resethandler của app.


# Tài liệu tham khảo
Reference manual (RM0008)

