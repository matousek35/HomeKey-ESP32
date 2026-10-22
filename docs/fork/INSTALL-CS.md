# Instalace a OTA (macOS)

Stahujte z [Releases](https://github.com/matousek35/HomeKey-ESP32/releases). Vyberte jednu variantu:

| Balíček | Použití |
|---|---|
| `waveshare-wifi-test.zip` | Běžný ESP32-S3 s alespoň 4 MB quad flash; vhodné i pro ověřenou desku 8 MB flash / 2 MB PSRAM. PSRAM se nepoužívá, kamera není dostupná. |
| `waveshare-camera.zip` | Waveshare ESP32-S3-POE-ETH-CAM-KIT, 16 MB flash / 8 MB OPI PSRAM, OV5640. |
| `waveshare-doorbell.zip` | Stejný Waveshare hardware, bez kamerového driveru. |

Všechny varianty podporují PN532, GPIO zvonek, MQTT a obousměrné HTTP. Nová konfigurace má Ethernet vypnutý; lze jej zapnout později v Hardware. Stávající konfigurace se při OTA zachovává.

## Aktualizace přes web OTA

Pro dosavadní nešifrované instalace tohoto forku se stejným `with_ota.csv`:

1. Zálohujte nastavení. Stáhněte ZIP odpovídající vašemu hardwaru.
2. Do OTA aplikace nahrajte **HomeKey-ESP32.bin** a počkejte na restart.
3. Do aktualizace **LittleFS / web UI** nahrajte **spiffs.bin** ze stejného vydání. Název souboru je historický; obsah je LittleFS.
4. Znovu načtěte web bez cache. Hardware obsahuje zvonek a kameru; MQTT / HTTP obsahuje komunikaci.

Stejné soubory jsou v Releases i samostatně, s prefixem varianty. OTA aplikace/webu nemaže NVS ani párování. `merged-binary.bin` do OTA **nepatří**. Při jiném rozdělení flash nebo šifrování nejdříve ověřte kompatibilitu.

## Kompletní instalace přes USB

Kompletní obraz přepisuje také NVS: počítejte se ztrátou nastavení a párování. Předem exportujte konfiguraci.

Pokud ještě nemáte esptool prostředí:

```sh
python3 -m venv ~/esp-flash-env
~/esp-flash-env/bin/python -m pip install esptool
```

Rozbalte vybraný ZIP a otevřete Terminál v jeho složce. Ověřte port příkazem `ls /dev/cu.usbmodem*`; skutečný název dosaďte:

```sh
~/esp-flash-env/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbmodem1101 --baud 460800 write-flash 0x0 merged-binary.bin
```

Pokud se zařízení nespojí, podržte BOOT, stiskněte RESET a BOOT uvolněte. PN532 připojený na GPIO0 může ovlivňovat boot; podle potřeby odpojte jeho CS. Po zápisu restartujte.

## První spuštění

Připojte se k Wi-Fi `HK_XXXXXXXX`, heslo `HomeKey$123$`, a otevřete `http://192.168.4.1`. Nastavte domácí Wi-Fi. Potom otevřete IP zařízení v místní síti.

- **Misc → Hardware:** piny PN532, Ethernet, zapnutí a GPIO zvonku, kamera a její piny.
- **MQTT / HTTP → Broker / Topics / SSL/TLS:** MQTT a společné topicy; prázdná adresa brokeru MQTT vypne.
- **MQTT / HTTP → HTTP:** jednoduché notifikace, shodné topic/payload zprávy a příchozí povely.
- **Misc → Security:** webové přihlášení, povinné pro příchozí HTTP API.

Uložení nastavení restartuje zařízení. Před změnou další komunikační sekce uložte aktuální sekci. Výchozí GPIO zvonku je 16: krátce spojte GPIO16 s GND. Číslo znamená GPIO, nikoliv pořadí pinu na konektoru. Přiřazení musí odpovídat vaší desce.

Při použití Waveshare zapněte Ethernet až se správným nastavením W5500/SPI3. Vypnutí Ethernetu nezmění hardwarové nároky kamerové binárky; na desku 8 MB / 2 MB používejte `waveshare-wifi-test`.
