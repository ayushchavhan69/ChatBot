@echo off
setlocal

echo [AI Buddy] Building C++ Backend...

set GXX_PATH=g++
if exist "tools\w64devkit\bin\g++.exe" (
    set GXX_PATH=tools\w64devkit\bin\g++.exe
)

echo [AI Buddy] Using compiler: %GXX_PATH%

"%GXX_PATH%" -std=c++17 -O2 -Ibackend/include backend/src/StorageManager.cpp backend/src/AIModel.cpp backend/src/ConversationManager.cpp backend/src/ChatBot.cpp backend/src/HttpServer.cpp backend/src/main.cpp -o server.exe -lws2_32 -lcrypt32 -lwinhttp

if %ERRORLEVEL% EQU 0 (
    echo [AI Buddy] Build SUCCESS! Generated server.exe
) else (
    echo [AI Buddy] Build FAILED! Error code: %ERRORLEVEL%
)

endlocal

