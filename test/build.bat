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
	call :build_release test || goto :eof
) else (
	echo Building debug
	call :build_debug test-debug || goto :eof
)


:: Test
if a"%2"==a"test" (
	echo Testing
	call :test 1 0 0
	call :test 1 1 0

	call :test 2 0 0
	call :test 2 1 0
	call :test 2 2 0

	call :test 3 0 0
	call :test 3 1 0
	call :test 3 2 0
	call :test 3 3 0

	call :test 4 0 0
	call :test 4 1 0
	call :test 4 2 0
	call :test 4 3 0
	call :test 4 4 0

	call :test 1 0 5
	call :test 1 1 5

	call :test 2 0 5
	call :test 2 1 5

	call :test 3 0 5
	call :test 3 2 5

	call :test 4 0 5
	call :test 4 3 5
)


goto :eof


:: Debug building
:build_debug
set EXE=%1
g++ -Wall -O0 -g -std=c++11 -I"%INCLUDE_PATH%" -o %EXE% test.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Release building
:build_release
set EXE=%1
g++ -Wall -O3 -DNDEBUG=1 -DSIMPLEX_NOISE_FAST_MATH=1 -std=c++11 -I"%INCLUDE_PATH%" -o %EXE% test.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Test image
:test
set DIM=%1
set DER=%2
set OCT=%3
if %DER%==0 (
	set DER_NAME=
	set DER_NAME_FULL=
) else (
	set DER_NAME=-d%DER%
	set DER_NAME_FULL=, derivative %DER%
)
if %OCT%==0 (
	set OCT_NAME=
) else (
	set OCT_NAME=-o%OCT%
)
set FILENAME=test%DIM%d%OCT_NAME%%DER_NAME%.pgm

echo %DIM%D image, %OCT% octaves%DER_NAME_FULL%

%EXE% "images\%FILENAME%" 512 512 0.02 %DIM% %OCT% %DER%

goto :eof


