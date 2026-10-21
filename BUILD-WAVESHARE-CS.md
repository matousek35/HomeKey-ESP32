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
   a `HomeKey-ESP32.bin` pro OTA aktualizaci aplikace. `spiffs.bin` obsahuje webové rozhraní (LittleFS).
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
GPIO tlačítka lze změnit ve webovém nastavení **Misc → Doorbell / Camera**.
Výchozí GPIO16, debounce 60 ms; menuconfig mění pouze výchozí hodnotu pro novou konfiguraci.

## První spuštění / již používaná deska

Nová konfigurace má Ethernet **vypnutý**. Bez uložených Wi-Fi údajů se spustí
konfigurační síť `HK_XXXXXXXX`, výchozí heslo `HomeKey$123$`. Připojte se k ní
 a otevřete konfigurační portál; nastavte domácí Wi-Fi. Ethernet lze dodatečně
zapnout v **Misc → Hardware** po nastavení typu adaptéru a pinů. Změna se použije
po uložení a restartu. U Waveshare použijte W5500/SPI3 a piny uvedené výše.

Změna výchozí hodnoty nepřepíše již uložené `ethernetEnabled=true` v NVS.
Tento Waveshare build nadále vyžaduje ESP32-S3 s 16 MB flash a 8 MB OPI PSRAM;
vypnutí Ethernetu z něj nedělá univerzální firmware pro jiné ESP desky.


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


## Přímé HTTP události do Loxone (bez MQTT)

V **Misc → Doorbell / Camera** zapněte HTTP notifications. Zadejte jednotlivé
adresy pro zazvonění, zamčení a odemčení. Prázdná adresa danou událost vypne.
Příklad pro virtuální vstupy `Zvonek` a `Zamek` vytvořené v Loxone Config:

| Událost | Příklad URL |
|---|---|
| Zazvonění | `http://IP_MINISERVERU/dev/sps/io/Zvonek/Pulse` |
| Zamčeno | `http://IP_MINISERVERU/dev/sps/io/Zamek/1` |
| Odemčeno | `http://IP_MINISERVERU/dev/sps/io/Zamek/0` |

