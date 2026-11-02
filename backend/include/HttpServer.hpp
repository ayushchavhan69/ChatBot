#pragma once

#include "ChatBot.hpp"
#include "httplib.h"
#include <memory>
#include <string>

namespace aibuddy {

class HttpServer {
public:
    HttpServer(std::shared_ptr<ChatBot> bot, const std::string& staticDir = "./stitch_ui_ux_design_replica");

    bool start(const std::string& host = "0.0.0.0", int port = 8080);
    void stop();

private:
    std::shared_ptr<ChatBot> m_bot;
    std::string m_staticDir;
    httplib::Server m_server;

    void registerRoutes();
    void setCorsHeaders(httplib::Response& res);
};

} // namespace aibuddy

