@echo off

set /p Path="Please enter the path to your library installation:"

mklink /J %cd%\Include %Path%\include
mklink /J %cd%\Win64 %Path%\bin\Debug

pause