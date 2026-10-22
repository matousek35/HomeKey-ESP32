# HomeKey ESP32 — Waveshare / Loxone fork

HomeKey čtečka s PN532, GPIO zvonkem, volitelnou OV5640 a komunikací přes MQTT nebo přímé HTTP bez brokeru. Rozšíření původního projektu [rednblkx/HomeKey-ESP32](https://github.com/rednblkx/HomeKey-ESP32).

**[Stáhnout firmware a OTA v Releases](https://github.com/matousek35/HomeKey-ESP32/releases/latest)** · [Přehled změn](CHANGELOG.md)

## Začít

1. Vyberte firmware podle desky v [návodu k instalaci a OTA](docs/fork/INSTALL-CS.md).
2. Zapojte PN532 a zvonek podle [hardwarového návodu](docs/fork/HARDWARE-CS.md).
3. Pro Loxone nastavte [MQTT / HTTP a zpětnou vazbu zámku](docs/fork/MQTT-HTTP-CS.md).

| Varianta | Hardware | Kamera |
|---|---|---|
| `waveshare-wifi-test` | Obecný ESP32-S3, ≥4 MB quad flash, PSRAM není potřeba | Ne |
| `waveshare-camera` | Waveshare ESP32-S3-POE-ETH-CAM-KIT, 16 MB flash / 8 MB OPI PSRAM | OV5640 JPEG/MJPEG |
| `waveshare-doorbell` | Stejná Waveshare deska | Ne |

Každý release obsahuje kompletní USB obraz, OTA aplikaci a OTA web pro všechny tři varianty. Pro uživatelovu testovací desku 8 MB flash / 2 MB PSRAM je určena varianta **wifi-test**. Kamera neposkytuje audio, RTSP ani HomeKit Secure Video.

## Nastavení

**Misc → Hardware** sdružuje čtečku, Ethernet, zvonek a kameru včetně pinů a přepínačů. **MQTT / HTTP** sdružuje broker, společné topicy, TLS, odchozí HTTP a příchozí HTTP API. MQTT a HTTP mohou běžet samostatně i současně; payloady vznikají společně. HTTP nemá MQTT retain, QoS ani offline last will.

Zvonek standardně publikuje `PREFIX/homekit/doorbell` s hodnotou `1`. HTTP API přijímá stejné příkazy jako MQTT. Stav zavřených dveří a stav zamčeného zámku nejsou totéž; podrobnosti a příklady jsou v integračním návodu.

## Vývoj a původ

[Návod k buildu a vydávání](docs/fork/BUILD-CS.md). Zdroj vychází z upstream `main` commitu [`f167cba`](https://github.com/rednblkx/HomeKey-ESP32/commit/f167cba1950aa9e37afb9cb06163a78299599a67), označení `dev`; **nejde o základ release v0.9**. Vydání tohoto forku používají vlastní řadu `loxone-v1.x`.

Původní dokumentace v `docs/` a licence jsou zachovány. Pokyny v `docs/fork/` mají přednost pro zde distribuované varianty. CI sestavuje všechny tři varianty a testuje debounce i HTTP/MQTT mapování; nenahrazuje zkoušku konkrétního zapojení, kamery a Loxone Miniserveru.

Autorem původního projektu je rednblkx a jeho přispěvatelé. Podmínky použití: [LICENSE](LICENSE).
