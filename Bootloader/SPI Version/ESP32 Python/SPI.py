from machine import Pin, SPI
import time
import network
import os
import struct
import urequests
import ubinascii
import json

# SPI master trên ESP32 (HSPI hoặc VSPI đều được)
# Ví dụ dùng HSPI (id=1)
spi = SPI(1,
          baudrate=10000000,
          polarity=0, phase=1,
          sck=Pin(14),
          mosi=Pin(13),
          miso=Pin(12))
boot = Pin(27, Pin.OUT)
rstPin = Pin(26, Pin.OUT)
cs = Pin(15, Pin.OUT)

boot.value(1)
rstPin.value(1)
cs.value(1)  # CS high

# Github Token
GITHUB_TOKEN = ""
version_url = "https://api.github.com/repos/DuongSlayer-Uet/Embedded/contents/Firmware/version.txt"
firmware_url = "https://api.github.com/repos/DuongSlayer-Uet/Embedded/contents/Firmware/blinkled.bin"

path = "blinkgithub.bin"
ssid = "Iphone"
password = "duongbuihihi"

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

# Brief: Lấy file version hiện tại
# Param: api_url - đường dẫn đến file trên github
# Param: ssid
# Param: password


def get_version(api_url):
    try:
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
            app_infor = content_decoded.split("\n")
            app_id = app_infor[0].split(":")[1].strip()
            app_ver = app_infor[1].split(":")[1].strip()
            app_entryaddr = app_infor[2].split(":")[1].strip()
            print(f"[ESP32] APP_ID = {app_id}")
            print(f"[ESP32] VERSION = {app_ver}")
            print(f"[ESP32] ENTRY_ADDR = {app_entryaddr}")
            result = {
                "APP_ID": app_id,
                "VERSION": app_ver,
                "ENTRY_ADDR": app_entryaddr
            }
            return result
        else:
            print("Lỗi HTTP:", r.status_code)
            return None
    except Exception as e:
        print("Lỗi khi lấy version từ API:", e)
        return None

# Brief: Hàm download firmware từ github bằng token
# Param: api_url (link firmware), ssid, password, save_path


def download_firmware(api_url, save_path="/blinkgithub.bin"):
    try:
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


def send_firmware_spi(file_path):
    print("[ESP32] Đang gửi firmware qua spi...")

    try:
        with open(file_path, 'rb') as f:
            total = 0
            while True:
                byte = f.read(1)
                if not byte:
                    break

                spi.write(byte)

                total += 1
                if total % 128 == 0:
                    print(f"[ESP32] Đã gửi: {total} bytes")
                # time.sleep(0.05)  # Delay 5ms
        print("[ESP32] Gửi hoàn tất.")
    except Exception as e:
        print("Lỗi khi gửi spi:", e)
# Brief: tính crc cho từng byte
# Retval: trả về crc result
# Param: crc - Giá trị CRC trước đó (khởi tạo sẽ là 0xFFFFFFFF)
# Param: data - byte dữ liệu cần ghi thêm vào


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
# Param: đường dẫn tới file


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
                data_word = (chunk[3] << 24) | (
                    chunk[2] << 16) | (chunk[1] << 8) | chunk[0]
                crc = crc32(crc, data_word)

        print("[ESP32] CRC32: 0x{:08X}".format(crc))
        return crc
    except Exception as e:
        print("Lỗi:", e)
        return None


connect_wifi(ssid, password)
app_infor = get_version(version_url)
download_firmware(firmware_url)
# Get file size
size = os.stat(path)[6]
print("[ESP32] Size: ", size)
# Đổi thành bytes little-endian (4 byte)
# < = little-endian, I = unsigned int 4 bytes
size_littleEDN = struct.pack("<I", size)
# Get CRC32
crc_result = crc_cal(path)
# Đổi CRC thành bytes little-endian (4 byte)
crc_littleEDN = struct.pack("<I", crc_result)
# Get ver
id = app_infor["APP_ID"]
ver = app_infor["VERSION"].split(".")
major = ver[0]
minor = ver[1]
patch = ver[2]
# Get entry addr
entry_addr = app_infor["ENTRY_ADDR"]
entry_addr_int = int(entry_addr, 16)
entry_addr_bytes = entry_addr_int.to_bytes(4, 'little')

# KÉO RESET PIN ĐỂ VÀO BOOTMODE,
# SAU ĐÓ KÉO CS PIN VÀ BOOT PIN ĐỂ BẮT ĐẦU NHẬN FIRMWARE
rstPin.value(0)
time.sleep(0.01)
cs.value(0)  # Chọn slave
boot.value(0)
rstPin.value(1)
rstPin = Pin(26, Pin.IN)  # nhả chân rst
spi.write(b"BOOT")
spi.write(bytes([int(id)]))
spi.write(bytes([int(major), int(minor), int(patch)]))
spi.write(bytes(size_littleEDN))  # SIZE
spi.write(bytes(crc_littleEDN))  # CRC
spi.write(entry_addr_bytes)
time.sleep(0.3)
send_firmware_spi(path)
boot.value(1)
cs.value(1)
