@echo off
:: shortcut to build all executables

echo Building test...
pushd test
call build d
call build r
popd

echo Building demo...
pushd demo
call build d
call build r
popd

echo Building solver...
pushd solver
call build d
call build r
popd
