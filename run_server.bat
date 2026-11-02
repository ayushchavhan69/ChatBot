@echo off
setlocal

if not exist "server.exe" (
    echo [AI Buddy] server.exe not found. Building first...
    call build.bat
)

if exist "server.exe" (
    echo [AI Buddy] Starting C++ AI Buddy Server on http://localhost:8080 ...
    start "" "http://localhost:8080"
    server.exe 8080
) else (
    echo [AI Buddy] Error: server.exe could not be built.
    pause
)

endlocal

