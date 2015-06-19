@echo off
:: Usage:
:: build [mode] [test]
::   mode = "r" | ...
::     "r" = release
::     ... = debug
::   test = "test" | ...
::     "test" = perform the default test
::     ...    = do nothing


:: Include paths
pushd "..\src"
set INCLUDE_PATH=%CD%
popd


:: Build mode
if a"%1"==a"r" (
	echo Building release
	call :build_release demo || goto :eof
) else (
	echo Building debug
	call :build_debug demo-debug || goto :eof
)


:: Test
if a"%2"==a"test" (
	echo Testing
	call :test
)


goto :eof


:: Debug building
:build_debug
set EXE=%1
g++ -Wall -O0 -g -std=gnu++11 -I"%INCLUDE_PATH%" -o %EXE% demo.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Release building
:build_release
set EXE=%1
g++ -Wall -O3 -DNDEBUG=1 -DSIMPLEX_NOISE_FAST_MATH=1 -std=gnu++11 -I"%INCLUDE_PATH%" -o %EXE% demo.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Test image
:test
:: %EXE% images\demo.pgm binary centered 1024 1024 0.03125 0.6 4.0 3 4 0 0 0.5
%EXE% - binary centered 1024 1024 0.03125 0.6 4.0 3 4 0 0 0.5 | ffmpeg -y -f image2pipe -an -c:v pgm -i pipe:0 -c:v png -compression_level 100 -pix_fmt gray -f image2 images\demo.png 2> NUL

goto :eof

