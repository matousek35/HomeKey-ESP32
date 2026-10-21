#!/usr/bin/env bash
set -euo pipefail
variant="${1:?Expected doorbell or camera}"
defaults='sdkconfig.defaults;profiles/waveshare.defaults'
case "$variant" in
  camera) defaults="$defaults;profiles/camera.defaults" ;;
  doorbell) ;;
  *) exit 2 ;;
esac
CI=true idf.py -DSDKCONFIG_DEFAULTS="$defaults" build merge-bin
grep -q '^CONFIG_HOMEKEY_DOORBELL=y' sdkconfig
grep -q '^CONFIG_HOMEKEY_WAVESHARE_BOARD=y' sdkconfig
grep -q '^CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y' sdkconfig
grep -q '^CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y' sdkconfig
grep -q '^CONFIG_ARDUINO_USE_USB_JTAG=y' sdkconfig
if [ "$variant" = camera ]; then
  grep -q '^CONFIG_HOMEKEY_WAVESHARE_CAMERA=y' sdkconfig
  grep -q '^CONFIG_SPIRAM_MODE_OCT=y' sdkconfig
fi
cp sdkconfig build/sdkconfig
