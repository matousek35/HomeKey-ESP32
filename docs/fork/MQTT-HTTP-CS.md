# MQTT / HTTP a Loxone

## Stejné zprávy přes MQTT a HTTP (obousměrně)

Firmware nyní vytváří topic/payload jen jednou a stejnou zprávu předává MQTT i
HTTP. HTTP nepotřebuje připojený broker; adresu brokeru lze ponechat prázdnou.
Názvy topiců, vlastní číselné stavy a filtr NFC tagů nastavujete ve stávající
záložce MQTT / HTTP a platí pro oba protokoly. Existující jednoduché tři HTTP notifikace
zůstávají dostupné; vypněte je, pokud by duplikovaly nové mapování.

### ESP → Loxone

V **MQTT / HTTP → HTTP → MQTT-equivalent HTTP messages** zapněte outgoing
mappings. Pro každou zprávu nastavte URL, případně společnou fallback URL.
Prázdná individuální URL použije fallback; prázdný fallback zprávu neposílá.
HTTP GET dosadí `{payload}` a případně `{topic}` do URL s procentovým kódováním.
Příjemce po dekódování získá přesně původní MQTT hodnotu (včetně celého JSON u NFC).
HTTP POST posílá původní payload přímo v těle, Content-Type `text/plain; charset=utf-8`;
URL může obsahovat `{topic}`. Hlavička `X-MQTT-Topic` obsahuje URL-encoded topic.
Přihlašovací údaje pro odchozí HTTP se nastavují ve společných HTTP polích nahoře.

Níže `PREFIX` znamená skutečné `ESP_XXXXXXXX` z vašeho MQTT nastavení:

| Výchozí topic | Směr | Payload a význam |
|---|---|---|
| `PREFIX/homekit/state` | ESP → Loxone | Stejný výpočet jako MQTT: 0 odemčeno, 1 zamčeno; při rozdílu aktuálního/cílového stavu 4 odemykání, 5 zamykání. Při shodě se posílá aktuální číselný stav. |
| `PREFIX/homekit/custom_state` | ESP → Loxone | Nakonfigurovaná vlastní hodnota povelu odemknout/zamknout; pouze se zapnutými custom states. |
| `PREFIX/homekey/auth` | ESP → Loxone | Nezměněný JSON úspěšného HomeKey nebo NFC tagu; respektuje Ignore NFC Tags. |
| `PREFIX/alt_action` | ESP → Loxone | `1` při alternativní akci. |
| `PREFIX/homekit/doorbell` | ESP → Loxone | `1` při stisku GPIO zvonku nebo zvonku klávesnice. MQTT QoS 0, retain=false. |
| `PREFIX/status` | ESP → Loxone | `online` při připojení zařízení; MQTT broker navíc zajišťuje svůj offline last will. |
| `PREFIX/homekit/set_state` | Loxone → ESP | `0`/`1`: aktualizuje současný i cílový stav stejně jako původní MQTT override. |
| `PREFIX/homekit/set_current_state` | Loxone → ESP | `0` odemčeno, `1` zamčeno, `2` zaseknuto, `3` neznámý stav; zpětná vazba, nemění cíl a nevyvolává HW_ACTION. |
| `PREFIX/homekit/set_target_state` | Loxone → ESP | `0`/`1`: požadavek na odemčení/zamčení. |
| `PREFIX/homekit/set_custom_state` | Loxone → ESP | Jedna z šesti nakonfigurovaných custom hodnot; stejné mapování jako MQTT, vyžaduje custom states enabled. |
| `PREFIX/homekit/set_battery_lvl` | Loxone → ESP | `0`–`100`, aktualizace baterie; její zobrazení vyžaduje aktivní battery service. |

Příklad URL pro odchozí stav:
`http://IP_MINISERVERU/dev/sps/io/StavZamku/{payload}`.
Pro zvonek lze poslat hodnotu `1` na virtuální vstup, např.
`http://IP_MINISERVERU/dev/sps/io/Zvonek/{payload}`; v Loxone zpracujte vstup
jako událost/impulz dle svého programu. Pro přímý Loxone příkaz Pulse lze použít
stávající jednoduchou Doorbell press URL končící `/Pulse`.

