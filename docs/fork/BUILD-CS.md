# Sestavení a vydávání

Použijte ESP-IDF **v5.5.5**, Bun a Git se submoduly. Binárky mají nešifrované OTA rozdělení `with_ota.csv`; běžná instalace nevyžaduje lokální build.

```sh
git clone --recursive https://github.com/matousek35/HomeKey-ESP32.git
cd HomeKey-ESP32
. ~/esp/esp-idf/export.sh
idf.py -B build-camera -DSDKCONFIG=sdkconfig.camera -DSDKCONFIG_DEFAULTS='sdkconfig.defaults;profiles/waveshare.defaults;profiles/camera.defaults' set-target esp32s3
idf.py -B build-camera -DSDKCONFIG=sdkconfig.camera -DSDKCONFIG_DEFAULTS='sdkconfig.defaults;profiles/waveshare.defaults;profiles/camera.defaults' build merge-bin
```

Lokální CMake sestaví web pomocí Bun. Nepřidávejte `CI=true`, pokud nemáte předem sestavené `data/dist`.

| Varianta | SDKCONFIG_DEFAULTS |
|---|---|
| `waveshare-camera` | `sdkconfig.defaults;profiles/waveshare.defaults;profiles/camera.defaults` |
| `waveshare-doorbell` | `sdkconfig.defaults;profiles/waveshare.defaults` |
| `waveshare-wifi-test` | `sdkconfig.defaults;profiles/esp32s3-wifi.defaults` |

Pro každou variantu použijte vlastní build adresář i sdkconfig. Výstupy: `merged-binary.bin`, `HomeKey-ESP32.bin`, `spiffs.bin`.

## CI a Releases

Workflow **Firmware and Releases** sestaví všechny tři varianty, web a hostitelské C++ testy. Ruční spuštění je možné v Actions. CI hlídá velikost aplikace a vytvoření LittleFS obrazu v původním partition layoutu.

Vydání se vytvoří pouze z větve `main` po úspěchu všech variant. Verze je v `VERSION`; poznámky v `docs/releases/<verze>.md`. Pro nové vydání změňte oba soubory a CHANGELOG. Existující tag se nepřepisuje. Release vzniká nejdříve jako draft, po nahrání všech souborů se zveřejní. Nedokončený draft lze opakovaným spuštěním dokončit, pokud odkazuje na stejný commit.

Každé vydání obsahuje tři ZIP balíčky, samostatné instalační a OTA obrazy, SHA256SUMS a dokumentaci. V ZIPu jsou navíc `sdkconfig` a `flasher_args.json`. GitHub Actions artefakty slouží pro vývoj; uživatelské soubory stahujte z Releases.

Původní upstream workflows pro jiné čipy, šifrované varianty a publikování wiki byly ve forku nahrazeny tímto workflow. Zdrojový kód, licence, upstream dokumentace i historie zůstávají zachované. Ostatní čipy nejsou součástí ověřované distribuční matice tohoto forku.