Jméno a heslo uživatele s oprávněním k těmto vstupům vyplňte do samostatných polí
HTTP Basic username/password, nikoliv do URL. Všechny tři adresy používají stejné
přihlášení. Příklady nahraďte IP a identifikátory svých vstupů; zvláštní znaky
v cestě URL musí být procentově zakódované. Podklady:
[Loxone Webservices](https://www.loxone.com/cscz/kb/api-webservices/).
Konkrétní generace/verze a oprávnění Miniservéru se musí ověřit v instalaci.
HTTPS ověřuje certifikát vůči standardnímu CA bundle; self-signed certifikáty
bez důvěryhodného řetězce nejsou podporované. HTTP používejte v důvěryhodné lokální síti.

Přenos probíhá metodou GET na samostatném tasku, bez MQTT. MQTT lze zcela zastavit
vymazáním adresy brokeru v jeho nastavení. HTTP funguje i pokud broker neexistuje.
Zvonek z GPIO i z aktivní klávesnice používá stejnou událost. Stav zámku se posílá
při změně **aktuálního stavu ve firmwaru** na zamčeno/odemčeno, nikoliv při pouhé
změně cíle. Bez čidla zpětné vazby to není potvrzení mechanické polohy zámku.
Stavy jammed/unknown/locking/unlocking vlastní URL nemají. Po startu se stav
automaticky nesynchronizuje; jde o hlášení změn, nikoliv MQTT retained náhradu.

Fronta má 8 událostí, události starší než 5 sekund se zahazují. Síťový timeout
je 2 sekundy; automatické opakování a přesměrování jsou vypnuté, aby se nezopakoval
již provedený impulz. Výpadky mohou způsobit ztrátu události. Log `HTTPNotify`
ukazuje HTTP stav, nikoliv URL nebo heslo; úspěch HTTP 2xx nepotvrzuje aplikování
příkazu uvnitř Loxone. Po nastavení ověřte vstupy v Live View.

## Vypnutí funkcí a změna pinů

Ve stejné záložce jsou samostatné přepínače **Enable GPIO doorbell** a
**Enable camera** a seznam všech 15 kamerových signálů včetně napájecího enable.
Uložením se zařízení restartuje; vypnutá funkce si po restartu nezabere GPIO.
Vypnutí GPIO zvonku neovlivňuje zvonek samostatné klávesnice.
Kamera vyžaduje variantu `waveshare-camera`, nelze ji zapnout v binárce bez driveru.
Při přechodu z varianty bez kamery na kamerovou variantu ji zapněte ručně,
pokud už byla v nastavení uložena hodnota vypnuto.

Piny FPC konektoru Waveshare jsou fyzicky pevné: pro dodaný kit ponechte výchozí
přiřazení. Změna v UI nedokáže změnit zapojení desky. Kontrola odmítá duplicitní
piny kamery/zvonku, rezervované piny a piny obsazené jinou běžící funkcí.
Při přesunu pinu z jiné funkce ji nejdříve vypněte/přepojte, uložte a restartujte.
Při kolizi z importovaného nastavení firmware dotčenou funkci nespustí; zkontrolujte log.

## OTA této aktualizace

Pro kompatibilní nešifrovanou instalaci ESP32-S3 se stejným rozdělením flash:

1. Zálohujte konfiguraci a stáhněte oba soubory ze stejného úspěšného buildu.
2. V OTA firmware nahrajte `HomeKey-ESP32.bin` a nechte zařízení restartovat.
3. V OTA aktualizaci **LittleFS / web UI** nahrajte `spiffs.bin` (jde o LittleFS
   obraz, název odpovídá partition labelu). Potom znovu načtěte stránku bez cache.
4. Otevřete novou záložku, nastavte URL a ověřte zvonek i obě změny zámku.

`merged-binary.bin` není soubor pro OTA. Při odlišném partition layoutu nebo
šifrování nelze kompatibilitu OTA předpokládat; použijte odpovídající postup pro
aktuální firmware. Nastavení v NVS aktualizace aplikace/webu nemaže.

## Výchozí verze a kontroly

Základ je upstream `main` commit `f167cba1950aa9e37afb9cb06163a78299599a67`
(označení `dev`), nikoliv release tag `v0.9`. K datu doplnění funkcí je poslední
upstream release `v0.9`; tento fork nadále zachovává dosavadní vývojový základ.
CI sestavuje oba firmwary a webové rozhraní. Host testy ověřují debounce a
výběr HTTP událostí, expiraci a základní validaci URL. Funkční zkouška na
ESP32/OV5640 a vašem Miniservéru zůstává nutná.


## Testování na jiné desce ESP32-S3

Artefakt **waveshare-wifi-test** je určený pro ESP32-S3 s alespoň 4 MB flash.
Nevyžaduje PSRAM (nepoužívá ji), neobsahuje kamerový driver a nepoužívá pevné
Waveshare přiřazení NFC/Ethernetu. HTTP události, webové nastavení a GPIO zvonek
zůstávají dostupné; GPIO tlačítka je výchozí 16. Piny PN532 nastavte podle
skutečného zapojení vlastní desky. Kamera v této variantě nejde zapnout.

Na čisté instalaci je Ethernet vypnutý. Připojte se k Wi-Fi `HK_XXXXXXXX`
heslem `HomeKey$123$` a otevřete `http://192.168.4.1`. Nastavte Wi-Fi domácí sítě.
Ethernet lze následně zapnout v nastavení Hardware, ale vyžaduje skutečný externí
Ethernet adaptér (např. W5500) a správné piny. Samotný ESP32-S3 Ethernet konektor nemá.

Přes USB použijte `merged-binary.bin` z tohoto artefaktu na adrese 0x0.
Kompletní merged obraz přepisuje i mezery obsahující NVS, proto počítejte se
ztrátou konfigurace a párování. Naopak OTA pouze aplikace zachová dříve uloženou
volbu Ethernetu; samotná změna výchozí hodnoty ji nevypne.