Fallback pro vlastní HTTP přijímač může mít tvar
`http://IP_PRIJIMACE:8080/event?topic={topic}&payload={payload}`.
Přes fallback se přenášejí i publikace z akcí automatizací `mqtt_publish`;
MQTT klient nemusí být spuštěný. Pokud je MQTT připojené a discovery zapnuté,
jsou přes fallback dostupné i skutečně publikované Home Assistant discovery
zprávy, ale HTTP z nich neudělá Home Assistant MQTT discovery integraci.

### Loxone → ESP

V Security nejdřív zapněte webové přihlášení a nastavte vlastní jméno a heslo.
Potom zapněte **Incoming HTTP commands → Enable HTTP command API**.
Vstupní API používá přihlašovací údaje WEBU ESP, nikoliv údaje odchozího Loxone.
Endpointy nejsou v konfiguračním AP portálu dostupné, až po připojení do sítě.

GET příklad pro potvrzení, že zámek je nově zamčený a cílový stav má být také zamčeno:

```text
http://IP_ESP/api/mqtt?topic=PREFIX%2Fhomekit%2Fset_state&payload=1
```

Použijte skutečný topic z MQTT nastavení. Stejný příkaz z Terminálu (curl si
vyžádá heslo uživatele `USER`):

```sh
curl --user USER --get 'http://IP_ESP/api/mqtt' \
  --data-urlencode 'topic=PREFIX/homekit/set_state' \
  --data-urlencode 'payload=1'
```

POST alternativa se stejným payloadem:

```sh
curl --user USER -H 'Content-Type: text/plain' \
  --data-binary '1' \
  'http://IP_ESP/api/mqtt?topic=PREFIX%2Fhomekit%2Fset_state'
```

HTTP `202 {"accepted":true}` potvrzuje zařazení příkazu, nikoliv mechanické
provedení zámkem. Neznámý topic, vypnutý custom příkaz nebo neplatná hodnota
vrátí 400; bez platného přihlášení 401, vypnuté API 404. Číselné hodnoty se
validují stejně v MQTT i HTTP (nejsou povolené záporné hodnoty, mezery ani
přípony jako `1abc`). API má GET kvůli kompatibilitě s Loxone; používejte ho
jen jako přímé volání, ne jako odkaz vložený do cizí webové stránky.

Pro potvrzení dokončení dříve požadovaného pohybu použijte `set_current_state`.
Například cíl 0 → po odemčení potvrzení aktuálního stavu 0. Pro samostatné
zamčení provedené mimo ESP, kdy je potřeba změnit i původní cíl 0 na 1, použijte
`set_state=1`, stejně jako v MQTT. Tato varianta zachovává i stávající MQTT
chování HW_ACTION/timerů. Kontakt zavřených dveří není samostatný stav zámku;
neposílejte „zamčeno“ jen na základě zavřených dveří bez potvrzení zamčení.
Opakované hlášení stejného aktuálního stavu již negeneruje další stavovou
událost. Loxone nesmí interpretovat každé hlášení stavu jako nový povel.

### Dostupnost, omezení a test

Přihlášený GET na `http://IP_ESP/api/mqtt/status` vrací `availability`,
`stateTopic`, `statePayload`, `currentState` a `targetState`. Loxone může
pravidelným dotazem a timeoutem poznat výpadek zařízení. HTTP nemá broker,
retain, QoS ani offline last will: při vypnutí ESP se žádný offline požadavek
odeslat nemůže. Odchozí fronta stále obsahuje nejvýše 8 událostí, expiraci
5 sekund, síťový timeout 2 sekundy a žádné automatické opakování. Úspěšný HTTP
status sám nepotvrzuje provedení příkazu v Loxone. Maximum payloadu je 8192 B,
topicu 512 B a URL po dosazení 8192 B; nadlimitní zprávy se zahodí s logem.

Pro test bez Loxone lze použít lokální HTTP server spuštěný pomocí `touch zvonek` a `python3 -m http.server 8080` v prázdné složce a URL
`http://IP_MAC:8080/zvonek?topic={topic}&payload={payload}` (soubor `zvonek`
vrací 200). V logu uvidíte topic i hodnotu. Příchozí příkazy vyzkoušejte pomocí
curl; opakované `set_current_state` se stejnou hodnotou nesmí vytvářet smyčku.
MQTT zvonek nyní funguje automaticky. Pokud jste dříve přidali vlastní automatizaci
publikující stejný topic při Doorbell pressed, odstraňte duplicitu.

