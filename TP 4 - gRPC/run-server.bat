@echo off
set SCRIPT_DIR=%~dp0
echo Demarrage du serveur gRPC...
java -cp "%SCRIPT_DIR%target\classes;%SCRIPT_DIR%target\lib\*" ma.ump.grpc.CalculServer
