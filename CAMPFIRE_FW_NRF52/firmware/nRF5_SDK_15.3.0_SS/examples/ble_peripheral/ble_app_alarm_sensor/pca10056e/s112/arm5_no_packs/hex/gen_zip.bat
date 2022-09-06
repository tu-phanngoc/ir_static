echo Start

set AppSource=..\_build\vt_sensor.hex
set App=vt_sensor.hex
set SoftDevice=s112_nrf52_6.1.1_softdevice.hex
set SoftDeviceVersion=0xB8
set BootLoader=nrf52811_bootloader.hex
set ZipPackage=VT_SENSOR

copy %AppSource% %App%

echo Creating zip package...
set /p Version="Enter application version (1 - 10000): "
echo Application version %Version%

echo Create DFU File
nrfutil pkg generate --hw-version 52 --sd-req 0xB8 --application-version %Version% --application %App% --key-file private.key %ZipPackage%_v%Version%.zip

echo Finish
pause