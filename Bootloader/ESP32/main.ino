#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

WebServer server(80);
File uploadFile;

const char* ap_ssid = "ESP32_OTA";
const char* ap_pass = "12345678";

#define UART_TX_PIN 32
#define UART_RX_PIN 33
#define STM32_RST_PIN   26
#define STM32_BOOT_PIN  27

#define FW_PATH   "/firmware.bin"
#define VER_PATH  "/version.txt"

#define BLOCK_SIZE 512
#define ACK_BYTE   0xAA
#define ACK_TIMEOUT 3000  // ms

typedef struct {
  uint32_t magic;     // 4
  uint8_t  ver[3];    // 3  (major, minor, patch)
  uint8_t  app_id;    // 1
  uint32_t size;      // 4
  uint32_t crc;       // 4
  uint32_t entry;     // 4
} fw_header_t;



bool waitForAck()
{
  unsigned long start = millis();
  while (millis() - start < ACK_TIMEOUT) {
    if (Serial2.available()) {
      uint8_t b = Serial2.read();
      if (b == ACK_BYTE) {
        return true;
      }
    }
  }
  return false; // timeout
}


void sendFileToUART(const char* path)
{
  File f = SPIFFS.open(path, "r");
  if (!f) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("Start sending file via UART...");

  uint8_t buf[BLOCK_SIZE];
  uint32_t blockCnt = 0;

  while (f.available()) {
    size_t len = f.read(buf, BLOCK_SIZE);

    // Send 1 block
    Serial2.write(buf, len);
    Serial2.flush();   // đảm bảo đẩy hết UART TX

    Serial.printf("Block %lu sent (%d bytes), waiting ACK...\n",
                  blockCnt++, len);

    // Wait ACK
    if (!waitForAck()) {
      Serial.println("ACK timeout! Abort sending.");
      f.close();
      return;
    }

    Serial.println("ACK received");
  }

  f.close();
  Serial.println("Send file done");
}

bool readVersionFile(const char* path,
                     uint8_t &app_id,
                     uint8_t ver[3],
                     uint32_t &entry_addr)
{
  File f = SPIFFS.open(path, "r");
  if (!f) {
    Serial.println("Failed to open version file");
    return false;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();

    if (line.startsWith("APP_ID:")) {
      app_id = line.substring(7).toInt();
    }
    else if (line.startsWith("VERSION:")) {
      String v = line.substring(8);
      v.trim();

      int p1 = v.indexOf('.');
      int p2 = v.lastIndexOf('.');

      ver[0] = v.substring(0, p1).toInt();
      ver[1] = v.substring(p1 + 1, p2).toInt();
      ver[2] = v.substring(p2 + 1).toInt();
    }
    else if (line.startsWith("ENTRY_ADDR:")) {
      String s = line.substring(11);
      s.trim();
      entry_addr = strtoul(s.c_str(), NULL, 16);
    }
  }

  f.close();
  return true;
}

uint32_t crc32_update_word(uint32_t crc, uint32_t data)
{
    crc ^= data;
    for (int i = 0; i < 32; i++) {
        if (crc & 0x80000000)
            crc = (crc << 1) ^ 0x04C11DB7;
        else
            crc <<= 1;

        crc &= 0xFFFFFFFF;
    }
    return crc;
}


uint32_t calcFileCRC32(const char* path)
{
    File f = SPIFFS.open(path, "r");
    if (!f) {
        Serial.println("Failed to open firmware file");
        return 0;
    }

    uint32_t crc = 0xFFFFFFFF;
    while (true) {
        uint8_t buf[4];
        int len = f.read(buf, 4);
        if (len <= 0) break;

        // Padding 0xFF nếu < 4 byte
        for (int i = len; i < 4; i++) {
            buf[i] = 0xFF;
        }

        // Little-endian → word 32-bit
        uint32_t data_word =
            ((uint32_t)buf[3] << 24) |
            ((uint32_t)buf[2] << 16) |
            ((uint32_t)buf[1] << 8)  |
            ((uint32_t)buf[0]);

        crc = crc32_update_word(crc, data_word);
    }

    f.close();
    return crc;
}


bool buildFirmwareHeader(fw_header_t &hdr)
{
  uint8_t app_id;
  uint8_t ver[3];
  uint32_t entry;

  if (!readVersionFile(VER_PATH, app_id, ver, entry)) {
    return false;
  }

  File fw = SPIFFS.open(FW_PATH, "r");
  if (!fw) {
    Serial.println("Failed to open firmware");
    return false;
  }

  uint32_t fw_size = fw.size();
  fw.close();

  uint32_t fw_crc = calcFileCRC32(FW_PATH);

  // ==== Fill header ====
  hdr.magic = 0x544F4F42;   // "BOOT"
  hdr.app_id = app_id;
  memcpy(hdr.ver, ver, 3);
  hdr.size = fw_size;
  hdr.crc  = fw_crc;
  hdr.entry = entry;

  return true;
}

