#include "ChatBot.hpp"
#include <chrono>

namespace aibuddy {

ChatBot::ChatBot(std::shared_ptr<IAIModel> aiModel,
                 std::shared_ptr<ConversationManager> conversationManager,
                 std::shared_ptr<StorageManager> storageManager)
    : m_aiModel(std::move(aiModel)),
      m_conversationManager(std::move(conversationManager)),
      m_storageManager(std::move(storageManager)),
      m_startTime(std::chrono::system_clock::now()) {}

nlohmann::json ChatBot::processChat(const std::string& message, const std::string& sessionId, const std::string& mode) {
    std::string activeSession = sessionId.empty() ? m_conversationManager->createSession("Chat with Bot", "chat") : sessionId;
    std::string timestamp = StorageManager::getCurrentIsoTimestamp();

    // Record user message
    Message userMsg;
    userMsg.id = StorageManager::generateId("usr_");
    userMsg.sender = "user";
    userMsg.content = message;
    userMsg.timestamp = timestamp;
    userMsg.type = "text";
    m_conversationManager->appendMessage(activeSession, userMsg);

    // Retrieve previous context
    std::string context = m_conversationManager->getSessionContext(activeSession);

    // Generate AI response
    AIResponse aiResp = m_aiModel->generateTextResponse(message, context);

    // Record bot response
    Message botMsg;
    botMsg.id = StorageManager::generateId("bot_");
    botMsg.sender = "bot";
    botMsg.content = aiResp.text;
    botMsg.timestamp = StorageManager::getCurrentIsoTimestamp();
    botMsg.type = "text";
    m_conversationManager->appendMessage(activeSession, botMsg);

    nlohmann::json responseJson = {
        {"status", "success"},
        {"sessionId", activeSession},
        {"userMessage", userMsg},
        {"reply", aiResp.text},
        {"speechText", aiResp.speechText},
        {"intent", aiResp.intent},
        {"suggestions", aiResp.suggestions},
        {"highlights", aiResp.highlights},
        {"timestamp", botMsg.timestamp},
        {"model", m_aiModel->getModelName()}
    };
    return responseJson;
}

nlohmann::json ChatBot::processVoice(const std::string& transcript, const std::string& audioBase64, const std::string& sessionId) {
    std::string activeSession = sessionId.empty() ? m_conversationManager->createSession("Voice Query", "voice") : sessionId;
    std::string timestamp = StorageManager::getCurrentIsoTimestamp();

    Message userMsg;
    userMsg.id = StorageManager::generateId("usr_v_");
    userMsg.sender = "user";
    userMsg.content = transcript.empty() ? "Voice recording input" : transcript;
    userMsg.timestamp = timestamp;
    userMsg.type = "voice";
    m_conversationManager->appendMessage(activeSession, userMsg);

    AIResponse aiResp = m_aiModel->generateVoiceResponse(userMsg.content);

    Message botMsg;
    botMsg.id = StorageManager::generateId("bot_v_");
    botMsg.sender = "bot";
    botMsg.content = aiResp.text;
    botMsg.timestamp = StorageManager::getCurrentIsoTimestamp();
    botMsg.type = "voice";
    m_conversationManager->appendMessage(activeSession, botMsg);

    nlohmann::json responseJson = {
        {"status", "success"},
        {"sessionId", activeSession},
        {"transcript", userMsg.content},
        {"reply", aiResp.text},
        {"speechText", aiResp.speechText},
        {"intent", aiResp.intent},
        {"timestamp", botMsg.timestamp},
        {"model", m_aiModel->getModelName()}
    };
    return responseJson;
}

nlohmann::json ChatBot::processImageAnalysis(const std::string& prompt, const std::string& imageBase64, const std::string& sessionId) {
    std::string activeSession = sessionId.empty() ? m_conversationManager->createSession("Image Analysis", "image") : sessionId;
    std::string timestamp = StorageManager::getCurrentIsoTimestamp();

    Message userMsg;
    userMsg.id = StorageManager::generateId("usr_img_");
    userMsg.sender = "user";
    userMsg.content = prompt.empty() ? "Analyze this uploaded interface image." : prompt;
    userMsg.timestamp = timestamp;
    userMsg.type = "image";
    m_conversationManager->appendMessage(activeSession, userMsg);

    AIResponse aiResp = m_aiModel->analyzeImage(userMsg.content, imageBase64);

    Message botMsg;
    botMsg.id = StorageManager::generateId("bot_img_");
    botMsg.sender = "bot";
    botMsg.content = aiResp.text;
    botMsg.timestamp = StorageManager::getCurrentIsoTimestamp();
    botMsg.type = "image";
    m_conversationManager->appendMessage(activeSession, botMsg);

    nlohmann::json responseJson = {
        {"status", "success"},
        {"sessionId", activeSession},
        {"analysis", aiResp.text},
        {"speechText", aiResp.speechText},
        {"suggestions", aiResp.suggestions},
        {"highlights", aiResp.highlights},
        {"metadata", aiResp.metadata},
        {"timestamp", botMsg.timestamp},
        {"model", m_aiModel->getModelName()}
    };
    return responseJson;
}

nlohmann::json ChatBot::getStatus() const {
    auto now = std::chrono::system_clock::now();
    auto uptimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();

    return nlohmann::json{
        {"status", "online"},
        {"assistantName", "AI Buddy"},
        {"engine", "C++ Native High-Performance Core"},
        {"model", m_aiModel->getModelName()},
        {"uptimeSeconds", uptimeSeconds},
        {"timestamp", StorageManager::getCurrentIsoTimestamp()},
        {"capabilities", {
            "Voice Speech Recognition & Synthesis",
            "Real-time Collaborative Design Intelligence",
            "Multimodal Image & Layout Critique",
            "Persistent JSON Session Storage",
            "Cross-Platform High-Concurrency REST API"
        }}
    };
}

nlohmann::json ChatBot::getHistory() const {
    return m_conversationManager->getRecentHistory();
}

nlohmann::json ChatBot::getUserProfile() const {
    UserProfile p = m_conversationManager->getUserProfile();
    return nlohmann::json{
        {"name", p.name},
        {"role", p.role},
        {"status", p.status},
        {"avatarUrl", p.avatarUrl},
        {"preferredMode", p.preferredMode}
    };
}

bool ChatBot::clearHistory() {
    m_conversationManager->clearAllHistory();
    return true;
}

} // namespace aibuddy

