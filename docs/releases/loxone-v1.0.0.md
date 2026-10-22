# Loxone v1.0.0 — první souhrnné vydání forku

## Změny

- GPIO zvonek s nastavitelným pinem, zapnutím/vypnutím a filtrováním zákmitů. Výchozí GPIO16 proti GND.
- OV5640 JPEG/MJPEG pro Waveshare, volitelné zapnutí a všechna přiřazení pinů.
- Ethernet je u nové konfigurace vypnutý a lze jej později aktivovat.
- Stejné topic/payload zprávy přes MQTT i HTTP; příchozí HTTP GET/POST používá stejnou obsluhu jako MQTT.
- Automatický MQTT topic zvonku, samostatné HTTP mapování událostí, fallback URL a stavový endpoint.
- Hardware sdružuje zvonek a kameru. Boční panel MQTT / HTTP obsahuje kompletní nastavení komunikace.
- Přehledná česká dokumentace, sjednocené CI a balíčky v Releases se SHA-256 součty.

## Co stáhnout

- **waveshare-wifi-test.zip:** obecný ESP32-S3, alespoň 4 MB quad flash; vhodné pro testovací 8 MB / 2 MB PSRAM desku. Kamera není dostupná.
- **waveshare-camera.zip:** Waveshare 16 MB flash / 8 MB OPI PSRAM s OV5640.
- **waveshare-doorbell.zip:** stejný Waveshare hardware bez kamery.

ZIP obsahuje `merged-binary.bin` pro USB instalaci, `HomeKey-ESP32.bin` pro OTA aplikace, `spiffs.bin` pro OTA webu a build konfiguraci. Obrazy jsou také samostatně s prefixem varianty. Návody obsahuje `documentation.zip` a README repozitáře.

## Aktualizace

Pro kompatibilní nešifrovaný layout tohoto forku aktualizujte **aplikaci i web** ze stejného vydání. OTA zachová NVS a párování. Kompletní merged obraz na adrese 0x0 přepisuje také nastavení a párování; předem zálohujte. Merged obraz nepatří do OTA.

HTTP příkazy vyžadují zapnuté webové přihlášení a samostatně povolené API. HTTP je best effort bez retain, last will a automatických opakování. Mechanické dokončení potvrzuje zpětná vazba, nikoliv HTTP 202.

Základ: upstream main/dev `f167cba1950aa9e37afb9cb06163a78299599a67`, nikoliv release v0.9. Toto je vlastní verzování forku. CI ověřuje build všech tří variant a host testy; fyzické ověření kamery a konkrétního Loxone zapojení zůstává na zařízení.
