echo Start

set AppSource=..\_build\vt_sensor.hex
set App=vt_sensor.hex
set SoftDevice=s112_nrf52_6.1.1_softdevice.hex
set SoftDeviceVersion=0xB8
set BootSource=..\..\..\..\..\..\dfu\secure_bootloader\pca10056e_ble\arm5_no_packs\_build\nrf52811_bootloader.hex
set BootLoader=nrf52811_bootloader.hex
set ZipPackage=VT_SENSOR

copy %BootSource% %BootLoader%
copy %AppSource% %App%

echo Creating zip package...
set /p Version="Enter application version (1 - 10000): "
echo Application version %Version%

echo Create DFU File
nrfutil pkg generate --hw-version 52 --sd-req 0xB8 --application-version %Version% --application %App% --key-file private.key %ZipPackage%_v%Version%.zip

echo Erase full chip
nrfjprog -f NRF52 -e

echo Gen bootloader_setting
nrfutil settings generate --family NRF52810 --application %App% --application-version 0 --bootloader-version 0 --bl-settings-version 2 bootloader_setting.hex

echo Merge hex
mergehex --merge bootloader_setting.hex %BootLoader% --output 1.hex
mergehex --merge 1.hex %SoftDevice% --output 2.hex
mergehex --merge 2.hex %App% --output 3.hex

echo Program firmware
nrfjprog -f NRF52 --program 3.hex --reset

ECHO nrfjprog -f NRF52 --rbp ALL
ECHO nrfjprog -f NRF52 --rbp ALL


echo Delete file
ECHO del 1.hex
ECHO del 2.hex
ECHO del bootloader_setting.hex

echo Finish
pause