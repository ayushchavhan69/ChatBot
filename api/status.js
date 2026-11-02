module.exports = async function handler(req, res) {
  res.setHeader("Access-Control-Allow-Origin", "*");
  res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.setHeader("Access-Control-Allow-Methods", "GET, OPTIONS");

  if (req.method === "OPTIONS") {
    return res.status(204).end();
  }

  return res.status(200).json({
    status: "online",
    assistantName: "AI Buddy",
    engine: "Vercel Cloud Serverless",
    model: "Google Gemini 3.5 Flash-Lite",
    uptimeSeconds: Math.floor(process.uptime()),
    timestamp: new Date().toISOString(),
    capabilities: [
      "AI Chat & Brainstorming",
      "Multimodal Image Question Solver & OCR",
      "Persistent Conversation History"
    ]
  });
};
