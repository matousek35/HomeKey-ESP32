# Hardware: Waveshare, PN532, zvonek a kamera

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


## Vypnutí funkcí a změna pinů

Od v1.0.1 jsou při chybějícím uloženém nastavení kamera i GPIO zvonek vypnuté. Zapněte je až po ověření zapojení. Uložené hodnoty přepínačů se zachovávají.

V **Misc → Hardware** jsou samostatné přepínače **Enable GPIO doorbell** a
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

