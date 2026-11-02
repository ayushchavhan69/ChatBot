#pragma once

#include <string>
#include <vector>
#include <memory>
#include "json.hpp"

namespace aibuddy {

struct AIResponse {
    std::string text;
    std::string speechText;
    std::string intent;
    std::vector<std::string> suggestions;
    std::vector<std::string> highlights;
    nlohmann::json metadata;
};

class IAIModel {
public:
    virtual ~IAIModel() = default;
    virtual AIResponse generateTextResponse(const std::string& prompt, const std::string& context = "") = 0;
    virtual AIResponse generateVoiceResponse(const std::string& query) = 0;
    virtual AIResponse analyzeImage(const std::string& prompt, const std::string& imageBase64) = 0;
    virtual std::string getModelName() const = 0;
};

// Smart Offline Native C++ AI Engine (Domain-focused on UI/UX, Design, Tech & Voice Chat)
class SmartNativeEngine : public IAIModel {
public:
    SmartNativeEngine();

    AIResponse generateTextResponse(const std::string& prompt, const std::string& context = "") override;
    AIResponse generateVoiceResponse(const std::string& query) override;
    AIResponse analyzeImage(const std::string& prompt, const std::string& imageBase64) override;
    std::string getModelName() const override { return "AI-Buddy-Neural-v1.4 (Native C++)"; }

    static std::string cleanSpeechText(const std::string& input);

private:
    std::string handleDesignQuery(const std::string& prompt, AIResponse& response);
    std::string handleVoiceQuery(const std::string& query, AIResponse& response);
    std::string handleCodeQuery(const std::string& prompt, AIResponse& response);
    std::string handleGeneralQuery(const std::string& prompt, AIResponse& response);
};

// Cloud LLM Provider (Google Gemini API via Windows WinHTTP Native TLS)
class CloudAIAdapter : public IAIModel {
public:
    CloudAIAdapter(std::string apiKey, std::string provider = "gemini");

    AIResponse generateTextResponse(const std::string& prompt, const std::string& context = "") override;
    AIResponse generateVoiceResponse(const std::string& query) override;
    AIResponse analyzeImage(const std::string& prompt, const std::string& imageBase64) override;
    std::string getModelName() const override;

private:
    std::string m_apiKey;
    std::string m_provider;
    std::string m_modelName = "gemini-3.5-flash-lite";
    std::unique_ptr<SmartNativeEngine> m_fallbackEngine;

    std::string callGeminiApi(const std::string& prompt, const std::string& systemInstruction = "");
    std::string callGeminiMultimodalApi(const std::string& prompt, const std::string& cleanBase64, const std::string& mimeType);
    static std::string base64Encode(const std::vector<unsigned char>& data);
};

} // namespace aibuddy
