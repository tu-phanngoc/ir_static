set SoftDevice=s132_nrf52_6.1.1_softdevice.hex
set BL=nrf52832_xxaa_s132_benkon_dfu.hex
ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE
ECHO nrfjprog -f NRF52 --recover
nrfjprog -f NRF52 --recover
ECHO nrfjprog -f NRF52 --program %SoftDevice% --chiperase
nrfjprog -f NRF52 --program %SoftDevice% --chiperase
ECHO nrfjprog -f NRF52 --program %BL%
nrfjprog -f NRF52 --program %BL%
ECHO nrfjprog -f NRF52 --program bootloader_setting.hex
nrfjprog -f NRF52 --program bootloader_setting.hex
ECHO nrfjprog -f NRF52 --program APP.hex
nrfjprog -f NRF52 --program APP.hex
ECHO nrfjprog -f NRF52 -r
nrfjprog -f NRF52 -r
PAUSE