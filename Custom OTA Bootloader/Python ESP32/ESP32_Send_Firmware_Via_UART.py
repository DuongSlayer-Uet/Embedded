import network
import urequests
import time
from machine import UART, Pin

bootpina5 = 5
bootpina4 = 4
pina5 = Pin(bootpina5, Pin.OUT)
pina4 = Pin(bootpina4, Pin.OUT)
pina5.value(1)
pina4.value(1)


# ==== 1. Kết nối WiFi ====
def connect_wifi(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print("Đang kết nối WiFi...")
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            time.sleep(0.5)
    print("Kết nối WiFi thành công:", wlan.ifconfig())

def ensure_wifi_connection(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    while not wlan.isconnected():
        print("Mất kết nối WiFi. Đang thử kết nối lại...")
        if not connect_wifi(ssid, password):
            print("Kết nối không thành công. Sẽ thử lại sau 5 giây...")
            time.sleep(5)  # Chờ 5 giây trước khi thử lại
        else:
            print("Đã kết nối WiFi thành công!")
    return True


# ==== 2. Đọc version từ GitHub ====
def get_online_version(ver_url, ssid, password):
    try:
        ensure_wifi_connection(ssid, password)
        r = urequests.get(ver_url)
        version = r.text.strip()
        print("Version hiện tại: ", version)
        r.close()
        return version
    except Exception as e:
        print("Lỗi khi lấy ver.txt:", e)
        return None

# ==== 3. Tải firmware và lưu ====
def download_and_save_firmware(url, ssid, password, save_path="/blinkgithub.bin"):
    ensure_wifi_connection(ssid, password)
    print("Đang tải và lưu firmware từ:", url)
    try:
        r = urequests.get(url)
        if r.status_code != 200:
            r.close()
            print("Không tải được file:", r.status_code)
            return False

        with open(save_path, "wb") as f:
            total = 0
            chunk_size = 256
            while True:
                chunk = r.raw.read(chunk_size)
                if not chunk:
                    break
                f.write(chunk)
                total += len(chunk)
                print("Đã ghi:", total, "bytes")
                time.sleep(0.01)

        r.close()
        print("Đã lưu firmware vào:", save_path)
        return True
    except Exception as e:
        print("Lỗi khi tải hoặc lưu:", e)
        return False

# ==== 4. Gửi file qua UART ====
def send_firmware_uart(file_path, tx_pin=17, rx_pin=16, baudrate=9600):
    print("Đang gửi firmware qua UART...")
    uart = UART(2, baudrate=baudrate, tx=tx_pin, rx=rx_pin)

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
                    print(f"📤 Đã gửi: {total} bytes")
                #time.sleep(0.01)  # Delay 5ms
        print("Gửi hoàn tất.")
    except Exception as e:
        print("Lỗi khi gửi UART:", e)

# ==== 5. Main loop kiểm tra version ====
def check_and_update_loop():
    version_url = "https://raw.githubusercontent.com/DuongSlayer-Uet/Embedded/refs/heads/main/Firmware/version.txt"
    firmware_url = "https://github.com/DuongSlayer-Uet/Embedded/raw/refs/heads/main/Firmware/blinkled.bin"
    firmware_path = "blinkgithub.bin"
    last_version = None
    ssid = "Truong"
    password = "07052004"
    connect_wifi(ssid, password)
    while True:
        print("Kiểm tra version mới...")
        current_version = get_online_version(version_url, ssid, password)
        if current_version and current_version != last_version:
            print(f"Phát hiện version mới: {current_version}")
            if download_and_save_firmware(firmware_url, ssid, password, firmware_path):
                # Reset bootpin -> kích hoạt bootloader STM32
                pina5.value(0)
                pina4.value(0)
                time.sleep(0.5)
                send_firmware_uart(firmware_path)
                time.sleep(0.5)
                pina5.value(1)
                pina4.value(1)
                last_version = current_version
        else:
            print("Không có cập nhật mới.")

        time.sleep(10)
# ==== Thực thi chương trình chính ====
check_and_update_loop()
