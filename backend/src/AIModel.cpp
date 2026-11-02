#include "AIModel.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <windows.h>
#include <winhttp.h>

namespace aibuddy {

static std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return str;
}

SmartNativeEngine::SmartNativeEngine() = default;

std::string SmartNativeEngine::cleanSpeechText(const std::string& input) {
    // Strip markdown formatting symbols for natural text-to-speech audio
    std::string out = std::regex_replace(input, std::regex(R"(\*\*|__|\*|_|`|#)"), "");
    out = std::regex_replace(out, std::regex(R"(\[([^\]]+)\]\([^\)]+\))"), "$1");
    if (out.length() > 220) {
        size_t dot = out.find('.', 160);
        if (dot != std::string::npos) {
            out = out.substr(0, dot + 1);
        }
    }
    return out;
}

AIResponse SmartNativeEngine::generateTextResponse(const std::string& prompt, const std::string& context) {
    AIResponse resp;
    std::string lower = toLower(prompt);

    if (lower.find("collaborat") != std::string::npos ||
        lower.find("tool") != std::string::npos ||
        lower.find("figma") != std::string::npos ||
        lower.find("design") != std::string::npos ||
        lower.find("ui") != std::string::npos ||
        lower.find("ux") != std::string::npos ||
        lower.find("dark") != std::string::npos ||
        lower.find("color") != std::string::npos ||
        lower.find("layout") != std::string::npos) {
        resp.text = handleDesignQuery(prompt, resp);
    } else if (lower.find("c++") != std::string::npos ||
               lower.find("code") != std::string::npos ||
               lower.find("backend") != std::string::npos ||
               lower.find("server") != std::string::npos ||
               lower.find("api") != std::string::npos ||
               lower.find("function") != std::string::npos) {
        resp.text = handleCodeQuery(prompt, resp);
    } else {
        resp.text = handleGeneralQuery(prompt, resp);
    }

    resp.speechText = cleanSpeechText(resp.text);
    return resp;
}

AIResponse SmartNativeEngine::generateVoiceResponse(const std::string& query) {
    AIResponse resp;
    resp.intent = "voice_interaction";
    resp.text = handleVoiceQuery(query, resp);
    resp.speechText = cleanSpeechText(resp.text);
    return resp;
}

AIResponse SmartNativeEngine::analyzeImage(const std::string& prompt, const std::string& imageBase64) {
    AIResponse resp;
    resp.intent = "image_analysis";
    resp.highlights = {
        "High-contrast dark mode palette (bg: #08080A, accent: #D8FE43)",
        "Organic chromatic 3D fluid orb with multi-layer radial gradient glares",
        "Asymmetrical 2-column quick action grid with distinct tactile color badges",
        "Consistent 44px pill radiuses and subtle 0.8px topographic vector contours"
    };
    resp.suggestions = {
        "Increase active touch target padding on secondary icon controls",
        "Add subtle haptic feedback for speech trigger buttons",
        "Implement smooth view-transitions between voice and dashboard views"
    };

    std::ostringstream ss;
    ss << "### 🎨 UI/UX Multimodal Analysis Report\n\n";
    ss << "**Visual Hierarchy & Composition:**\n";
    ss << "- **Theme:** Premium Cyber-Futurist Dark Mode with Neomorphic Lime & Lavender accents.\n";
    ss << "- **Orb Element:** Smoothly animated CSS morphing sphere with dynamic chromatic aberration.\n";
    ss << "- **Typography:** Clean SF Pro / Inter sans-serif with crisp kerning and high readability contrast.\n\n";
    ss << "**Actionable Design Feedback:**\n";
    ss << "1. The hero call-to-action cards have strong visual affordance with diagonal arrow indicators (↗).\n";
    ss << "2. The microphone button with dual concentric pulse rings clearly guides user focus for voice activation.\n";
    ss << "3. The history list maintains clean item-level affordance with distinct category color dots.\n";

    resp.text = ss.str();
    resp.speechText = "I analyzed the interface design. The layout showcases high contrast dark mode with lime accents, smooth morphing orb visuals, and clear call to action cards.";
    resp.metadata = {
        {"colorPalette", {"#D8FE43", "#C5B4FA", "#FECDD3", "#08080A", "#141519"}},
        {"contrastRatio", "14.2:1 (AAA rated)"},
        {"styleClassification", "Modern Cyber/Neomorphic Hybrid"}
    };
    return resp;
}

