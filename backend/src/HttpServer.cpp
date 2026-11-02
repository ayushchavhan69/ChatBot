#include "HttpServer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace aibuddy {

static std::string safeGetString(const nlohmann::json &j,
                                 const std::string &key,
                                 const std::string &defaultVal = "") {
  if (j.contains(key) && !j[key].is_null() && j[key].is_string()) {
    return j[key].get<std::string>();
  }
  return defaultVal;
}

HttpServer::HttpServer(std::shared_ptr<ChatBot> bot,
                       const std::string &staticDir)
    : m_bot(std::move(bot)), m_staticDir(staticDir) {
  registerRoutes();
}

void HttpServer::setCorsHeaders(httplib::Response &res) {
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods",
                 "GET, POST, PUT, DELETE, OPTIONS");
  res.set_header("Access-Control-Allow-Headers",
                 "Content-Type, Authorization, X-Requested-With");
  res.set_header("Access-Control-Max-Age", "86400");
}

void HttpServer::registerRoutes() {
  // Global Pre-Routing Handler for CORS and OPTIONS Preflight
  m_server.set_pre_routing_handler(
      [this](const httplib::Request &req, httplib::Response &res) {
        setCorsHeaders(res);
        if (req.method == "OPTIONS") {
          res.status = 204;
          return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
      });

  // Health & System Status
  m_server.Get(
      "/api/status", [this](const httplib::Request &, httplib::Response &res) {
        setCorsHeaders(res);
        nlohmann::json status = m_bot->getStatus();
        res.set_content(status.dump(2), "application/json; charset=utf-8");
      });

  // User Profile
  m_server.Get(
      "/api/user", [this](const httplib::Request &, httplib::Response &res) {
        setCorsHeaders(res);
        nlohmann::json profile = m_bot->getUserProfile();
        res.set_content(profile.dump(2), "application/json; charset=utf-8");
      });

  // Text Chat Endpoint
  m_server.Post(
      "/api/chat", [this](const httplib::Request &req, httplib::Response &res) {
        setCorsHeaders(res);
        try {
          nlohmann::json body = nlohmann::json::parse(req.body);
          std::string message = safeGetString(body, "message");
          std::string sessionId = safeGetString(body, "sessionId");
          std::string mode = safeGetString(body, "mode", "chat");

          if (message.empty()) {
            nlohmann::json err = {
                {"status", "error"},
                {"message", "Empty 'message' parameter in request"}};
            res.status = 400;
            res.set_content(err.dump(2), "application/json; charset=utf-8");
            return;
          }

          nlohmann::json result = m_bot->processChat(message, sessionId, mode);
          res.set_content(result.dump(2), "application/json; charset=utf-8");
        } catch (const std::exception &e) {
          std::cerr << "[HttpServer] Chat exception: " << e.what() << std::endl;
          nlohmann::json err = {{"status", "error"}, {"message", e.what()}};
          res.status = 400;
          res.set_content(err.dump(2), "application/json; charset=utf-8");
        }
      });

  // Voice Transcription & Response Endpoint
  m_server.Post("/api/voice/transcribe", [this](const httplib::Request &req,
                                                httplib::Response &res) {
    setCorsHeaders(res);
    try {
      nlohmann::json body = nlohmann::json::parse(req.body);
      std::string query = safeGetString(body, "query");
      std::string audioBase64 = safeGetString(body, "audioBase64");
      std::string sessionId = safeGetString(body, "sessionId");

      nlohmann::json result =
          m_bot->processVoice(query, audioBase64, sessionId);
      res.set_content(result.dump(2), "application/json; charset=utf-8");
    } catch (const std::exception &e) {
      std::cerr << "[HttpServer] Voice exception: " << e.what() << std::endl;
      nlohmann::json err = {{"status", "error"}, {"message", e.what()}};
      res.status = 400;
      res.set_content(err.dump(2), "application/json; charset=utf-8");
    }
  });

  // Multimodal Image Analysis Endpoint
  m_server.Post("/api/image/analyze", [this](const httplib::Request &req,
                                             httplib::Response &res) {
    setCorsHeaders(res);
    try {
      nlohmann::json body = nlohmann::json::parse(req.body);
      std::string prompt = safeGetString(body, "prompt");
      std::string imageBase64 = safeGetString(body, "image");
      std::string sessionId = safeGetString(body, "sessionId");

      nlohmann::json result =
          m_bot->processImageAnalysis(prompt, imageBase64, sessionId);
      res.set_content(result.dump(2), "application/json; charset=utf-8");
    } catch (const std::exception &e) {
      std::cerr << "[HttpServer] Image exception: " << e.what() << std::endl;
      nlohmann::json err = {{"status", "error"}, {"message", e.what()}};
      res.status = 400;
      res.set_content(err.dump(2), "application/json; charset=utf-8");
    }
  });

  // History Retrieval
  m_server.Get(
      "/api/history", [this](const httplib::Request &, httplib::Response &res) {
        setCorsHeaders(res);
        nlohmann::json history = m_bot->getHistory();
        res.set_content(history.dump(2), "application/json; charset=utf-8");
      });

  // Clear History
  m_server.Delete(
      "/api/history", [this](const httplib::Request &, httplib::Response &res) {
        setCorsHeaders(res);
        m_bot->clearHistory();
        nlohmann::json ok = {{"status", "success"},
                             {"message", "History cleared successfully"}};
        res.set_content(ok.dump(2), "application/json; charset=utf-8");
      });

  m_server.Post("/api/history/clear", [this](const httplib::Request &,
                                             httplib::Response &res) {
    setCorsHeaders(res);
    m_bot->clearHistory();
    nlohmann::json ok = {{"status", "success"},
                         {"message", "History cleared successfully"}};
    res.set_content(ok.dump(2), "application/json; charset=utf-8");
  });

  // Static UI File Hosting
  m_server.set_mount_point("/", m_staticDir);
  m_server.Get("/", [this](const httplib::Request &, httplib::Response &res) {
    std::string indexPath = m_staticDir + "/code.html";
    std::ifstream file(indexPath, std::ios::binary);
    if (file.is_open()) {
      std::stringstream buffer;
      buffer << file.rdbuf();
      res.set_content(buffer.str(), "text/html; charset=utf-8");
    } else {
      res.status = 404;
      res.set_content(
          "<h1>404 Not Found</h1><p>code.html missing in static directory</p>",
          "text/html");
    }
  });
}

bool HttpServer::start(const std::string &host, int port) {
  std::cout << "========================================================\n";
  std::cout << "  🤖 AI Buddy C++ Backend Server Starting...\n";
  std::cout << "  🚀 Listening on http://"
            << (host == "0.0.0.0" ? "localhost" : host) << ":" << port << "\n";
  std::cout << "  📁 Serving UI from: " << m_staticDir << "\n";
  std::cout << "  ⚡ Endpoints: \n";
  std::cout << "     - GET  /api/status\n";
  std::cout << "     - GET  /api/user\n";
  std::cout << "     - POST /api/chat\n";
  std::cout << "     - POST /api/voice/transcribe\n";
  std::cout << "     - POST /api/image/analyze\n";
  std::cout << "     - GET  /api/history\n";
  std::cout << "     - POST /api/history/clear\n";
  std::cout << "========================================================\n"
            << std::endl;

  return m_server.listen(host, port);
}

void HttpServer::stop() { m_server.stop(); }

} // namespace aibuddy
