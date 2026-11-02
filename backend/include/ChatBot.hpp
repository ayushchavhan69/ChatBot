#pragma once

#include "AIModel.hpp"
#include "ConversationManager.hpp"
#include "StorageManager.hpp"
#include "json.hpp"
#include <memory>
#include <string>
#include <chrono>

namespace aibuddy {

class ChatBot {
public:
    ChatBot(std::shared_ptr<IAIModel> aiModel,
            std::shared_ptr<ConversationManager> conversationManager,
            std::shared_ptr<StorageManager> storageManager);

    nlohmann::json processChat(const std::string& message, const std::string& sessionId = "", const std::string& mode = "chat");
    nlohmann::json processVoice(const std::string& transcript, const std::string& audioBase64 = "", const std::string& sessionId = "");
    nlohmann::json processImageAnalysis(const std::string& prompt, const std::string& imageBase64, const std::string& sessionId = "");

    nlohmann::json getStatus() const;
    nlohmann::json getHistory() const;
    nlohmann::json getUserProfile() const;
    bool clearHistory();

    std::shared_ptr<IAIModel> getModel() const { return m_aiModel; }
    void setModel(std::shared_ptr<IAIModel> model) { m_aiModel = model; }

private:
    std::shared_ptr<IAIModel> m_aiModel;
    std::shared_ptr<ConversationManager> m_conversationManager;
    std::shared_ptr<StorageManager> m_storageManager;
    std::chrono::system_clock::time_point m_startTime;
};

} // namespace aibuddy