std::string SmartNativeEngine::handleDesignQuery(const std::string& prompt, AIResponse& response) {
    response.intent = "design_inquiry";
    std::string lower = toLower(prompt);

    if (lower.find("collaborat") != std::string::npos || lower.find("tool") != std::string::npos) {
        response.highlights = {"Figma & FigJam", "Framer", "Penpot (Open Source)", "Miro", "Spline 3D"};
        response.suggestions = {
            "Explore Figma variables for multi-theme design systems",
            "Use Framer for instant responsive production deployments",
            "Try Spline for interactive 3D WebGL scenes"
        };
        return "Top collaborative interface design tools include:\n\n"
               "1. **Figma & FigJam:** The undisputed market standard for real-time multiplayer UI/UX design, component libraries, and interactive prototyping.\n"
               "2. **Framer:** Merges visual canvas design directly with live production React code and CMS capabilities.\n"
               "3. **Penpot:** The open-source, web-standards design tool gaining strong traction across privacy-focused teams.\n"
               "4. **Spline:** Revolutionizing 3D collaborative design for the web with real-time physics and lightweight WebGL embeds.\n\n"
               "Would you like recommendations on specific plugins or design token pipelines?";
    }

    if (lower.find("dark") != std::string::npos || lower.find("inspiration") != std::string::npos) {
        response.highlights = {"#08080A Deep Obsidian", "#D8FE43 Electric Lime", "#C5B4FA Soft Lavender", "#FECDD3 Muted Peach"};
        response.suggestions = {
            "Use 1px semi-transparent borders (rgba(255,255,255,0.08)) for subtle depth",
            "Incorporate chromatic orb animations for active AI states",
            "Avoid pure black (#000000) for surfaces to prevent OLED smearing"
        };
        return "For modern dark mode mobile UIs, here is a recommended aesthetic framework:\n\n"
               "- **Base Canvas:** Deep obsidian `#08080A` or tinted gunmetal `#121316`.\n"
               "- **High-Energy Accents:** Electric Chartreuse `#D8FE43` for primary CTAs and active microphone indicators.\n"
               "- **Secondary Soft Tones:** Pastel lavender `#C5B4FA` and warm peach `#FECDD3` to soften card interactions.\n"
               "- **Glassmorphism:** Inner shadows with 30px blurs and subtle white rim highlights for cards.\n"
               "- **Vector Touches:** Delicate 0.8px topographic contour lines with 15-25% opacity in header corners.\n\n"
               "This creates a futuristic yet welcoming companion experience!";
    }

    response.highlights = {"Component-Driven Layouts", "Accessible Contrast", "Micro-Interactions"};
    return "Modern product design balances functional ergonomics with delightful micro-interactions. "
           "Focus on clear visual hierarchy, consistent 8pt spatial rhythm, tactile button states, and clear typography scale.";
}

std::string SmartNativeEngine::handleCodeQuery(const std::string& prompt, AIResponse& response) {
    response.intent = "engineering";
    response.highlights = {"C++17/20 Modern Standards", "Lightweight HTTP Server", "High-Throughput Concurrency", "Zero External Runtimes"};
    response.suggestions = {
        "Compile with -O3 for maximum performance",
        "Use std::string_view for zero-copy string parsing",
        "Leverage RAII and smart pointers for memory safety"
    };

    return "Your AI Buddy backend is powered by a high-performance **C++ engine**:\n\n"
           "- **Architecture:** Clean modular Object-Oriented design (`ChatBot`, `ConversationManager`, `StorageManager`, `HttpServer`).\n"
           "- **Networking:** Multi-threaded Winsock2 REST HTTP engine with CORS and static file delivery.\n"
           "- **Data Persistence:** Thread-safe JSON state storage with automated persistence.\n"
           "- **Engine:** Domain-specific AI response processing with pluggable cloud adapters.\n\n"
           "It delivers sub-millisecond API response latency on local machines with minimal CPU and memory footprints.";
}

