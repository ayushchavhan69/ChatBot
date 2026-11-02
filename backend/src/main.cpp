#include "HttpServer.hpp"
#include "ChatBot.hpp"
#include "AIModel.hpp"
#include "ConversationManager.hpp"
#include "StorageManager.hpp"
#include "SecurityConfig.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

static std::shared_ptr<aibuddy::HttpServer> g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[AI Buddy] Received interrupt signal (" << signum << "). Stopping server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    // Register signal handlers for graceful shutdown (Ctrl+C)
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting AI Buddy C++ Backend..." << std::endl;

    // Securely retrieve API Key from environment, .env file, or obfuscated secret
    std::string apiKey = aibuddy::SecurityConfig::getGeminiApiKey();

    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    std::string staticDir = "./stitch_ui_ux_design_replica";
    if (argc > 2) {
        staticDir = argv[2];
    }

    // 1. Storage Manager (persists to backend/data/history.json)
    auto storage = std::make_shared<aibuddy::StorageManager>("backend/data/history.json");

    // 2. Conversation Manager
    auto convMgr = std::make_shared<aibuddy::ConversationManager>(storage);

    // 3. AI Model Engine (Gemini Cloud LLM with Native C++ fallback)
    std::shared_ptr<aibuddy::IAIModel> aiModel;
    if (!apiKey.empty()) {
        std::cout << "[AI Buddy] Initialized with Google Gemini Multimodal Cloud AI (Secured Key: "
                  << aibuddy::SecurityConfig::getMaskedKey(apiKey) << ")" << std::endl;
        aiModel = std::make_shared<aibuddy::CloudAIAdapter>(apiKey, "gemini");
    } else {
        std::cout << "[AI Buddy] Initialized with Native C++ Domain Intelligence Engine" << std::endl;
        aiModel = std::make_shared<aibuddy::SmartNativeEngine>();
    }

    // 4. ChatBot Controller
    auto bot = std::make_shared<aibuddy::ChatBot>(aiModel, convMgr, storage);

    // 5. HTTP Server
    g_server = std::make_shared<aibuddy::HttpServer>(bot, staticDir);

    // Start server (blocking)
    if (!g_server->start("0.0.0.0", port)) {
        std::cerr << "[AI Buddy] Failed to bind to port " << port << "!" << std::endl;
        return 1;
    }

    std::cout << "[AI Buddy] Server exited cleanly." << std::endl;
    return 0;
}

