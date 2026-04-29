@echo off

mkdir bin\obj

set ARGS=

call build_one.bat AsmWriter
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Implementation
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Main
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Operand
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Operator
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Program
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat RegisterOccupation
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat StackTrace
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat StringCoding
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Tokens
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Architecture architecture
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat x86_64 architecture
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Assembler assembler
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat NASM assembler
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Environment environment
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Win64 environment
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat CharGrouper parsing
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Parser parsing
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat Wordizer parsing
if "%ERRORLEVEL%" == "106" exit /B 1


g++ -std=c++20 %ARGS% -o bin/cpasm.exe

