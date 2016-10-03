echo off

SET SANDBOX=%CD%

SET BUILD_PATH=build

IF NOT EXIST %BUILD_PATH% mkdir %BUILD_PATH%

cd %BUILD_PATH%

cmake -G"Visual Studio 14 2015 Win64" %SANDBOX%

cd %SANDBOX%