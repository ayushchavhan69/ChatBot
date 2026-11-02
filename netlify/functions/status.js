const CORS_HEADERS = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Headers": "Content-Type, Authorization",
  "Access-Control-Allow-Methods": "GET, OPTIONS",
  "Content-Type": "application/json; charset=utf-8"
};

exports.handler = async function(event, context) {
  if (event.httpMethod === "OPTIONS") {
    return { statusCode: 204, headers: CORS_HEADERS, body: "" };
  }

  return {
    statusCode: 200,
    headers: CORS_HEADERS,
    body: JSON.stringify({
      status: "online",
      assistantName: "AI Buddy",
      engine: "Cloud Serverless & C++ Native Core",
      model: "Google Gemini 3.5 Flash-Lite",
      uptimeSeconds: Math.floor(process.uptime()),
      timestamp: new Date().toISOString(),
      capabilities: [
        "AI Chat & Brainstorming",
        "Multimodal Image Question Solver & OCR",
        "Persistent Conversation History"
      ]
    })
  };
};
