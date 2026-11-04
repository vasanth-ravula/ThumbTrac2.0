# ThumbTrac 2.0

Firmware for **ThumbTrac**, a fingerprint- and RFID-based attendance and access device by Relgo.
The device enrolls and matches fingerprints locally. It then reports events (enroll, match, delete) to a
configurable HTTP back end, which maps them to forms in a relationship-management app (for example, an HR attendance form).

## Repository layout

```
firmware/
  thumbtrac-noble/relgochhavi/   Current firmware (Dec 2024). ESP32 + FPC BM-Lite sensor, Wi-Fi only (no BLE).
  thumbtrac-ble/relgochhavi/     Earlier ESP32 firmware (Jan 2024) with BLE provisioning and a separate admin page (adminhtml.h).
legacy/
  esp8266-rfid-v1/               First RFID prototype (NodeMCU ESP8266, MFRC522), PlatformIO project.
  esp8266-rfid/                  Later RFID prototype (NodeMCU ESP8266), PlatformIO project.
  esp8266-fingerprint/           Fingerprint prototype (NodeMCU ESP8266, Adafruit fingerprint sensor), PlatformIO project.
docs/
  architecture.md                How the device, the command protocol and the server fit together.
```

Each sketch folder under `firmware/` is named `relgochhavi` because the Arduino IDE requires a sketch's folder to have the same name as its `.ino` file.

## Hardware (current firmware)

- ESP32 dev module (built with the `esp32:esp32` Arduino core)
- Fingerprint Cards (FPC) BM-Lite sensor over SPI. The host-side protocol stack is in `fpc_*`, `com_*`, `bep_host_if.*` and `platform.*` (Apache-2.0, Fingerprint Cards AB).
- SSD1306 OLED and/or 16x2 character LCD for status

## Building (ESP32 firmware)

1. Install the Arduino IDE 2.x and the **esp32 by Espressif** board package.
2. Install these libraries: Adafruit GFX, Adafruit SSD1306, LiquidCrystal, List (`List.hpp`), AsyncTCP, AsyncElegantOTA.
   `ESPAsyncWebServer` and `HTTPClient` are vendored in the sketch folder, with local changes.
3. Open `firmware/thumbtrac-noble/relgochhavi/relgochhavi.ino`, select your ESP32 board and upload.

The legacy ESP8266 projects build with PlatformIO (`pio run` inside each folder).

## First-time device setup

No credentials are compiled into the firmware. On first boot (or after `/resetDevice`) the device starts an open
access point named `RelgoWifi`. Connect to it, open the device's IP address in a browser, and submit the configuration
form. The firmware calls `/configureDevice` with the Wi-Fi SSID and password, location, time zone, static IP and back-end server URL,
and stores these values in SPIFFS.

Device HTTP endpoints: `/`, `/configurePage`, `/configureDevice`, `/addDevice`, `/addListner`, `/getDeviceType`,
`/deviceMode`, `/enrollUser`, `/matchFinger`, `/executeCommand`, `/getTemplate`, `/getTemplateIds`, `/putTemplate`,
`/resetDevice`, plus `/update` for OTA.

## Security notes for anyone analyzing this code

- The provisioning access point is open (no password), and the HTTP endpoints have no authentication.
- Traffic to the back end uses plain HTTP.
- The legacy ESP8266 projects use `WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")` placeholders. Fill them in locally and do not commit real values.

## Sanitization

This public repository was assembled from internal working folders. Before publishing:
real Wi-Fi credentials, employee e-mail addresses, production server hosts/IPs and device/relation GUIDs were
replaced with placeholders (`YOUR_WIFI_*`, `user@example.com`, `server.example.com`, `00000000-0000-0000-0000-000000000000`).
Build outputs, ZIP snapshots, IDE metadata, server-side code and internal notes were left out.

## License

MIT. See `LICENSE` for this project's code and `THIRD_PARTY_NOTICES.md` for vendored components.
