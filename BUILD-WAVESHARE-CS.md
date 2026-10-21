# Waveshare ESP32-S3-POE-ETH-CAM-KIT + PN532 + zvonek + OV5640

Tento fork přidává samostatné zvonkové tlačítko a volitelnou JPEG/MJPEG kameru.
Firmware je určený pro Waveshare ESP32-S3-ETH (16 MB Flash, 8 MB OPI PSRAM).
Kamera není HomeKit Secure Video ani audio interkom. Hardwarové ověření na skutečné
sestavě a ověření v konkrétní verzi Loxone jsou stále nutné.

## Zapojení

Čísla označují GPIO, nikoliv pořadí pinů na konektoru. Vypněte napájení před zapojením.
PN532 přepněte fyzickými přepínači do režimu **SPI** podle jeho výrobce.

| Signál | GPIO / připojení |
|---|---|
| Tlačítko zvonku, normálně rozepnutý beznapěťový kontakt | GPIO16 proti GND |
| PN532 SS / CS (některé moduly značí SDA) | GPIO0 |
| PN532 SCK | GPIO17 |
| PN532 MISO | GPIO43 |
| PN532 MOSI | GPIO44 |
| PN532 GND | GND |
| PN532 napájení | dle modulu; logika musí být 3,3 V |
| Pull-up pro CS | 10 kΩ mezi GPIO0 a 3,3 V |
| OV5640 | originální kamerový FPC konektor |

GPIO0 je také BOOT: při resetu musí být HIGH. PN532 nesmí tento pin stahovat dolů.
Za běhu nemačkejte BOOT (zároveň aktivuje CS). Při problému s flashováním odpojte
PN532 CS a použijte BOOT + RESET. GPIO43/44 jsou uvolněné jen v tomto profilu
s logováním přes USB; nesmí na nich být současně UART adaptér nebo Serial0.
Tlačítko má interní pull-up. Pro dlouhý venkovní kabel použijte vhodnou ochranu
vstupu / galvanické oddělení; nepřipojujte na GPIO napětí existujícího zvonku.

W5500 má na desce SCK=13, MISO=12, MOSI=11, CS=14, IRQ=10, RST=9 a používá SPI3.
PN532 používá SPI2. Není potřeba pájet datové vodiče k W5500.
Kamera zabírá GPIO1,2,3,8,15,18,38,39,40,41,42,45,46,47,48.
Nepřiřazujte tyto piny současně relé, klávesnici nebo signalizaci. Alokátor GPIO
při kolizi kameru odmítne spustit; důvod je v logu.

## Nejjednodušší build: GitHub Actions

1. Otevřete Actions ve svém forku. Pokud GitHub zobrazuje upozornění na vypnuté
   workflow ve forku, povolte je.
2. Vyberte **Waveshare doorbell and camera**, **Run workflow** a větev
   `waveshare-doorbell-camera`.
3. Po úspěšném běhu stáhněte artefakt `waveshare-camera`, nebo `waveshare-doorbell`
   pro variantu bez kamery. Uvnitř je `merged-binary.bin` pro první instalaci
   a `HomeKey-ESP32.bin` pro OTA aktualizaci aplikace.
4. V ESP-IDF prostředí použijte např.:

```sh
esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX write-flash 0x0 merged-binary.bin
```

Přesný název USB portu zjistíte po připojení desky; Windows používá například COM5.
Tento workflow sestavuje nešifrovanou variantu a neaktivuje nevratné eFuse změny.
Factory image obsahuje i webové rozhraní. Samotná OTA aplikace webový filesystem
nenahradí. Neprovádějte erase-flash, pokud nechcete přijít o konfiguraci a párování.

## Lokální build

Použijte **ESP-IDF v5.5.5** (aktuální manifest vyžaduje >=5.5.5) a **Bun**.
Instalace IDF: https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/get-started/
Bun: https://bun.sh/docs/installation
Po instalaci aktivujte prostředí ESP-IDF, například `. ~/esp/esp-idf/export.sh`.

```sh
git clone --recursive -b waveshare-doorbell-camera https://github.com/matousek35/HomeKey-ESP32.git
cd HomeKey-ESP32
idf.py -B build-waveshare -DSDKCONFIG=sdkconfig.waveshare -DSDKCONFIG_DEFAULTS='sdkconfig.defaults;profiles/waveshare.defaults;profiles/camera.defaults' set-target esp32s3
idf.py -B build-waveshare -DSDKCONFIG=sdkconfig.waveshare -DSDKCONFIG_DEFAULTS='sdkconfig.defaults;profiles/waveshare.defaults;profiles/camera.defaults' build merge-bin
idf.py -B build-waveshare -DSDKCONFIG=sdkconfig.waveshare -p /dev/cu.usbmodemXXXX flash monitor
```

