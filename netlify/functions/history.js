const CORS_HEADERS = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Headers": "Content-Type, Authorization",
  "Access-Control-Allow-Methods": "GET, POST, DELETE, OPTIONS",
  "Content-Type": "application/json; charset=utf-8"
};

// In-memory session cache for serverless environment
let serverlessHistory = [
  {
    id: "demo_welcome",
    title: "Welcome to AI Buddy",
    type: "chat",
    timestamp: "Today",
    messages: [
      { sender: "bot", content: "Welcome! Ask me anything or upload an image to solve questions.", timestamp: "Just now" }
    ]
  }
];

exports.handler = async function(event, context) {
  if (event.httpMethod === "OPTIONS") {
    return { statusCode: 204, headers: CORS_HEADERS, body: "" };
  }

  if (event.httpMethod === "DELETE" || (event.httpMethod === "POST" && event.path.endsWith("/clear"))) {
    serverlessHistory = [];
    return {
      statusCode: 200,
      headers: CORS_HEADERS,
      body: JSON.stringify({ status: "success", message: "History cleared successfully" })
    };
  }

  return {
    statusCode: 200,
    headers: CORS_HEADERS,
    body: JSON.stringify(serverlessHistory)
  };
};
