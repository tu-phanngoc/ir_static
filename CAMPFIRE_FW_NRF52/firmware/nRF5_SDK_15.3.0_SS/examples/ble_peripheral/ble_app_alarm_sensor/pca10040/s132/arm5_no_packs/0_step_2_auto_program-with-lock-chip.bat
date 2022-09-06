ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE
ECHO nrfjprog -f NRF52 --recover
nrfjprog -f NRF52 --recover
ECHO nrfjprog -f NRF52 --program s132_nrf52_4.0.2_softdevice.hex --chiperase
nrfjprog -f NRF52 --program s132_nrf52_4.0.2_softdevice.hex --chiperase
ECHO nrfjprog -f NRF52 --program BL.hex
nrfjprog -f NRF52 --program BL.hex
ECHO nrfjprog -f NRF52 --program APP.hex
nrfjprog -f NRF52 --program APP.hex
ECHO nrfjprog -f NRF52 --program bootloader_setting.hex
nrfjprog -f NRF52 --program bootloader_setting.hex
ECHO nrfjprog -f NRF52 --rbp ALL
nrfjprog -f NRF52 --rbp ALL
ECHO nrfjprog -f NRF52 -r
nrfjprog -f NRF52 -r
PAUSE