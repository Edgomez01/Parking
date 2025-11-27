@echo off
REM Recompilar cliente y servidor con timestamps

echo.
echo ================================================
echo   RECOMPILANDO CON TIMESTAMPS
echo ================================================
echo.

cd /d "%~dp0"

echo [1/2] Compilando servidor_multicliente.cpp...
cl /EHsc servidor_multicliente.cpp /Fe:servidor_multicliente.exe /link ws2_32.lib
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Fallo al compilar servidor
    pause
    exit /b 1
)
echo      ✓ servidor_multicliente.exe

echo.
echo [2/2] Compilando cliente.cpp...
cl cliente.cpp /Fe:cliente.exe /link ws2_32.lib
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Fallo al compilar cliente
    pause
    exit /b 1
)
echo      ✓ cliente.exe

echo.
echo ================================================
echo   ✓ COMPILACION COMPLETA
echo ================================================
echo.
echo AHORA el cliente envia:
echo   - Numero de plaza (1-40)
echo   - Placa del vehiculo (AAA000)
echo   - Timestamp (YYYY-MM-DD HH:MM:SS)
echo.
echo Ejecuta en orden:
echo   1. servidor_multicliente.exe
echo   2. python main.py
echo   3. cliente.exe
echo.
pause
