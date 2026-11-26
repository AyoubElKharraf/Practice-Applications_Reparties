@echo off
set SCRIPT_DIR=%~dp0
echo Demarrage du client gRPC...
java -cp "%SCRIPT_DIR%target\classes;%SCRIPT_DIR%target\lib\*" ma.ump.grpc.CalculClient
pause
