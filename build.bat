@echo off

mkdir bin\obj

set ARGS=

call build_one.bat tokenizer tokenizer
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat token_base tokenizer
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat number_token tokenizer
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat rules wordizer
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat wordizer wordizer
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat parser parser
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat operators
if "%ERRORLEVEL%" == "106" exit /B 1
call build_one.bat main
if "%ERRORLEVEL%" == "106" exit /B 1

::call build_one.bat AsmWriter
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Implementation
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Main
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Operand
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Operator
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Program
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat FuncCall
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat RegisterOccupation
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat StackTrace
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat StringCoding
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Tokens
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Architecture architecture
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat x86_64 architecture
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Assembler assembler
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat NASM assembler
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Environment environment
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Win64 environment
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Linux environment
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat CharGrouper parsing
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Parser parsing
::if "%ERRORLEVEL%" == "106" exit /B 1
::call build_one.bat Wordizer parsing
::if "%ERRORLEVEL%" == "106" exit /B 1


g++ -std=c++20 %ARGS% -o bin/cropall.exe

