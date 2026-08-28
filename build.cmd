@echo off

REM Cargar el entorno de Visual Studio 2022 (buscando en Program Files y Program Files (x86))
IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
) ELSE (
    echo [ERROR] No se pudo encontrar VsDevCmd.bat. Asegurate de tener Visual Studio 2022 Community instalado.
    pause
    exit /b 1
)

:Compile
REM Generar los archivos de proyecto para Visual Studio 2022
premake5.exe vs2022

set "solutionFile=./build/GoFlyKiller.sln"

if not exist "%solutionFile%" (
    echo [ERROR] No se encontro el archivo de solucion en %solutionFile%
    pause
    exit /b 1
)

REM Compilar la solucion
msbuild /t:Build /p:Configuration=Release -m "%solutionFile%"

:Done
echo Proceso finalizado.
pause