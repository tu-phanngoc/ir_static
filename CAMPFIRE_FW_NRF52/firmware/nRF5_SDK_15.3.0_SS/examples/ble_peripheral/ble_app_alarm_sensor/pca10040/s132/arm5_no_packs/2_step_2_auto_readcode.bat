ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE
ECHO nrfjprog -f NRF52 --readcode redcode.hex
nrfjprog -f NRF52 --readcode redcode.hex
ECHO nrfjprog -f NRF52 -r
nrfjprog -f NRF52 -r
PAUSE