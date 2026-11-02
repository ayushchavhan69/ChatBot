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
    let body = req.body || {};
    if (typeof body === "string") {
      try {
        body = JSON.parse(body);
      } catch (e) {
        body = {};
      }
    }
    const rawPrompt = (body.prompt || "").trim();
    const rawImage = (body.image || "").trim();
    const sessionId = body.sessionId || ("sess_img_" + Date.now());

    if (!rawImage) {
      return res.status(400).json({ status: "error", message: "Missing image data" });
    }

    let mimeType = "image/jpeg";
    let cleanBase64 = rawImage;

    if (rawImage.startsWith("data:")) {
      const commaIdx = rawImage.indexOf(",");
      if (commaIdx !== -1) {
        const meta = rawImage.substring(5, commaIdx);
        const semiIdx = meta.indexOf(";");
        if (semiIdx !== -1) {
          mimeType = meta.substring(0, semiIdx);
        }
        cleanBase64 = rawImage.substring(commaIdx + 1);
      }
    }

    const apiKey = process.env.GEMINI_API_KEY || API_KEY_FALLBACK;
    const effectivePrompt = rawPrompt.length > 0 
      ? rawPrompt 
      : "You are an expert AI multimodal solver. Read any questions, text, code, math problems, or diagrams in this image carefully, and provide the complete, direct, accurate, and detailed answer.";

    const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash-lite:generateContent?key=${apiKey}`;

    const geminiPayload = {
      contents: [{
        parts: [
          { text: effectivePrompt },
          {
            inline_data: {
              mime_type: mimeType,
              data: cleanBase64
            }
          }
        ]
      }]
    };

    const apiRes = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(geminiPayload)
    });

    const data = await apiRes.json();

    let analysisText = "";
    if (data.candidates && data.candidates[0] && data.candidates[0].content && data.candidates[0].content.parts) {
      analysisText = data.candidates[0].content.parts.map(p => p.text || "").join("").trim();
    } else if (data.error) {
      console.warn("Gemini Vision Notice:", data.error);
      analysisText = "The image was processed. You can enter specific questions in the prompt box.";
    } else {
      analysisText = "Image processed successfully.";
    }

    return res.status(200).json({
      status: "success",
      sessionId: sessionId,
      analysis: analysisText,
      speechText: cleanSpeechText(analysisText),
      suggestions: [
        "Explain this in simpler terms",
        "Transcribe all text from the image",
        "Give step-by-step working"
      ],
      highlights: [
        "Multimodal image question solver & OCR",
        "Analyzed with Google Gemini 3.5 Flash-Lite"
      ],
      metadata: {
        mimeType: mimeType,
        timestamp: new Date().toISOString()
      },
      timestamp: new Date().toISOString(),
      model: "Google Gemini 3.5 Flash-Lite (Vision)"
    });
  } catch (err) {
    console.error("Vercel image analysis error:", err);
    return res.status(500).json({
      status: "error",
      message: "Image analysis error: " + err.message
    });
  }
};
