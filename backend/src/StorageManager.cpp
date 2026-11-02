#include "StorageManager.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

namespace aibuddy {

void to_json(nlohmann::json& j, const Message& m) {
    j = nlohmann::json{
        {"id", m.id},
        {"sender", m.sender},
        {"content", m.content},
        {"timestamp", m.timestamp},
        {"type", m.type}
    };
}

void from_json(const nlohmann::json& j, Message& m) {
    m.id = j.value("id", "");
    m.sender = j.value("sender", "user");
    m.content = j.value("content", "");
    m.timestamp = j.value("timestamp", "");
    m.type = j.value("type", "text");
}

void to_json(nlohmann::json& j, const HistoryItem& h) {
    j = nlohmann::json{
        {"id", h.id},
        {"title", h.title},
        {"type", h.type},
        {"timestamp", h.timestamp},
        {"messages", h.messages}
    };
}

void from_json(const nlohmann::json& j, HistoryItem& h) {
    h.id = j.value("id", "");
    h.title = j.value("title", "");
    h.type = j.value("type", "chat");
    h.timestamp = j.value("timestamp", "");
    if (j.contains("messages") && j["messages"].is_array()) {
        h.messages = j["messages"].get<std::vector<Message>>();
    }
}

StorageManager::StorageManager(std::string filePath)
    : m_filePath(std::move(filePath)) {
    if (!load()) {
        populateDefaultHistory();
        save();
    }
}

bool StorageManager::load() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        if (j.is_array()) {
            m_items = j.get<std::vector<HistoryItem>>();
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "[StorageManager] Error reading JSON: " << e.what() << std::endl;
    }
    return false;
}

bool StorageManager::save() {
    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        nlohmann::json j = m_items;
        std::ofstream file(m_filePath);
        if (!file.is_open()) {
            std::cerr << "[StorageManager] Failed to open " << m_filePath << " for writing" << std::endl;
            return false;
        }
        file << j.dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[StorageManager] Error saving JSON: " << e.what() << std::endl;
        return false;
    }
}

std::vector<HistoryItem> StorageManager::getHistory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items;
}

void StorageManager::addHistoryItem(const HistoryItem& item) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_items.insert(m_items.begin(), item);
        // Keep up to 50 historical entries
        if (m_items.size() > 50) {
            m_items.resize(50);
        }
    }
    save();
}

bool StorageManager::deleteHistoryItem(const std::string& id) {
    bool removed = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_items.begin(); it != m_items.end(); ++it) {
            if (it->id == id) {
                m_items.erase(it);
                removed = true;
                break;
            }
        }
    }
    if (removed) {
        save();
    }
    return removed;
}

void StorageManager::clear() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_items.clear();
    }
    save();
}

void StorageManager::populateDefaultHistory() {
    // Matches the history shown in UI screenshot and mockups
    HistoryItem item1;
    item1.id = "hist_1";
    item1.title = "Voice Assistant Session";
    item1.type = "voice";
    item1.timestamp = getCurrentIsoTimestamp();
    item1.messages = {
        {"msg_v1_1", "user", "Hello AI Buddy", item1.timestamp, "voice"},
        {"msg_v1_2", "bot", "Hello! I am your AI Buddy voice assistant. How can I help you design, code, or brainstorm today?", item1.timestamp, "voice"}
    };

    HistoryItem item2;
    item2.id = "hist_2";
    item2.title = "I need some UI inspiration for dark ...";
    item2.type = "chat";
    item2.timestamp = getCurrentIsoTimestamp();
    item2.messages = {
        {"msg_c1_1", "user", "I need some UI inspiration for dark mobile dashboard with lime accents and fluid orbs", item2.timestamp, "text"},
        {"msg_c1_2", "bot", "Consider deep charcoal slate (#08080A) with vivid chartreuse (#D8FE43), subtle 1px border glassmorphism, iridescent gradient blobs, and oversized typography with rounded pills.", item2.timestamp, "text"}
    };

    HistoryItem item3;
    item3.id = "hist_3";
    item3.title = "Design review of modern dashboard mockups";
    item3.type = "image";
    item3.timestamp = getCurrentIsoTimestamp();
    item3.messages = {
        {"msg_i1_1", "user", "Can you review this mobile dashboard UI layout and check visual hierarchy?", item3.timestamp, "image"},
        {"msg_i1_2", "bot", "The hierarchy is strong: clear callout banner, 3-card asymmetric grid with high-contrast accent buttons, and clean recent history listing with icon badges.", item3.timestamp, "image"}
    };

    m_items = {item1, item2, item3};
}

std::string StorageManager::getCurrentIsoTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string StorageManager::generateId(const std::string& prefix) {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist(100000, 999999);
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    return prefix + std::to_string(now) + "_" + std::to_string(dist(rng));
}

} // namespace aibuddy

