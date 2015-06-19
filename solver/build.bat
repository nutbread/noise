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
	call :build_release solver || goto :eof
) else (
	echo Building debug
	call :build_debug solver-debug || goto :eof
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
g++ -Wall -O0 -g -std=c++11 -I"%INCLUDE_PATH%" -o %EXE% solver.cpp Matrix.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Release building
:build_release
set EXE=%1
g++ -Wall -O3 -DNDEBUG=1 -ffast-math -std=c++11 -I"%INCLUDE_PATH%" -o %EXE% solver.cpp Matrix.cpp "%INCLUDE_PATH%\SimplexNoise.cpp" || exit /b 1

goto :eof


:: Test image
:test
%EXE% false 0 2 500 2000 > solve-2d.txt
:: solver false 0 2 500 2000 > solve-2d.txt
:: solver false 0 3 200 1000 > solve-3d.txt
:: solver false 0 4 100 200 > solve-4d.txt

goto :eof

