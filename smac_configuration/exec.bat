@echo off
set temp=%1
set instance=%temp:"=%
set BEGINNING=--smac --test %instance% --instance_info %2 --cutoff_time %3 --cutoff_length %4 --seed %5
for /f delims^=^"^ tokens^=2 %%a in ("%*") do set REST1=%%a
for /f "tokens=4,* delims= " %%a in ("%REST1%") do set REST=%%b
rem echo "Command line parameters"
rem echo %BEGINNING% %REST%
rem echo "Input"
rem echo %*
..\x64\release\keyboardlayout.exe %BEGINNING% %REST%
