@echo off
:: converts .pgm images to .pngs

set /A INDEX=0

for /r %%i in (".\images\*.pgm") do call :convert %%i

goto :eof

:convert
set IMG=%~f1
set TARGET=%~dp1\%~n1.png

set /A INDEX=INDEX+1
echo %INDEX%...

ffmpeg -y -i "%IMG%" -c:v png -compression_level 100 -pix_fmt gray -f image2 "%TARGET%" 2> NUL
del "%IMG%" > NUL 2> NUL
