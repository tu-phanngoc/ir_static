ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE
ECHO nrfjprog -f NRF52 --recover
nrfjprog -f NRF52 --recover
ECHO nrfjprog -f NRF52 --program s112_nrf52_6.1.1_softdevice.hex
nrfjprog -f NRF52 --program s112_nrf52_6.1.1_softdevice.hex
ECHO nrfjprog -f NRF52 --program nrf52811_bootloader.hex
nrfjprog -f NRF52 --program nrf52811_bootloader.hex
ECHO nrfjprog -f NRF52 --program APP.hex
ECHO nrfjprog -f NRF52 --program vt_sensor.hex
ECHO nrfjprog -f NRF52 --program bootloader_setting.hex
nrfjprog -f NRF52 --program bootloader_setting.hex
ECHO nrfjprog -f NRF52 -r
nrfjprog -f NRF52 -r
PAUSE