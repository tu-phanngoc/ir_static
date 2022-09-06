echo Start

set AppSource=..\_build\vt_sensor.hex
set App=vt_sensor.hex
set SoftDevice=s112_nrf52_6.1.1_softdevice.hex
set SoftDeviceVersion=0xB8
set BootLoader=nrf52811_bootloader.hex
set ZipPackage=VT_SENSOR

copy %AppSource% %App%

echo Create firmware
mergehex --merge vt_sensor.hex s112_nrf52_6.1.1_softdevice.hex --output CF_FW.hex

echo Flash Firmware
nrfjprog -f NRF52 -e
nrfjprog -f NRF52 --program CF_FW.hex --reset
echo nrfjprog -f NRF52 --rbp ALL


echo Finish
pause