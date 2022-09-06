set HexFile=nrf52832_xxaa.hex
ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE

ECHO copy _build\%HexFile% APP.hex
copy _build\%HexFile% APP.hex
copy APP.hex hex_release\APP.hex

ECHO nrfutil settings generate --family NRF52 --application APP.hex --application-version 1 --bootloader-version 0 --bl-settings-version 1 bootloader_setting.hex
nrfutil settings generate --family NRF52 --application APP.hex --application-version 1 --bootloader-version 0 --bl-settings-version 1 bootloader_setting.hex
copy bootloader_setting.hex hex_release\bootloader_setting.hex
PAUSE
