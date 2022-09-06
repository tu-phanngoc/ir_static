set SoftDeviceVersion=0xB7
set AppVersion=2
set AppInput=APP.hex
set Output=APP_OTA_V%AppVersion%.zip
ECHO OFF 
ECHO Nhan phim bat ky de tiep tuc
PAUSE
ECHO nrfutil pkg generate --hw-version 52 --sd-req %SoftDeviceVersion% --application-version %AppVersion% --application APP.hex --key-file private.pem %Output%
nrfutil pkg generate --hw-version 52 --sd-req %SoftDeviceVersion% --application-version %AppVersion% --application APP.hex --key-file private.pem %Output%

PAUSE