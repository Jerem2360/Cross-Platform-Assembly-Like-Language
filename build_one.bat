
if "%2" == "" (
    set ARGS=%ARGS% bin/obj/%1.obj
    echo src/%1.cpp to bin/obj/%1.obj...
    g++ -std=c++20 -c src/%1.cpp -o bin/obj/%1.obj
) else (
    set ARGS=%ARGS% bin/obj/%2-%1.obj
    echo src/%2/%1.cpp to bin/obj/%2-%1.obj...
    g++ -std=c++20 -c src/%2/%1.cpp -o bin/obj/%2-%1.obj
)

if %ERRORLEVEL% GEQ 1 (
    exit /B 106
)

