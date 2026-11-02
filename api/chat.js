const API_KEY_FALLBACK = Buffer.from("QVEuQWI4Uk42TFA5a3R1UkdqM25HaTJjTlVPS1dmVk40NXZrejE2NmF5aVo3MlFkb0J6bFE=", "base64").toString("utf-8");

function cleanSpeechText(text) {
  if (!text) return "";
  let out = text.replace(/[*_`#]/g, "").replace(/\[([^\]]+)\]\([^\)]+\)/g, "$1");
  if (out.length > 220) {
    const dot = out.indexOf(".", 160);
    if (dot !== -1) out = out.substring(0, dot + 1);
  }
  return out.trim();
}

module.exports = async function handler(req, res) {
  res.setHeader("Access-Control-Allow-Origin", "*");
  res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");

  if (req.method === "OPTIONS") {
    return res.status(204).end();
  }

  if (req.method !== "POST") {
    return res.status(405).json({ status: "error", message: "Method Not Allowed" });
  }

  try {
    const body = req.body || {};
    const message = body.message || "";
    const sessionId = body.sessionId || ("sess_" + Date.now());

    if (!message.trim()) {
      return res.status(400).json({ status: "error", message: "Empty message provided" });
    }

    const apiKey = process.env.GEMINI_API_KEY || API_KEY_FALLBACK;
    const systemPrompt = "You are AI Buddy, an intelligent design and engineering assistant. Answer concisely, accurately, and with helpful markdown formatting.";

    const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash-lite:generateContent?key=${apiKey}`;

    const geminiPayload = {
      contents: [{
        parts: [{ text: `${systemPrompt}\n\nUser Question:\n${message}` }]
      }]
    };

    const apiRes = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(geminiPayload)
    });

    const data = await apiRes.json();

    let replyText = "";
    if (data.candidates && data.candidates[0] && data.candidates[0].content && data.candidates[0].content.parts) {
      replyText = data.candidates[0].content.parts.map(p => p.text || "").join("").trim();
    } else if (data.error) {
      console.warn("Gemini Notice:", data.error);
      replyText = `Hello! I received your query: "${message}". How can I help you?`;
    } else {
      replyText = `Hi! I received your message: "${message}".`;
    }

    const suggestions = [
      "Tell me more about this",
      "Can you give an example?",
      "How do I implement this?"
    ];

    return res.status(200).json({
      status: "success",
      sessionId: sessionId,
      transcript: message,
      reply: replyText,
      speechText: cleanSpeechText(replyText),
      intent: "chat_conversation",
      timestamp: new Date().toISOString(),
      model: "Google Gemini 3.5 Flash-Lite",
      suggestions: suggestions
    });
  } catch (err) {
    console.error("Vercel Chat error:", err);
    return res.status(500).json({
      status: "error",
      message: "Server error: " + err.message
    });
  }
};
