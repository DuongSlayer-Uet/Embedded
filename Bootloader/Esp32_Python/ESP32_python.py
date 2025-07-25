"""
=========================================
ESP32 Firmware OTA Updater via Github API
=========================================

Author: Duong (DuongSlayer-UET)
Board: ESP32
Objective:
    - Kết nối wifi
    - Định kỳ kiểm tra file version.txt trên github (mỗi 10s)
    - Sử dụng github API để chống cache (limit time response)
    - Hỗ trợ reconnect khi ngắt wifi
Feature:
    - Token github, lên tới 5000 requests/hour
    - Dễ bảo trì,...
Github repo: https://github.com/DuongSlayer-Uet/Embedded

Update time: 19/7/2025
"""
import os
import network
import urequests
import time
import urequests
import ubinascii
import ujson
from machine import UART, Pin

tx_pin = 17
rx_pin = 16
baudrate = 9600

# Bật pull-up cho RX2 (GPIO16)
rx_pin_obj = Pin(rx_pin, Pin.IN, Pin.PULL_UP)

# Buffer RX interrupt
rx_buffer = b''

# Bật uart2
uart = UART(2, baudrate=baudrate, tx=tx_pin, rx=rx_pin)

# Khởi tạo PD5 (bootpin) và PD18 (Reset pin)
pin_PD5 = Pin(5, Pin.OUT)
pin_PD18 = Pin(18, Pin.OUT)
pin_PD18.value(1)
pin_PD5.value(1)

# Github Token
GITHUB_TOKEN = ""

