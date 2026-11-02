#include "ConversationManager.hpp"
#include <iostream>

namespace aibuddy {

ConversationManager::ConversationManager(std::shared_ptr<StorageManager> storage)
    : m_storage(std::move(storage)) {}

std::string ConversationManager::createSession(const std::string& title, const std::string& type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string sessionId = StorageManager::generateId("sess_");
    
    HistoryItem item;
    item.id = sessionId;
    item.title = title;
    item.type = type;
    item.timestamp = StorageManager::getCurrentIsoTimestamp();

    m_activeSessions[sessionId] = item;
    return sessionId;
}

void ConversationManager::appendMessage(const std::string& sessionId, const Message& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_activeSessions.find(sessionId);
    if (it != m_activeSessions.end()) {
        it->second.messages.push_back(msg);
        it->second.timestamp = msg.timestamp;
        if (msg.sender == "user" && (it->second.title == "New Conversation" || it->second.title.empty())) {
            it->second.title = msg.content.substr(0, 48);
            if (msg.content.length() > 48) it->second.title += "...";
        }
        m_storage->addHistoryItem(it->second);
    } else {
        // Create new session if not present
        HistoryItem newItem;
        newItem.id = sessionId;
        newItem.title = msg.content.substr(0, 48);
        if (msg.content.length() > 48) newItem.title += "...";
        newItem.type = msg.type;
        newItem.timestamp = msg.timestamp;
        newItem.messages.push_back(msg);
        m_activeSessions[sessionId] = newItem;
        m_storage->addHistoryItem(newItem);
    }
}

std::vector<Message> ConversationManager::getSessionMessages(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_activeSessions.find(sessionId);
    if (it != m_activeSessions.end()) {
        return it->second.messages;
    }
    // Check storage
    auto history = m_storage->getHistory();
    for (const auto& item : history) {
        if (item.id == sessionId) {
            return item.messages;
        }
    }
    return {};
}

std::string ConversationManager::getSessionContext(const std::string& sessionId, int maxTurns) {
    auto messages = getSessionMessages(sessionId);
    if (messages.empty()) return "";

    std::string context;
    int startIdx = std::max(0, static_cast<int>(messages.size()) - (maxTurns * 2));
    for (size_t i = startIdx; i < messages.size(); ++i) {
        context += (messages[i].sender == "user" ? "User: " : "Assistant: ") + messages[i].content + "\n";
    }
    return context;
}

UserProfile ConversationManager::getUserProfile() const {
    return m_profile;
}

void ConversationManager::setUserProfile(const UserProfile& profile) {
    m_profile = profile;
}

std::vector<HistoryItem> ConversationManager::getRecentHistory() {
    return m_storage->getHistory();
}

bool ConversationManager::deleteHistoryItem(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeSessions.erase(id);
    }
    return m_storage->deleteHistoryItem(id);
}

void ConversationManager::clearAllHistory() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeSessions.clear();
    }
    m_storage->clear();
}

} // namespace aibuddy

