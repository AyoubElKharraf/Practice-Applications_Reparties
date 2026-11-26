@echo off
set SCRIPT_DIR=%~dp0
echo Compilation du fichier .proto en classes Java et stubs gRPC...
set REPO=%USERPROFILE%\.m2\repository

"%REPO%\com\google\protobuf\protoc\3.25.5\protoc-3.25.5-windows-x86_64.exe" ^
  --proto_path="%SCRIPT_DIR%src\main\proto" ^
  --java_out="%SCRIPT_DIR%target\generated-sources\protobuf\java" ^
  --plugin=protoc-gen-grpc-java="%REPO%\io\grpc\protoc-gen-grpc-java\1.68.1\protoc-gen-grpc-java-1.68.1-windows-x86_64.exe" ^
  --grpc-java_out="%SCRIPT_DIR%target\generated-sources\protobuf\grpc-java" ^
  "%SCRIPT_DIR%src\main\proto\calcule.proto"

echo Code gRPC genere avec succes dans target\generated-sources\protobuf !