# Brief: Kết nối wifi
# Param: ssid - tên wifi
# Param: password - Mật khẩu
def connect_wifi(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print("[ESP32] Đang kết nối WiFi...")
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            time.sleep(0.5)
    print("[ESP32] Kết nối WiFi thành công:", wlan.ifconfig())

# Brief: Kết nối lại wifi
# Param: ssid
# Param: password
def ensure_wifi_connection(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    while not wlan.isconnected():
        print("Mất kết nối WiFi. Đang thử kết nối lại...")
        if not connect_wifi(ssid, password):
            print("Kết nối không thành công. Sẽ thử lại sau 5 giây...")
            time.sleep(5)  # Chờ 5 giây trước khi thử lại
        else:
            print("[ESP32] Đã kết nối WiFi thành công!")
    return True

# Brief: Gửi firmware qua UART1
# Param: File path - đường dẫn lưu file
def send_firmware_uart(file_path):
    print("[ESP32] Đang gửi firmware qua UART...")

    try:
        with open(file_path, 'rb') as f:
            total = 0
            while True:
                byte = f.read(1)
                if not byte:
                    break
                uart.write(byte)
                total += 1
                if total % 256 == 0:
                    print(f"[ESP32] Đã gửi: {total} bytes")
                # time.sleep(0.05)  # Delay 5ms
        print("[ESP32] Gửi hoàn tất.")
    except Exception as e:
        print("Lỗi khi gửi UART:", e)

# Brief: Lấy file version hiện tại
# Param: api_url - đường dẫn đến file trên github
# Param: ssid
# Param: password
def get_online_version_github_api(api_url, ssid, password):
    try:
        ensure_wifi_connection(ssid, password)
        headers = {
            "User-Agent": "ESP32-Client",
            "Authorization": "token " + GITHUB_TOKEN
        }
        r = urequests.get(api_url, headers=headers)
        if r.status_code == 200:
            json_data = r.json()
            content_encoded = json_data['content']
            content_decoded = ubinascii.a2b_base64(
                content_encoded).decode('utf-8').strip()
            print("[ESP32] Version hiện tại (API):", content_decoded)
            return content_decoded
        else:
            print("Lỗi HTTP:", r.status_code)
            return None
    except Exception as e:
        print("Lỗi khi lấy version từ API:", e)
        return None
    
# Brief: Hàm download firmware từ github bằng token
# Param: api_url (link firmware), ssid, password, save_path
def download_firmware_github_api(api_url, ssid, password, save_path="/blinkgithub.bin"):
    try:
        ensure_wifi_connection(ssid, password)
        headers = {
            "User-Agent": "ESP32-Client",
            "Authorization": "token " + GITHUB_TOKEN
        }
        print("[ESP32] Đang tải firmware (API)...")
        r = urequests.get(api_url, headers=headers)
        if r.status_code == 200:
            json_data = r.json()
            content_encoded = json_data['content']
            content_binary = ubinascii.a2b_base64(content_encoded)
            with open(save_path, "wb") as f:
                f.write(content_binary)
            print("[ESP32] Đã lưu firmware từ API vào:", save_path)
            return True
        else:
            print("Lỗi HTTP:", r.status_code)
            return False
    except Exception as e:
        print("Lỗi khi tải firmware API:", e)
        return False
    
# Brief: Hàm xử lý ngắt cho rx
# Param: Ngắt UARTn (vd: Uart1,...)
# Retval: void
def uart_irq_rx_handler(uart_inst):
    global rx_buffer
    while uart_inst.any():
        byte = uart_inst.read(1)
        if byte == b'\n':
            try:
                print(rx_buffer.decode().strip())
            except:
                print("Decode lỗi")
            rx_buffer = b''
        else:
            rx_buffer += byte

# Brief: Hàm tính crc32bit
# Param: crc - Giá trị hiện tại
# Param: data - Giá trị muốn tính thêm
# Retval: Giá trị crc32 trả về
def crc32(crc, data):
    crc ^= data
    for _ in range(32):
        if crc & 0x80000000:
            crc = (crc << 1) ^ 0x04C11DB7
        else:
            crc <<= 1
        crc &= 0xFFFFFFFF  # Giữ crc luôn nằm trong 32-bit
    return crc

# Brief: Hàm này tính CRC cho file blinkgithub.bin trong thư mục chính
# Retval: CRC32 trả về của file bin
def crc_cal(filepath="/blinkgithub.bin"):
    try:
        with open(filepath, "rb") as f:
            crc = 0xFFFFFFFF
            while True:
                chunk = f.read(4)
                if not chunk:
                    break
                # Padding nếu < 4 byte
                if len(chunk) < 4:
                    chunk += b'\xFF' * (4 - len(chunk))
                
                # Little endian: LSB first
                data_word = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0]
                crc = crc32(crc, data_word)

        print("[ESP32] CRC32 của", filepath, "là: 0x{:08X}".format(crc))
        return crc
    except Exception as e:
        print("Lỗi:", e)
        return None
    

# Đăng kí ngắt RX
uart.irq(handler=uart_irq_rx_handler, trigger=UART.IRQ_RX)

# Hàm main
def check_and_update_loop():
    global rx_buffer
    base_version_url = "https://api.github.com/repos/DuongSlayer-Uet/Embedded/contents/Firmware/version.txt"
    firmware_api_url = "https://api.github.com/repos/DuongSlayer-Uet/Embedded/contents/Firmware/blinkled.bin"
    firmware_path = "blinkgithub.bin"
    last_version = None
    ssid = "Iphone"
    password = "duongbuihihi"
    connect_wifi(ssid, password)
    while True:
        print("[ESP32] Kiểm tra version mới...")
        current_version = get_online_version_github_api(
            base_version_url, ssid, password)
        # check nếu có sự khác biệt
        if current_version and current_version != last_version:
            print(f"[ESP32] Phát hiện version mới: {current_version}")
            # tránh cache cho cả firmware
            if download_firmware_github_api(firmware_api_url, ssid, password, firmware_path):
                # Kéo cả boot pin và rst pin về 0
                print("[ESP32] Kéo NRST và Bootpin")
                pin_PD5.value(0)
                pin_PD18.init(Pin.OUT)
                pin_PD18.value(0)
                #print("[ESP32] Reset for 1 seconds")
                time.sleep(0.1)
                # nhấc rst pin lên trước, boot pin lên sau
                pin_PD18.value(1)
                time.sleep(0.1)
                pin_PD5.value(1)
                pin_PD18.init(Pin.IN)
                # Bắt đầu gửi firmware
                uart.write(b"START")
                print("[ESP32] Đã gửi START")
                time.sleep(0.5)
                
                send_firmware_uart(firmware_path)
                
                uart.write(b"CRC32")
                
                crc = crc_cal()
                
                if crc is not None:
                    crc_bytes = crc.to_bytes(4, 'little')
                    uart.write(crc_bytes)
                    print("[ESP32] CRC Sent!")
                
                # Stop gửi
                #uart.write(b"STOPP")
                #print("[ESP32] Đã gửi STOP")
                last_version = current_version
        else:
            print("[ESP32] Không có cập nhật mới.")

        time.sleep(10)


# ==== Thực thi chương trình chính ====
check_and_update_loop()