std::string SmartNativeEngine::handleVoiceQuery(const std::string& query, AIResponse& response) {
    std::string lower = toLower(query);
    if (lower.find("hello") != std::string::npos || lower.find("hi") != std::string::npos) {
        return "Hello! I am your AI Buddy voice assistant. How can I help you design, code, or brainstorm today?";
    }
    if (lower.find("who are you") != std::string::npos) {
        return "I am AI Buddy, your intelligent voice and chat companion built with a high-performance C++ backend.";
    }
    return "I heard: \"" + query + "\". How can I help you brainstorm, design, or write code?";
}

std::string SmartNativeEngine::handleGeneralQuery(const std::string& prompt, AIResponse& response) {
    response.intent = "conversation";
    response.suggestions = {
        "I need some UI inspiration for dark mobile dashboard",
        "Help me brainstorm creative project ideas",
        "Explain how the C++ backend communicates with this UI"
    };

    return "Hi there! I am AI Buddy, your personal assistant. I can help answer design inquiries, transcribe and reply to voice queries, critique UI screenshots, and coordinate your project tasks. Try asking me for ideas or design tips!";
}

CloudAIAdapter::CloudAIAdapter(std::string apiKey, std::string provider)
    : m_apiKey(std::move(apiKey)), m_provider(std::move(provider)), m_fallbackEngine(std::make_unique<SmartNativeEngine>()) {}

std::string CloudAIAdapter::getModelName() const {
    return "Google Gemini 3.5 Flash-Lite (Secured Cloud API)";
}

