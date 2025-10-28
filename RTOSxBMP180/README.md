
# STM32xModbus sử dụng RTOS

### Mô tả: Project này hướng tới việc thiết kế hệ thống giao tiếp sử dụng modbus protocol thông qua chuẩn vật lý RS485


Công nghệ:  Custom modbus, C, windowforms, rtos, uart

### Chi tiết: 

Thiết kế modbus frame theo ý muốn dựa trên chuẩn modbus tiêu chuẩn. Bao gồm đầy đủ các thành phần như addr, func code, crc,... 

Bên master được xử lý trên windowforms bằng C#, dưới slave được xử lý bằng ngắt UART sử dụng C.

RTOS được ứng dụng cho slave, tách ra làm 2 tasks:
1 task đọc sensor và 1 task send data. Hai tasks này sẽ rơi vào sleep khi master chưa request (sử dụng semaphore), nếu master request, sẽ đánh thức task đọc sensor, sau đó sẽ đánh thức task send data.



