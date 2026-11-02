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

module.exports = async function handler(req, res) {
  res.setHeader("Access-Control-Allow-Origin", "*");
  res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.setHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");

  if (req.method === "OPTIONS") {
    return res.status(204).end();
  }

  if (req.method === "DELETE" || (req.method === "POST" && req.url && req.url.includes("clear"))) {
    serverlessHistory = [];
    return res.status(200).json({ status: "success", message: "History cleared successfully" });
  }

  return res.status(200).json(serverlessHistory);
};
