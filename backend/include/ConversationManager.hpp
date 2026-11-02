#pragma once

#include "StorageManager.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace aibuddy {

struct UserProfile {
    std::string name = "Michael";
    std::string avatarUrl = "https://lh3.googleusercontent.com/aida-public/AB6AXuAhCBStJnFK_pUqcPtgsGQAKbA4BpoX-M41ekJFL3fkkOF9L1JXx3mAp-yp6iubw6VpH3z9yhcN8Z2CKscOYteie1ghxlkbnuSVVTLRYdGoaWwDhWUu4_9kOwSSMb-xpVcLPzXJ0FzianqSLB2lOiarfile5rmn64BXvJPOYh0AcAtPdHDCEsLLA6a106THrHJ-yO_w9BjCuhihy8FEKzLT9KEM0dN_Il7SA91JBSA";
    std::string role = "Product Designer";
    std::string preferredMode = "voice";
    std::string status = "Online";
};

class ConversationManager {
public:
    explicit ConversationManager(std::shared_ptr<StorageManager> storage);

    std::string createSession(const std::string& title = "New Conversation", const std::string& type = "chat");
    void appendMessage(const std::string& sessionId, const Message& msg);
    std::vector<Message> getSessionMessages(const std::string& sessionId);
    std::string getSessionContext(const std::string& sessionId, int maxTurns = 5);

    UserProfile getUserProfile() const;
    void setUserProfile(const UserProfile& profile);

    std::vector<HistoryItem> getRecentHistory();
    bool deleteHistoryItem(const std::string& id);
    void clearAllHistory();

private:
    std::shared_ptr<StorageManager> m_storage;
    UserProfile m_profile;
    std::unordered_map<std::string, HistoryItem> m_activeSessions;
    std::mutex m_mutex;
};

} // namespace aibuddy

