# Loxone v1.0.1 — zachování stávajícího zapojení při OTA

## Změna

Nově přidaný GPIO zvonek a kamera jsou ve všech variantách ve výchozím stavu vypnuté. Při OTA ze staršího firmwaru, který jejich nastavení nemá, proto nové funkce nezačnou obsazovat piny. Dříve uložené hodnoty těchto přepínačů se zachovávají; již zapnuté funkce se automaticky nevypínají.

Uložené piny PN532, konfigurace Ethernetu, MQTT, Wi-Fi a párování nejsou touto změnou přepisovány. Pořád platí ochrana proti přidělení obsazeného GPIO.

## Konkrétní provozní instalace

PN532: SS18 / SCK17 / MISO15 / MOSI16. Ethernet: W5500, SPI3, 20 MHz, CS14 / IRQ10 / RST9 / SCK13 / MISO12 / MOSI11. Tyto hodnoty zůstávají při kompatibilní OTA uložené; firmware je nenahrazuje Waveshare výchozím zapojením.

GPIO16 nesmí být současně tlačítko a PN532 MOSI. Waveshare kamera navíc používá GPIO15 a GPIO18, takže ji s uvedeným fyzickým zapojením PN532 nezapínejte. Nejdříve je nutné přepojit čtečku a změnit nastavení.

## Výběr balíčku a OTA

Pro první aktualizaci ESP32-S3 s nejistým typem PSRAM použijte **waveshare-wifi-test.zip**: nevyžaduje PSRAM, používá 4 MB layout a podporuje i již nakonfigurovaný W5500 Ethernet. Název wifi-test neznamená, že vypíná uložený Ethernet. Vyžaduje nejméně 4 MB quad flash. Kamera v této variantě není dostupná; zvonek a HTTP/MQTT lze zapnout dodatečně.

**waveshare-camera.zip** a **waveshare-doorbell.zip** nadále vyžadují Waveshare 16 MB flash / 8 MB OPI PSRAM. V kamerové variantě je driver přítomen, ale nová konfigurace jej nezapne automaticky.

Před aktualizací exportujte konfiguraci a ponechte si původní firmware i možnost lokálního USB obnovení. OTA je určena pro kompatibilní nešifrované rozdělení flash `with_ota.csv`; samotný screenshot pinů rozdělení flash nepotvrzuje.

Nahrajte **HomeKey-ESP32.bin** jako aplikaci a **spiffs.bin** jako LittleFS/web ze stejného balíčku. **merged-binary.bin do OTA nepatří** a kompletní USB zápis přepisuje NVS/párování. Po restartu ověřte NFC, Ethernet a odemykání, teprve potom nastavujte další funkce.

Vydání neobsahuje změnu partition tabulky. Sestavení a hostitelské testy probíhají pro všechny tři varianty; fyzický provoz na konkrétní desce nebyl ověřen automatizovaným buildem.
