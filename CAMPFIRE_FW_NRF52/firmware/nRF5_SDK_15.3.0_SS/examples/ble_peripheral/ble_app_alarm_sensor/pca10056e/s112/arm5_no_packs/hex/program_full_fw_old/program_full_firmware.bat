echo Start

ECHO nrfjprog -f NRF52 --recover
nrfjprog -f NRF52 --recover
echo Erase full chip
nrfjprog -f NRF52 -e

echo Program firmware
nrfjprog -f NRF52 --program FW_FULL_TOSHIBA.hex --reset

ECHO nrfjprog -f NRF52 --rbp ALL
REM nrfjprog -f NRF52 --rbp ALL


echo Finish
pause