#pragma once

#include "json.hpp"
#include <string>
#include <vector>
#include <mutex>

namespace aibuddy {

struct Message {
    std::string id;
    std::string sender; // "user" or "bot"
    std::string content;
    std::string timestamp;
    std::string type;   // "text", "voice", "image"
};

struct HistoryItem {
    std::string id;
    std::string title;
    std::string type;       // "voice", "chat", "image"
    std::string timestamp;
    std::vector<Message> messages;
};

void to_json(nlohmann::json& j, const Message& m);
void from_json(const nlohmann::json& j, Message& m);

void to_json(nlohmann::json& j, const HistoryItem& h);
void from_json(const nlohmann::json& j, HistoryItem& h);

class StorageManager {
public:
    explicit StorageManager(std::string filePath = "backend/data/history.json");

    bool load();
    bool save();

    std::vector<HistoryItem> getHistory();
    void addHistoryItem(const HistoryItem& item);
    bool deleteHistoryItem(const std::string& id);
    void clear();

    static std::string getCurrentIsoTimestamp();
    static std::string generateId(const std::string& prefix = "msg_");

private:
    std::string m_filePath;
    std::vector<HistoryItem> m_items;
    std::mutex m_mutex;

    void populateDefaultHistory();
};

} // namespace aibuddy