Build sám zavolá `bun install` a sestaví web. Nepoužívejte lokálně `CI=true`,
pokud jste předem nesestavili `data/dist`.
Pro variantu bez kamery vynechte `;profiles/camera.defaults` a použijte jiné
`-B` a `-DSDKCONFIG`, aby se nepřevzala již uložená konfigurace kamery.
Změna GPIO tlačítka: stejným příkazem spusťte `menuconfig`, sekce
**HomeKey doorbell and Waveshare camera**. Výchozí GPIO16, debounce 60 ms.

## První spuštění / již používaná deska

Profil nastavuje výchozí piny pouze při chybějící konfiguraci v NVS. Stará nastavení
zůstanou zachována. V existující instalaci v nastavení hardwaru zkontrolujte:
PN532 SPI, Custom pins SS=0, SCK=17, MISO=43, MOSI=44; Ethernet enabled,
W5500, SPI3 a výše uvedené ethernetové piny (nebo preset Waveshare ESP32-S3-ETH).
Pokud UI odmítá GPIO0 jako strapping pin, povolte příslušnou volbu po ověření pull-upu.
Vypněte funkce, které by používaly stejné GPIO, a restartujte zařízení.

Stisk zvonku publikuje existující událost `keypad_doorbell` i bez klávesnice.
V editoru automatizací vyberte **Doorbell pressed**; můžete na ni navázat MQTT
publish (např. topic `homekey/doorbell`, payload `1`, retain vypnutý) pro váš
MQTT–Loxone most. Miniserver potřebuje odpovídající integraci; samotné video URL
nepřenáší stisk tlačítka. Tlačítko samo neodemyká zámek.
HomeKit dostává událost SINGLE_PRESS; chování upozornění závisí na HomeKit klientu.
Při držení vznikne jedna událost, další až po stabilním uvolnění a novém stisku.
Tlačítko držené během startu nezazvoní.

## Loxone video

V zařízení **Interkom uživatele / User-defined Intercom** nastavte interní video URL:

```text
http://IP_ESP32:81/stream
```

Pro JPEG snímek / kompatibilní záložní variantu:

```text
http://IP_ESP32/camera.jpg
```

Pokud na hlavním webu aktivujete HTTPS, JPEG je na jeho HTTPS adrese.
MJPEG na portu 81 je vždy HTTP pro lokální Loxone. Obě adresy respektují stejné
HTTP Basic přihlášení jako web. Při zapnuté webové autentizaci nastavte stejné
jméno a heslo v Loxone. Nepublikujte port 81 do internetu; HTTP nešifruje obraz
ani přihlašovací údaje. Vypnutá webová autentizace znamená přístup bez hesla.

Výchozí obraz: JPEG VGA 640×480, kvalita 15, nejvýše přibližně 5 fps.
Stream má vlastní server/task a **jeden současný MJPEG klient**. Při testování
v Loxone zavřete stream v prohlížeči. Pro více klientů použijte lokální video proxy;
chování Loxone při souběžných aplikacích je třeba ověřit. JPEG může při obsazené
kameře vrátit 503; klient může požadavek zopakovat. Nejsou implementovány zvuk,
RTSP, autofocus ani záznam na kartu. Po změně hesla restartujte zařízení,
aby se ukončil i případný dříve autorizovaný stream.

## Ověření na zařízení

- Log musí potvrdit PN532, přidělenou Ethernet IP a `VGA JPEG ready`.
- Otevřete `/camera.jpg` a `:81/stream`; se zapnutou autentizací musí bez hesla
  vracet 401. Totéž ověřte po restartu.
- Ověřte jeden stisk, dlouhé držení, opakované stisky a zapnutí s drženým tlačítkem.
- Během streamu opakovaně vyzkoušejte HomeKey, zvonek a dostupnost webu.
- Vyzkoušejte odpojení/obnovení sítě a opětovné otevření streamu.

Host test debounce: `c++ -std=c++17 -Imain/include tests/doorbell_debounce.cpp -o /tmp/button-test && /tmp/button-test`.

Zdroje: [Waveshare pinout a schéma](https://docs.waveshare.com/ESP32-S3-ETH/Resources-And-Documents),
[signály kamery a Ethernetu](https://docs.waveshare.com/ESP32-S3-ETH/Development-Environment-Setup-Arduino),
[Espressif camera driver](https://github.com/espressif/esp32-camera),
[Loxone podporované formáty](https://www.loxone.com/eses/kb/intercom-personalizado/).
