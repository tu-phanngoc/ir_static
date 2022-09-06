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

e

echo Gen bootloader_setting
nrfutil settings generate --family NRF52810 --application %App% --application-version 0 --bootloader-version 0 --bl-settings-version 2 bootloader_setting.hex
