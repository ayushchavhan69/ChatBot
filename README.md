# 🤖 AI Buddy - C++ High-Performance Backend & UI

This project pairs an **AI Chatbot UI/UX** with a **native C++ HTTP/REST backend**. It provides real-time voice interaction, interactive text chat, multimodal UI critique, persistent JSON history, and sub-millisecond local responses.

---

## 📁 Project Architecture

```
opps chatbot v0.1/
├── backend/
│   ├── include/
│   │   ├── AIModel.hpp               # AI Engine interface & Native Domain Intelligence
│   │   ├── ChatBot.hpp               # Core chatbot controller & orchestrator
│   │   ├── ConversationManager.hpp   # Session tracking, context memory, user profile
│   │   ├── StorageManager.hpp        # Thread-safe JSON persistence
│   │   ├── HttpServer.hpp            # Multi-threaded HTTP/REST server & CORS handler
│   │   ├── httplib.h                 # High-performance C++ HTTP library
│   │   └── json.hpp                  # Modern C++ JSON library (nlohmann/json)
│   ├── src/
│   │   ├── AIModel.cpp               # Implementation of design, voice, code & chat logic
│   │   ├── ChatBot.cpp               # Message routing and timestamping
│   │   ├── ConversationManager.cpp   # Active session memory and context building
│   │   ├── StorageManager.cpp        # File I/O for backend/data/history.json
│   │   ├── HttpServer.cpp            # REST API endpoints & static UI hosting
│   │   └── main.cpp                  # Server entry point & signal handling
│   ├── data/
│   │   └── history.json              # Persistent conversation storage
│   ├── CMakeLists.txt                # CMake build definition
│   └── Makefile                      # GNU Make build definition
├── stitch_ui_ux_design_replica/
│   ├── code.html                     # Responsive UI (Dual screen: Voice & Dashboard)
│   ├── app.js                        # Client JS connecting UI to C++ backend
│   └── screen.png                    # Original UI/UX design reference
├── tools/
│   └── w64devkit/                    # Portable MinGW-w64 GCC 16.2.0 toolchain
├── server.exe                        # Precompiled C++ server binary (Ready to run!)
├── build.bat                         # 1-click build script
├── run_server.bat                    # 1-click launch script (starts server & opens browser)
└── README.md                         # Documentation
```

---

## 🚀 Quick Start (1-Click Run)

### Option 1: Double-Click `run_server.bat`
Simply double-click `run_server.bat`. It will:
1. Start the C++ backend on `http://localhost:8080`
2. Automatically launch your default browser to view the app!

### Option 2: Command Line
```powershell
.\server.exe 8080
```
Then open `http://localhost:8080` in your web browser.

---

## 🛠️ Recompiling the C++ Backend

If you modify any `.cpp` or `.hpp` file, recompile anytime using:

```cmd
build.bat
```

Or manually with the included GCC toolchain:
```cmd
tools\w64devkit\bin\g++.exe -std=c++17 -O2 -Ibackend/include backend/src/*.cpp -o server.exe -lws2_32 -lcrypt32
```

---

## 🌐 REST API Endpoints

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/` | Serves the interactive Web UI (`code.html`) |
| `GET` | `/api/status` | Returns backend health, uptime, active AI engine and capabilities |
| `GET` | `/api/user` | Returns user profile (`Michael`, Product Designer) |
| `POST` | `/api/chat` | Send a text query and receive an AI answer with prompt suggestions |
| `POST` | `/api/voice/transcribe` | Processes voice transcriptions and generates speech-optimized audio text |
| `POST` | `/api/image/analyze` | Multimodal layout and color critique of UI screenshots |
| `GET` | `/api/history` | Fetches saved sessions and past conversation records |
| `DELETE`| `/api/history` | Clears all stored chat and voice history |

---

## ✨ Features & Capabilities

1. **Active Voice Screen**:
   - Chromatic 3D morphing liquid orb with multi-layer glares.
   - Click the **Microphone Button** (or the orb) to speak.
   - Built-in speech synthesis (the AI speaks back aloud using browser Text-to-Speech!).
2. **Dashboard Screen**:
   - **Talk with Bot**: Smoothly scrolls and engages the active voice screen.
   - **Chat with Bot**: Opens an interactive modal with quick prompt chips, full markdown rendering, and typing indicators.
   - **Search by Image**: Triggers instant visual AI critique of UI layouts, contrast, and color balance.
   - **Dynamic History**: Real-time list synchronized with `backend/data/history.json`.
3. **Smart Native C++ AI Engine**:
   - Tuned for design inquiries (Figma, Framer, Penpot, dark-mode palettes, design tokens).
   - Instant response times (< 2ms) with zero external network dependencies.
   - Optional support for `GEMINI_API_KEY` environment variable to bridge cloud LLM completions.

