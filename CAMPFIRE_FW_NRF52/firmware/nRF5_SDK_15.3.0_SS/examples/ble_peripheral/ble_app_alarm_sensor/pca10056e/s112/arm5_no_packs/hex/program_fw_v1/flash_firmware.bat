echo Flash Firmware
nrfjprog -f NRF52 -e
nrfjprog -f NRF52 --program CF_FW_V1.0.11.hex --reset
echo nrfjprog -f NRF52 --rbp ALL
echo Finish
pause