@echo off
set BEGINNING=--smac --test %1 --instance_info %2 --cutoff_time %3 --cutoff_length %4 --seed %5
for /f "tokens=5,* delims= " %%a in ("%*") do set REST=%%b
..\x64\release\keyboardlayout.exe %BEGINNING% %REST%