void sendHeaderToUART(const fw_header_t &hdr)
{
  const uint8_t* p = (const uint8_t*)&hdr;
  size_t len = sizeof(fw_header_t);

  Serial.println("Sending header to STM32...");

  for (size_t i = 0; i < 20; i++) {
    Serial2.write(p[i]);   // UART tới STM32
  }

  Serial1.flush();
  Serial.println("Header sent");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.println("UART2 ready on GPIO32");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  SPIFFS.begin(true);

  // Trang chính
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<h2>ESP32 OTA Upload</h2>"

      "<h3>Upload Firmware</h3>"
      "<form method='POST' action='/upload_fw' enctype='multipart/form-data'>"
      "<input type='file' name='file'>"
      "<input type='submit' value='Upload Firmware'>"
      "</form>"

      "<h3>Upload Version</h3>"
      "<form method='POST' action='/upload_ver' enctype='multipart/form-data'>"
      "<input type='file' name='file'>"
      "<input type='submit' value='Upload Version'>"
      "</form>"

      "<hr>"

      "<h3>Program STM32</h3>"
      "<form method='POST' action='/program'>"
      "<input type='submit' value='FLASH STM32'>"
      "</form>"
    );
  });

  // Upload firmware
  server.on("/upload_fw", HTTP_POST,
    []() {
      server.send(200, "text/plain", "Firmware Upload OK");
    },
    []() {
      HTTPUpload& up = server.upload();

      if (up.status == UPLOAD_FILE_START) {
        Serial.printf("Firmware upload start: %s\n", up.filename.c_str());
        uploadFile = SPIFFS.open(FW_PATH, "w");
        if (!uploadFile) {
          Serial.println("Failed to open firmware file");
        }
      }
      else if (up.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
          uploadFile.write(up.buf, up.currentSize);
        }
      }
      else if (up.status == UPLOAD_FILE_END) {
        if (uploadFile) uploadFile.close();
        Serial.printf("Firmware upload end, size: %u bytes\n", up.totalSize);
      }
    }
  );

  // Upload version
  server.on("/upload_ver", HTTP_POST,
    []() {
      server.send(200, "text/plain", "Version Upload OK");
    },
    []() {
      HTTPUpload& up = server.upload();

      if (up.status == UPLOAD_FILE_START) {
        Serial.printf("Version upload start: %s\n", up.filename.c_str());
        uploadFile = SPIFFS.open(VER_PATH, "w");
      }
      else if (up.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
          uploadFile.write(up.buf, up.currentSize);
        }
      }
      else if (up.status == UPLOAD_FILE_END) {
        if (uploadFile) uploadFile.close();
        Serial.printf("Version upload end, size: %u bytes\n", up.totalSize);
      }
    }
  );

  // Flash STM32
  server.on("/program", HTTP_POST, []() {
    if (!SPIFFS.exists(FW_PATH)) {
      server.send(400, "text/plain", "No firmware found");
      return;
    }

    server.send(200, "text/plain", "Programming STM32...");
    Serial.println("User triggered STM32 programming");
    fw_header_t header;
    if (buildFirmwareHeader(header)) {
    Serial.println("Header build OK");
    Serial.printf("APP_ID: %d\n", header.app_id);
    Serial.printf("VER: %d.%d.%d\n",
                  header.ver[0], header.ver[1], header.ver[2]);
    Serial.printf("SIZE: %lu\n", header.size);
    Serial.printf("CRC:  0x%08lX\n", header.crc);
    Serial.printf("ENTRY:0x%08lX\n", header.entry);
    }


    // ***BOOTING***
      // RESET = 0
    pinMode(STM32_RST_PIN, OUTPUT);
    digitalWrite(STM32_RST_PIN, LOW);
      // BOOT0 = 0
    pinMode(STM32_BOOT_PIN, OUTPUT);
    digitalWrite(STM32_BOOT_PIN, LOW);
    delay(10);
      // RESET = 1 (nhả reset)
    digitalWrite(STM32_RST_PIN, HIGH);
      // Thả reset pin (tránh giữ cưỡng bức)
    pinMode(STM32_RST_PIN, INPUT);
    delay(50);

    sendHeaderToUART(header);
    sendFileToUART(FW_PATH);

    // BOOT0 = 0
    pinMode(STM32_BOOT_PIN, OUTPUT);
    digitalWrite(STM32_BOOT_PIN, HIGH);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
