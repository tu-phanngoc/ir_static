ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE

nrfutil settings generate --family NRF52810 --application vt_sensor.hex --application-version 1 --bootloader-version 0 --bl-settings-version 1 bootloader_setting.hex
PAUSE
