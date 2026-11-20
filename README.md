# RIT - IOT App (NodeMCU ESP8266)
- `index.html` = phone app (upload to GitHub Pages). Tap **!** on any sensor/RFID screen for pins + full code.
- `firmware/` = one ready sketch per project (folder name = sketch name). Upload ONE at a time to the NodeMCU.
- Start with `firmware/Home_Control` for gesture + voice relay control.
- Arduino IDE: add ESP8266 board package; libraries: LiquidCrystal_I2C, DHT sensor library, MFRC522 (Servo is built in).
- Offline: join WiFi `RIT-IOT` (pass `12345678`), app IP `192.168.4.1`. Online: set STA_SSID/STA_PASS in the sketch.