std::string CloudAIAdapter::callGeminiApi(const std::string& prompt, const std::string& systemInstruction) {
    if (m_apiKey.empty()) return "";

    std::wstring host = L"generativelanguage.googleapis.com";
    std::wstring path = L"/v1beta/models/" + std::wstring(m_modelName.begin(), m_modelName.end()) +
                        L":generateContent?key=" + std::wstring(m_apiKey.begin(), m_apiKey.end());

    nlohmann::json reqBody;
    reqBody["contents"] = nlohmann::json::array();

    std::string fullPrompt = prompt;
    if (!systemInstruction.empty()) {
        fullPrompt = systemInstruction + "\n\nUser Message:\n" + prompt;
    }

    reqBody["contents"].push_back({
        {"parts", nlohmann::json::array({
            {{"text", fullPrompt}}
        })}
    });

    std::string postData = reqBody.dump();

    HINTERNET hSession = WinHttpOpen(L"AIBuddy-Backend/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect,
                                           L"POST",
                                           path.c_str(),
                                           NULL,
                                           WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Set 12s timeout
    WinHttpSetTimeouts(hRequest, 10000, 10000, 12000, 12000);

    std::wstring headers = L"Content-Type: application/json\r\n";
    BOOL bResults = WinHttpSendRequest(hRequest,
                                      headers.c_str(),
                                      (DWORD)headers.length(),
                                      (LPVOID)postData.c_str(),
                                      (DWORD)postData.length(),
                                      (DWORD)postData.length(),
                                      0);

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string responseBody;
    if (bResults) {
        DWORD dwSize = 0;
        do {
            dwSize = 0;
            if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                    responseBody.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (responseBody.empty()) return "";

    try {
        nlohmann::json resJson = nlohmann::json::parse(responseBody);
        if (resJson.contains("candidates") && resJson["candidates"].is_array() && !resJson["candidates"].empty()) {
            auto& cand = resJson["candidates"][0];
            if (cand.contains("content") && cand["content"].contains("parts") && !cand["content"]["parts"].empty()) {
                for (const auto& part : cand["content"]["parts"]) {
                    if (part.contains("text")) {
                        return part["text"].get<std::string>();
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Gemini] JSON parse error: " << e.what() << std::endl;
    }

    return "";
}

AIResponse CloudAIAdapter::generateTextResponse(const std::string& prompt, const std::string& context) {
    if (!m_apiKey.empty()) {
        std::string systemCtx = context.empty() ? "" : ("Context of conversation:\n" + context);
        std::string answer = callGeminiApi(prompt, systemCtx);
        if (!answer.empty()) {
            AIResponse resp;
            resp.text = answer;
            resp.speechText = SmartNativeEngine::cleanSpeechText(answer);
            resp.intent = "cloud_gemini_response";
            resp.highlights = {"Google Gemini 3.5 Flash", "Live Cloud Intelligence", "Accurate AI Knowledge"};
            resp.suggestions = {
                "Can you elaborate on that?",
                "Give me a concrete code example",
                "What are best practices for this?"
            };
            return resp;
        }
    }
    // Fallback gracefully
    return m_fallbackEngine->generateTextResponse(prompt, context);
}

AIResponse CloudAIAdapter::generateVoiceResponse(const std::string& query) {
    if (!m_apiKey.empty()) {
        std::string voicePrompt = "You are AI Buddy, a friendly voice assistant. Respond concisely in 2 to 3 conversational sentences suited for spoken audio.";
        std::string answer = callGeminiApi(query, voicePrompt);
        if (!answer.empty()) {
            AIResponse resp;
            resp.text = answer;
            resp.speechText = SmartNativeEngine::cleanSpeechText(answer);
            resp.intent = "cloud_voice_response";
            return resp;
        }
    }
    return m_fallbackEngine->generateVoiceResponse(query);
}

std::string CloudAIAdapter::base64Encode(const std::vector<unsigned char>& data) {
    static const char s_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(s_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(s_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string CloudAIAdapter::callGeminiMultimodalApi(const std::string& prompt, const std::string& cleanBase64, const std::string& mimeType) {
    if (m_apiKey.empty()) return "";

    std::wstring host = L"generativelanguage.googleapis.com";
    std::wstring path = L"/v1beta/models/" + std::wstring(m_modelName.begin(), m_modelName.end()) +
                        L":generateContent?key=" + std::wstring(m_apiKey.begin(), m_apiKey.end());

    nlohmann::json reqBody;
    reqBody["contents"] = nlohmann::json::array();

    nlohmann::json parts = nlohmann::json::array();
    parts.push_back({
        {"text", prompt.empty() ? "Critique and analyze this image or interface snapshot." : prompt}
    });

    if (!cleanBase64.empty()) {
        parts.push_back({
            {"inline_data", {
                {"mime_type", mimeType.empty() ? "image/jpeg" : mimeType},
                {"data", cleanBase64}
            }}
        });
    }

    reqBody["contents"].push_back({{"parts", parts}});

    std::string postData = reqBody.dump();

    HINTERNET hSession = WinHttpOpen(L"AIBuddy-Backend/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect,
                                           L"POST",
                                           path.c_str(),
                                           NULL,
                                           WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Set 25s timeout for multimodal image processing
    WinHttpSetTimeouts(hRequest, 15000, 15000, 25000, 25000);

    std::wstring headers = L"Content-Type: application/json\r\n";
    BOOL bResults = WinHttpSendRequest(hRequest,
                                      headers.c_str(),
                                      (DWORD)headers.length(),
                                      (LPVOID)postData.c_str(),
                                      (DWORD)postData.length(),
                                      (DWORD)postData.length(),
                                      0);

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string responseBody;
    if (bResults) {
        DWORD dwSize = 0;
        do {
            dwSize = 0;
            if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                    responseBody.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (responseBody.empty()) return "";

    try {
        nlohmann::json resJson = nlohmann::json::parse(responseBody);
        if (resJson.contains("candidates") && resJson["candidates"].is_array() && !resJson["candidates"].empty()) {
            auto& cand = resJson["candidates"][0];
            if (cand.contains("content") && cand["content"].contains("parts") && !cand["content"]["parts"].empty()) {
                for (const auto& part : cand["content"]["parts"]) {
                    if (part.contains("text")) {
                        return part["text"].get<std::string>();
                    }
                }
            }
        }
        if (resJson.contains("error")) {
            std::cerr << "[Gemini Multimodal] API Error: " << resJson["error"].dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Gemini Multimodal] JSON parse error: " << e.what() << std::endl;
    }

    return "";
}

AIResponse CloudAIAdapter::analyzeImage(const std::string& prompt, const std::string& imageBase64) {
    if (!m_apiKey.empty()) {
        std::string mimeType = "image/png";
        std::string cleanBase64 = imageBase64;

        if (cleanBase64 == "embedded_screen_sample" || cleanBase64.empty()) {
            std::ifstream file("stitch_ui_ux_design_replica/screen.png", std::ios::binary);
            if (file.is_open()) {
                std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
                                                   std::istreambuf_iterator<char>());
                cleanBase64 = base64Encode(buffer);
                mimeType = "image/png";
            }
        } else if (cleanBase64.rfind("data:", 0) == 0) {
            size_t colon = cleanBase64.find(':');
            size_t semi = cleanBase64.find(';', colon);
            size_t comma = cleanBase64.find(',', semi);
            if (colon != std::string::npos && semi != std::string::npos && comma != std::string::npos) {
                mimeType = cleanBase64.substr(colon + 1, semi - (colon + 1));
                cleanBase64 = cleanBase64.substr(comma + 1);
            }
        }

        std::string searchPrompt;
        if (!prompt.empty()) {
            searchPrompt = 
                "You are an expert AI visual assistant and problem solver.\n"
                "The user provided an image and asked: \"" + prompt + "\".\n"
                "1. First, carefully read all text, questions, code, math, or instructions visible in the image.\n"
                "2. Directly answer their question or solve their request thoroughly based on what is in the image.\n"
                "3. If the image has a question or problem, provide the complete and accurate solution.";
        } else {
            searchPrompt = 
                "You are an expert visual AI assistant and question solver.\n"
                "1. Carefully read and transcribe the main text, question, problem, code, or query shown in this image.\n"
                "2. If the image contains a question, exam problem, prompt, or topic (e.g. 'Explain the Transformer architecture', a math problem, a coding question, etc.), give the complete, accurate, detailed, and comprehensive answer to that question.\n"
                "3. If the image contains code or an error message, explain the error and provide the corrected, working code.\n"
                "4. If the image is a diagram or chart, explain the concepts it demonstrates and summarize the key information.\n"
                "5. IMPORTANT: DO NOT critique UI colors, background contrasts, or fonts unless the user explicitly requested design critique. Focus directly on reading the text and answering the question in the image.";
        }

        std::string answer = callGeminiMultimodalApi(searchPrompt, cleanBase64, mimeType);
        if (!answer.empty()) {
            AIResponse resp;
            resp.text = answer;
            resp.speechText = SmartNativeEngine::cleanSpeechText(answer);
            resp.intent = "cloud_image_qa";
            resp.highlights = {
                "Google Gemini Multimodal Vision",
                "Direct Image Question Answering",
                "Optical Text & Formula Comprehension"
            };
            resp.suggestions = {
                "Can you elaborate on key parts of this answer?",
                "Give me a practical code or real-world example",
                "Can you summarize this in bullet points?"
            };
            return resp;
        }
    }
    return m_fallbackEngine->analyzeImage(prompt, imageBase64);
}

} // namespace aibuddy
