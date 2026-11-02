/**
 * AI Buddy - Frontend Client Controller
 * Connects the UI to the C++ REST backend
 */

const API_BASE = window.location.origin.startsWith('http') 
  ? window.location.origin 
  : 'http://localhost:8080';

// Application State
const state = {
  activeSessionId: null,
  isRecording: false,
  recognition: null,
  speechSynth: window.speechSynthesis || null,
  currentMode: 'dashboard', // 'dashboard', 'voice', 'chat'
  history: [],
  selectedImageBase64: null,
  selectedImageFileName: '',
  selectedImageMime: '',
  lastRecognizedTranscript: '',
  silenceTimer: null,
  listenTimeoutTimer: null
};

// Initialize on DOM Ready
document.addEventListener('DOMContentLoaded', () => {
  bindUIEvents();
  fetchServerStatus();
  fetchHistory();
  fetchUserProfile();
  navigateTo('dashboard');
});

// Screen Navigation Controller
function navigateTo(screenName) {
  state.currentScreen = 'dashboard';
  const dashScreen = document.getElementById('screen-dashboard');
  if (dashScreen) {
    dashScreen.classList.remove('hidden');
    dashScreen.classList.add('flex');
  }
}
window.navigateTo = navigateTo;

// 1. Backend Connectivity & Health Check
async function fetchServerStatus() {
  const statusBadge = document.getElementById('online-status-indicator');
  const statusText = document.getElementById('online-status-text');
  
  try {
    const res = await fetch(`${API_BASE}/api/status`);
    if (res.ok) {
      const data = await res.json();
      if (statusBadge) statusBadge.className = 'w-2 h-2 rounded-full bg-[#D8FE43] shadow-[0_0_8px_#D8FE43] animate-pulse';
      if (statusText) statusText.textContent = 'Online';
      console.log('✅ Connected to C++ Backend:', data);
    } else {
      throw new Error('Status not ok');
    }
  } catch (err) {
    console.warn('⚠️ C++ backend not reachable at', API_BASE, err);
    if (statusBadge) statusBadge.className = 'w-2 h-2 rounded-full bg-amber-400 shadow-[0_0_8px_#fbbf24]';
    if (statusText) statusText.textContent = 'Offline (Start Server)';
  }
}

async function fetchUserProfile() {
  try {
    const res = await fetch(`${API_BASE}/api/user`);
    if (res.ok) {
      const user = await res.json();
      const nameEl = document.getElementById('greeting-user-name');
      if (nameEl) nameEl.textContent = `Hi, ${user.name}-`;
    }
  } catch (e) {
    // default already in HTML
  }
}

// 2. Fetch & Render History
async function fetchHistory() {
  try {
    const res = await fetch(`${API_BASE}/api/history`);
    if (res.ok) {
      state.history = await res.json();
      renderHistoryList(state.history);
    }
  } catch (err) {
    console.warn('Could not fetch history:', err);
  }
}

function renderHistoryList(items) {
  const container = document.getElementById('history-items-container');
  if (!container) return;

  if (!items || items.length === 0) {
    container.innerHTML = `
      <div class="text-center py-6 text-gray-500 text-xs">
        No conversation history yet. Start a chat or voice session!
      </div>
    `;
    return;
  }

  // Show top 3 recent items in dashboard
  const displayItems = items.slice(0, 3);
  container.innerHTML = displayItems.map((item, idx) => {
    let accentColor = '#D8FE43';
    let iconSvg = '';

    if (item.type === 'voice') {
      accentColor = '#D8FE43';
      iconSvg = `<svg class="w-3.5 h-3.5 text-black" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><path d="M12 1a3 3 0 0 0-3 3v8a3 3 0 0 0 6 0V4a3 3 0 0 0-3-3z"></path><path d="M19 10v1a7 7 0 0 1-14 0v-1"></path></svg>`;
    } else if (item.type === 'image') {
      accentColor = '#FECDD3';
      iconSvg = `<svg class="w-3.5 h-3.5 text-black" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><rect height="18" rx="2" ry="2" width="18" x="3" y="3"></rect><circle cx="8.5" cy="8.5" r="1.5"></circle><polyline points="21 15 16 10 5 21"></polyline></svg>`;
    } else {
      accentColor = '#C5B4FA';
      iconSvg = `<svg class="w-3.5 h-3.5 text-black" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><path d="M8 12h.01M12 12h.01M16 12h.01M21 12c0 4.418-4.03 8-9 8a9.863 9.863 0 01-4.255-.949L3 20l1.395-3.72C3.512 15.042 3 13.574 3 12c0-4.418 4.03-8 9-8s9 3.582 9 8z" stroke-linecap="round" stroke-linejoin="round"></path></svg>`;
    }

    return `
      <div onclick="openHistoryItem('${item.id}')" class="w-full bg-[#141519] hover:bg-[#1a1c22] rounded-2xl p-3 flex items-center justify-between transition-colors cursor-pointer group">
        <div class="flex items-center gap-3 overflow-hidden">
          <span class="w-7 h-7 rounded-full flex-shrink-0 flex items-center justify-center" style="background-color: ${accentColor}">
            ${iconSvg}
          </span>
          <p class="text-xs text-gray-300 font-normal truncate group-hover:text-white transition-colors">
            ${escapeHtml(item.title || 'Conversation')}
          </p>
        </div>
        <button onclick="event.stopPropagation(); deleteHistoryEntry('${item.id}')" aria-label="Delete item" class="text-gray-500 hover:text-red-400 flex-shrink-0 ml-2 p-1" type="button">
          <svg class="w-3.5 h-3.5" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" /></svg>
        </button>
      </div>
    `;
  }).join('');
}

function openHistoryItem(id) {
  const item = state.history.find(h => h.id === id);
  if (!item) return;

  state.activeSessionId = item.id;
  openChatModal();
  
  const container = document.getElementById('chat-messages-container');
  if (container) {
    container.innerHTML = '';
    if (item.messages && item.messages.length > 0) {
      item.messages.forEach(msg => appendChatMessage(msg.sender, msg.content, msg.timestamp));
    }
  }
}

async function deleteHistoryEntry(id) {
  try {
    state.history = state.history.filter(h => h.id !== id);
    renderHistoryList(state.history);
    // You can call DELETE if individual delete is desired, or clear
  } catch (e) {
    console.error(e);
  }
}

async function clearAllHistory() {
  if (!confirm('Are you sure you want to clear all history?')) return;
  try {
    const res = await fetch(`${API_BASE}/api/history`, { method: 'DELETE' });
    if (res.ok) {
      state.history = [];
      renderHistoryList([]);
      closeSeeAllModal();
    }
  } catch (e) {
    console.error('Error clearing history:', e);
  }
}

// 3. Web Speech API (Voice Recognition & Audio Output)
function initSpeechRecognition() {
  const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (SpeechRecognition) {
    state.recognition = new SpeechRecognition();
    state.recognition.continuous = true;
    state.recognition.interimResults = true;
    state.recognition.lang = 'en-US';

    state.recognition.onstart = () => {
      state.isRecording = true;
      state.lastRecognizedTranscript = '';
      updateMicUI(true);
      updateVoiceListeningDisplay('Listening... Speak now', true);

      // Auto-fallback timer: if no speech received in 5 seconds, show quick chips
      clearTimeout(state.listenTimeoutTimer);
      state.listenTimeoutTimer = setTimeout(() => {
        if (state.isRecording && !state.lastRecognizedTranscript) {
          showVoiceFallbackNotice('Still listening... Speak into your mic, or tap a sample question:');
        }
      }, 5000);
    };

    state.recognition.onresult = (event) => {
      clearTimeout(state.listenTimeoutTimer);
      let fullTranscript = '';
      for (let i = 0; i < event.results.length; ++i) {
        fullTranscript += event.results[i][0].transcript + ' ';
      }
      const trimmed = fullTranscript.trim();
      if (trimmed) {
        state.lastRecognizedTranscript = trimmed;
        updateVoiceListeningDisplay(trimmed, false);

        // Auto-submit after 1.8 seconds of silence once speech is detected!
        clearTimeout(state.silenceTimer);
        state.silenceTimer = setTimeout(() => {
          if (state.isRecording && state.lastRecognizedTranscript) {
            finishVoiceRecordingAndSubmit();
          }
        }, 1800);
      }
    };

    state.recognition.onerror = (event) => {
      console.warn('Speech recognition notice:', event.error);
      clearTimeout(state.silenceTimer);
      clearTimeout(state.listenTimeoutTimer);

      // If we already captured a speech transcript, submit it rather than failing!
      if (state.lastRecognizedTranscript) {
        finishVoiceRecordingAndSubmit();
        return;
      }

      updateMicUI(false);
      state.isRecording = false;

      if (event.error === 'not-allowed') {
        showVoiceFallbackNotice('Microphone access is blocked by browser. Enable mic or tap a question below:');
      } else if (event.error === 'no-speech') {
        showVoiceFallbackNotice('No speech detected. Tap microphone to speak again, or try:');
      } else {
        showVoiceFallbackNotice('Microphone ready. Speak clearly into your mic or choose a prompt:');
      }
    };

    state.recognition.onend = () => {
      clearTimeout(state.silenceTimer);
      clearTimeout(state.listenTimeoutTimer);

      // If recognition ended and we have a transcript, submit it!
      if (state.lastRecognizedTranscript) {
        finishVoiceRecordingAndSubmit();
      } else {
        state.isRecording = false;
        updateMicUI(false);
      }
    };
  } else {
    console.info('Web Speech API not supported in this browser. Fallback interactive mode active.');
  }
}

function toggleVoiceRecording() {
  // If AI is currently speaking, stop the audio on mic click
  if (state.speechSynth && state.speechSynth.speaking) {
    state.speechSynth.cancel();
  }

  if (state.isRecording) {
    // Clicking the mic button while recording means: DONE SPEAKING -> SUBMIT!
    finishVoiceRecordingAndSubmit();
  } else {
    state.lastRecognizedTranscript = '';
    if (state.recognition) {
      try {
        state.recognition.start();
      } catch (err) {
        console.warn('Recognition start exception:', err);
        // If already active or browser throws, reset and start
        try {
          state.recognition.stop();
          setTimeout(() => {
            try { state.recognition.start(); } catch (e) {}
          }, 200);
        } catch (e) {
          showVoiceFallbackNotice('Microphone ready. Speak into your mic or choose a prompt:');
        }
      }
    } else {
      showVoiceFallbackNotice('Web Speech API not supported in this browser. Tap a question:');
    }
  }
}
window.toggleVoiceRecording = toggleVoiceRecording;

function finishVoiceRecordingAndSubmit() {
  clearTimeout(state.silenceTimer);
  clearTimeout(state.listenTimeoutTimer);

  const text = (state.lastRecognizedTranscript || '').trim();
  state.lastRecognizedTranscript = '';

  if (state.recognition) {
    try { state.recognition.stop(); } catch (e) {}
  }
  state.isRecording = false;
  updateMicUI(false);

  if (text && text !== 'Listening... Speak now') {
    updateVoiceAnalyzingDisplay(text);
    handleVoiceQuerySubmit(text);
  } else {
    showVoiceFallbackNotice('Didn\'t catch that. Tap the mic and speak clearly, or try:');
  }
}
window.finishVoiceRecordingAndSubmit = finishVoiceRecordingAndSubmit;

function updateMicUI(active) {
  const micBtn = document.getElementById('voice-mic-button');
  const orb = document.querySelector('.iridescent-orb');
  const aura = document.querySelector('.iridescent-orb')?.parentElement?.firstElementChild;
  const waveOuter = document.getElementById('mic-wave-outer');
  const waveInner = document.getElementById('mic-wave-inner');

  if (active) {
    if (micBtn) {
      micBtn.classList.add('ring-8', 'ring-lime-300/40', 'scale-105');
    }
    if (orb) {
      orb.style.animationDuration = '3s';
      orb.style.filter = 'brightness(1.3) saturate(1.4)';
    }
    if (aura) {
      aura.classList.add('scale-125');
    }
    if (waveOuter) waveOuter.classList.add('animate-ping');
    if (waveInner) waveInner.classList.add('animate-pulse');
  } else {
    if (micBtn) {
      micBtn.classList.remove('ring-8', 'ring-lime-300/40', 'scale-105');
    }
    if (orb) {
      orb.style.animationDuration = '10s';
      orb.style.filter = 'none';
    }
    if (aura) {
      aura.classList.remove('scale-125');
    }
    if (waveOuter) waveOuter.classList.remove('animate-ping');
    if (waveInner) waveInner.classList.remove('animate-pulse');
  }
}

function updateVoiceListeningDisplay(text, isInterim = false) {
  const container = document.getElementById('spoken-query-display');
  if (!container) return;

  const isPlaceholder = text === 'Listening... Speak now';

  container.innerHTML = `
    <div class="flex flex-col items-center gap-1.5 w-full">
      <div class="inline-flex items-center gap-1.5 px-3 py-1 rounded-full bg-[#D8FE43]/20 text-[#D8FE43] text-[11px] font-semibold animate-pulse">
        <span class="w-2 h-2 rounded-full bg-[#D8FE43] animate-ping"></span> ${isPlaceholder ? 'Listening... Speak now' : 'Hearing you...'}
      </div>
      <p class="text-white text-sm font-medium leading-relaxed max-h-20 overflow-y-auto px-2">
        "${escapeHtml(text)}"
      </p>
      <button onclick="finishVoiceRecordingAndSubmit()" class="mt-1 px-3 py-1 bg-white/10 hover:bg-white/20 text-[#D8FE43] text-[11px] font-semibold rounded-full border border-lime-400/30 transition-colors shadow flex items-center gap-1">
        <span>Tap when done speaking</span> <span>✓</span>
      </button>
    </div>
  `;
}

function updateVoiceAnalyzingDisplay(query) {
  const container = document.getElementById('spoken-query-display');
  if (!container) return;

  container.innerHTML = `
    <div class="flex flex-col items-center gap-1.5 w-full">
      <div class="inline-flex items-center gap-1.5 px-3 py-1 rounded-full bg-purple-500/20 text-purple-300 text-[11px] font-semibold">
        <svg class="animate-spin h-3.5 w-3.5 text-purple-300" viewBox="0 0 24 24" fill="none"><circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle><path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path></svg>
        AI Buddy Analyzing with Gemini...
      </div>
      <p class="text-gray-300 text-xs italic px-2 max-h-16 overflow-y-auto">
        "${escapeHtml(query)}"
      </p>
    </div>
  `;
}

function displayVoiceQA(userQuestion, botAnswer) {
  const container = document.getElementById('spoken-query-display');
  if (!container) return;

  let formatted = escapeHtml(botAnswer).replace(/\n/g, '<br/>');
  formatted = formatted.replace(/\*\*(.*?)\*\*/g, '<strong class="text-[#D8FE43] font-semibold">$1</strong>');
  formatted = formatted.replace(/\* (.*?)(<br\/>|$)/g, '<li class="ml-2">$1</li>');

  container.innerHTML = `
    <div class="w-full flex flex-col gap-2 max-h-[210px] overflow-y-auto pr-1 select-text text-left">
      <!-- User Spoken Text -->
      <div class="bg-white/5 border border-white/10 rounded-2xl p-2.5 shadow-sm">
        <div class="text-[10px] text-[#D8FE43] font-bold flex items-center gap-1.5 uppercase tracking-wide">
          <span>🎙️ You Asked:</span>
        </div>
        <p class="text-white text-xs font-medium leading-snug mt-0.5">"${escapeHtml(userQuestion)}"</p>
      </div>

      <!-- AI Buddy Text Answer -->
      <div class="bg-[#121317] border border-lime-400/40 rounded-2xl p-2.5 shadow-xl relative">
        <div class="flex items-center justify-between mb-1 pb-1 border-b border-gray-800">
          <span class="text-[10px] text-[#D8FE43] font-bold flex items-center gap-1.5">
            <span>🤖 AI Buddy Answer</span>
            <span id="speech-live-indicator" class="text-gray-400 text-[10px] font-normal flex items-center gap-1">
              <span class="w-1.5 h-1.5 rounded-full bg-[#D8FE43] animate-ping"></span> Speaking 🔊
            </span>
          </span>
          <button onclick="stopSpeaking()" class="text-[10px] text-gray-400 hover:text-white px-1.5 py-0.5 bg-black/50 hover:bg-black/80 rounded border border-gray-700 transition-colors">
            🔇 Stop
          </button>
        </div>
        <div class="text-gray-200 text-xs leading-relaxed max-h-24 overflow-y-auto pr-1">
          ${formatted}
        </div>
      </div>
    </div>
  `;
}

function showVoiceFallbackNotice(message) {
  const container = document.getElementById('spoken-query-display');
  if (!container) return;

  const samples = [
    "What are trending interface design tools?",
    "Explain how machine learning works",
    "Give me color palette ideas for a dark UI",
    "What is the Transformer architecture?"
  ];

  container.innerHTML = `
    <div class="w-full flex flex-col items-center gap-2 text-left">
      <p class="text-[11px] text-amber-300 text-center font-medium">${escapeHtml(message)}</p>
      <form onsubmit="handleVoiceFallbackSubmit(event)" class="w-full flex items-center gap-1.5">
        <input id="voice-fallback-input" type="text" placeholder="Type your question to speak..." class="flex-1 bg-white/10 border border-gray-700 rounded-xl px-3 py-1.5 text-xs text-white placeholder-gray-500 focus:outline-none focus:border-[#D8FE43]">
        <button type="submit" class="px-3 py-1.5 bg-[#D8FE43] hover:bg-[#cbf13b] text-black font-bold text-xs rounded-xl shadow cursor-pointer">Ask</button>
      </form>
      <div class="flex items-center gap-1 flex-wrap justify-center pt-1">
        <span class="text-[10px] text-gray-500">Quick suggestions:</span>
        ${samples.map(s => `
          <button type="button" onclick="handleVoiceQuerySubmit('${escapeHtml(s.replace(/'/g, "\\'"))}')" class="px-2 py-0.5 bg-[#171920] hover:bg-[#222530] text-[#D8FE43] hover:text-white rounded text-[10px] border border-gray-800 transition-colors">
            ${escapeHtml(s)}
          </button>
        `).join('')}
      </div>
    </div>
  `;
}

function handleVoiceFallbackSubmit(event) {
  if (event) event.preventDefault();
  const input = document.getElementById('voice-fallback-input');
  if (!input) return;
  const val = input.value.trim();
  if (!val) return;
  handleVoiceQuerySubmit(val);
}
window.handleVoiceFallbackSubmit = handleVoiceFallbackSubmit;

async function handleVoiceQuerySubmit(query) {
  updateVoiceAnalyzingDisplay(query);

  try {
    const res = await fetch(`${API_BASE}/api/voice/transcribe`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ query: query, sessionId: state.activeSessionId || '' })
    });

    if (res.ok) {
      const data = await res.json();
      state.activeSessionId = data.sessionId;
      
      // 1. Display user question and AI answer in text format on screen!
      displayVoiceQA(query, data.reply);

      // 2. Speak response aloud using Web Speech Synthesis
      speakText(data.speechText || data.reply);
      
      // 3. Refresh recent history on dashboard
      fetchHistory();
    } else {
      const err = await res.json().catch(() => ({}));
      const container = document.getElementById('spoken-query-display');
      if (container) {
        container.innerHTML = `<div class="p-2 bg-red-950/40 border border-red-900 rounded-xl text-red-300 text-xs">Error: ${escapeHtml(err.message || 'Voice request failed')}</div>`;
      }
    }
  } catch (err) {
    console.error('Error sending voice query:', err);
    const container = document.getElementById('spoken-query-display');
    if (container) {
      container.innerHTML = `<div class="p-2 bg-red-950/40 border border-red-900 rounded-xl text-red-300 text-xs">Cannot reach C++ backend server on port 8080.</div>`;
    }
  }
}
window.handleVoiceQuerySubmit = handleVoiceQuerySubmit;

function speakText(text) {
  if (!state.speechSynth) return;
  state.speechSynth.cancel(); // Stop any pending speech

  const utterance = new SpeechSynthesisUtterance(text);
  utterance.rate = 1.05;
  utterance.pitch = 1.0;
  
  // Try to find a friendly natural voice
  const voices = state.speechSynth.getVoices();
  const englishVoice = voices.find(v => v.lang.startsWith('en') && (v.name.includes('Natural') || v.name.includes('Google') || v.name.includes('Samantha')));
  if (englishVoice) {
    utterance.voice = englishVoice;
  }

  const orb = document.querySelector('.iridescent-orb');
  utterance.onstart = () => {
    if (orb) {
      orb.style.animationDuration = '4s';
      orb.style.filter = 'brightness(1.25) saturate(1.3)';
    }
  };
  utterance.onend = () => {
    if (orb) {
      orb.style.animationDuration = '10s';
      orb.style.filter = 'none';
    }
    const indicator = document.getElementById('speech-live-indicator');
    if (indicator) {
      indicator.innerHTML = `<span class="text-gray-500">• Finished speaking</span>`;
    }
  };

  state.speechSynth.speak(utterance);
}

function stopSpeaking() {
  if (state.speechSynth) {
    state.speechSynth.cancel();
  }
  const indicator = document.getElementById('speech-live-indicator');
  if (indicator) {
    indicator.innerHTML = `<span class="text-gray-500">• Muted 🔇</span>`;
  }
  const orb = document.querySelector('.iridescent-orb');
  if (orb) {
    orb.style.animationDuration = '10s';
    orb.style.filter = 'none';
  }
}
window.stopSpeaking = stopSpeaking;

function resetVoiceDisplay() {
  const container = document.getElementById('spoken-query-display');
  if (container) {
    container.innerHTML = `
      <p class="text-gray-400 text-xs font-normal">
        Tap microphone or orb to speak...
      </p>
    `;
  }
}
window.resetVoiceDisplay = resetVoiceDisplay;

function closeVoiceSession() {
  if (state.isRecording) {
    if (state.recognition) {
      try { state.recognition.stop(); } catch (e) {}
    }
    state.isRecording = false;
    updateMicUI(false);
  }
  if (state.speechSynth) {
    state.speechSynth.cancel();
  }
  resetVoiceDisplay();
  navigateTo('dashboard');
}
window.closeVoiceSession = closeVoiceSession;

// 4. Chat Modal Interaction
function openChatModal() {
  const modal = document.getElementById('chat-modal');
  if (modal) {
    modal.classList.remove('hidden');
    modal.classList.add('flex');
    const input = document.getElementById('chat-input-field');
    if (input) input.focus();
  }
}

function closeChatModal() {
  const modal = document.getElementById('chat-modal');
  if (modal) {
    modal.classList.add('hidden');
    modal.classList.remove('flex');
  }
}

async function sendChatMessage() {
  const input = document.getElementById('chat-input-field');
  if (!input) return;
  const message = input.value.trim();
  if (!message) return;

  // Append user message
  appendChatMessage('user', message, new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }));
  input.value = '';

  // Show typing indicator
  const typingId = showTypingIndicator();

  try {
    const res = await fetch(`${API_BASE}/api/chat`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        message: message,
        sessionId: state.activeSessionId || '',
        mode: 'chat'
      })
    });

    removeTypingIndicator(typingId);

    if (res.ok) {
      const data = await res.json();
      state.activeSessionId = data.sessionId;
      appendChatMessage('bot', data.reply, new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }), data.suggestions);
      fetchHistory();
    } else {
      const errData = await res.json().catch(() => ({}));
      appendChatMessage('bot', errData.message || 'Sorry, I encountered an issue processing your request.', '');
    }
  } catch (err) {
    removeTypingIndicator(typingId);
    appendChatMessage('bot', 'Could not reach C++ backend server. Please make sure server.exe is running on port 8080.', '');
  }
}

function appendChatMessage(sender, text, time, suggestions = []) {
  const container = document.getElementById('chat-messages-container');
  if (!container) return;

  const isUser = sender === 'user';
  const bubble = document.createElement('div');
  bubble.className = `flex flex-col ${isUser ? 'items-end' : 'items-start'} mb-3`;

  let formattedText = escapeHtml(text).replace(/\n/g, '<br/>');
  // Simple bold markdown
  formattedText = formattedText.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>');

  bubble.innerHTML = `
    <div class="max-w-[85%] rounded-2xl px-4 py-2.5 text-xs sm:text-sm ${
      isUser 
        ? 'bg-[#D8FE43] text-black rounded-tr-none font-medium' 
        : 'bg-[#181A20] text-gray-200 border border-gray-800 rounded-tl-none'
    }">
      ${formattedText}
    </div>
    ${time ? `<span class="text-[10px] text-gray-500 mt-1 px-1">${time}</span>` : ''}
    ${suggestions && suggestions.length > 0 ? `
      <div class="flex flex-wrap gap-1.5 mt-2">
        ${suggestions.map(s => `
          <button onclick="sendQuickPrompt('${escapeHtml(s)}')" class="text-[11px] bg-[#22242b] hover:bg-[#2c2f38] text-gray-300 px-2.5 py-1 rounded-full border border-gray-700/60 transition-colors">
            ${escapeHtml(s)}
          </button>
        `).join('')}
      </div>
    ` : ''}
  `;

  container.appendChild(bubble);
  container.scrollTop = container.scrollHeight;
}

function sendQuickPrompt(promptText) {
  const input = document.getElementById('chat-input-field');
  if (input) {
    input.value = promptText;
    sendChatMessage();
  }
}

function showTypingIndicator() {
  const container = document.getElementById('chat-messages-container');
  if (!container) return null;

  const id = 'typing_' + Date.now();
  const el = document.createElement('div');
  el.id = id;
  el.className = 'flex items-center gap-1.5 py-2 px-4 bg-[#181A20] rounded-2xl max-w-[70px] border border-gray-800 mb-3';
  el.innerHTML = `
    <span class="w-1.5 h-1.5 bg-gray-400 rounded-full animate-bounce"></span>
    <span class="w-1.5 h-1.5 bg-gray-400 rounded-full animate-bounce [animation-delay:0.2s]"></span>
    <span class="w-1.5 h-1.5 bg-gray-400 rounded-full animate-bounce [animation-delay:0.4s]"></span>
  `;
  container.appendChild(el);
  container.scrollTop = container.scrollHeight;
  return id;
}

function removeTypingIndicator(id) {
  const el = document.getElementById(id);
  if (el) el.remove();
}

// 5. Image Search & Multimodal Analysis
function openImageModal() {
  const modal = document.getElementById('image-modal');
  if (modal) {
    modal.classList.remove('hidden');
    modal.classList.add('flex');
  }
}

function closeImageModal() {
  const modal = document.getElementById('image-modal');
  if (modal) {
    modal.classList.add('hidden');
    modal.classList.remove('flex');
  }
}

function triggerImageUpload() {
  const fileInput = document.getElementById('image-file-input');
  if (fileInput) fileInput.click();
}

function handleImageFileSelect(event) {
  const file = event.target.files && event.target.files[0];
  if (file) {
    processImageFile(file);
  }
}

function processImageFile(file) {
  if (!file.type.startsWith('image/')) {
    alert('Please select a valid image file (PNG, JPG, WebP, etc.)');
    return;
  }
  const reader = new FileReader();
  reader.onload = (e) => {
    state.selectedImageBase64 = e.target.result;
    state.selectedImageFileName = file.name;
    state.selectedImageMime = file.type;
    displayImagePreview(e.target.result, file.name, (file.size / 1024).toFixed(1) + ' KB');
  };
  reader.readAsDataURL(file);
}

function displayImagePreview(dataUrl, name, sizeText = '') {
  const promptBox = document.getElementById('image-upload-prompt');
  const previewContainer = document.getElementById('image-preview-container');
  const previewImg = document.getElementById('image-upload-preview');
  const fileInfo = document.getElementById('image-file-info');

  if (previewImg) previewImg.src = dataUrl;
  if (fileInfo) fileInfo.textContent = `${name} ${sizeText ? '(' + sizeText + ')' : ''}`;

  if (promptBox) promptBox.classList.add('hidden');
  if (previewContainer) previewContainer.classList.remove('hidden');

  const resultContainer = document.getElementById('image-analysis-result');
  if (resultContainer) resultContainer.innerHTML = '';
}

function removeSelectedImage(event) {
  if (event) event.stopPropagation();
  state.selectedImageBase64 = null;
  state.selectedImageFileName = '';
  state.selectedImageMime = '';

  const fileInput = document.getElementById('image-file-input');
  if (fileInput) fileInput.value = '';

  const promptBox = document.getElementById('image-upload-prompt');
  const previewContainer = document.getElementById('image-preview-container');
  const previewImg = document.getElementById('image-upload-preview');

  if (previewImg) previewImg.src = '';
  if (previewContainer) previewContainer.classList.add('hidden');
  if (promptBox) promptBox.classList.remove('hidden');
}

async function useCurrentSnapshot() {
  try {
    state.selectedImageBase64 = 'embedded_screen_sample';
    state.selectedImageFileName = 'Current App Screen Snapshot';
    displayImagePreview('./screen.png', 'Current App Screen Snapshot', '238 KB');
  } catch (err) {
    console.error('Snapshot error:', err);
  }
}

function setImagePromptPreset(text) {
  const input = document.getElementById('image-prompt-input');
  if (input) {
    input.value = text;
    input.focus();
  }
}

function handleImageDragOver(event) {
  event.preventDefault();
  event.stopPropagation();
  const dropzone = document.getElementById('image-dropzone');
  if (dropzone) {
    dropzone.classList.add('border-[#FECDD3]', 'bg-[#181a20]');
  }
}

function handleImageDragLeave(event) {
  event.preventDefault();
  event.stopPropagation();
  const dropzone = document.getElementById('image-dropzone');
  if (dropzone) {
    dropzone.classList.remove('border-[#FECDD3]', 'bg-[#181a20]');
  }
}

function handleImageDrop(event) {
  event.preventDefault();
  event.stopPropagation();
  const dropzone = document.getElementById('image-dropzone');
  if (dropzone) {
    dropzone.classList.remove('border-[#FECDD3]', 'bg-[#181a20]');
  }
  if (event.dataTransfer && event.dataTransfer.files && event.dataTransfer.files.length > 0) {
    processImageFile(event.dataTransfer.files[0]);
  }
}

// Paste support: when modal is open and user presses Ctrl+V with an image
window.addEventListener('paste', (event) => {
  const modal = document.getElementById('image-modal');
  if (!modal || modal.classList.contains('hidden')) return;

  const items = (event.clipboardData || event.originalEvent.clipboardData).items;
  for (let i = 0; i < items.length; i++) {
    if (items[i].type.indexOf('image') !== -1) {
      const file = items[i].getAsFile();
      if (file) {
        processImageFile(file);
        break;
      }
    }
  }
});

async function analyzeSelectedImage(presetPrompt = '') {
  const resultContainer = document.getElementById('image-analysis-result');
  const promptInput = document.getElementById('image-prompt-input');
  const runBtn = document.getElementById('btn-run-image-analysis');

  const customPrompt = presetPrompt || (promptInput ? promptInput.value.trim() : '');
  const activePrompt = customPrompt; // If empty, backend automatically reads and answers the question inside the image!
  const payloadImage = state.selectedImageBase64 || 'embedded_screen_sample';

  if (runBtn) {
    runBtn.disabled = true;
    runBtn.classList.add('opacity-60', 'cursor-not-allowed');
  }

  if (resultContainer) {
    resultContainer.innerHTML = `
      <div class="p-4 bg-[#141519] border border-gray-800 rounded-2xl flex items-center gap-3 text-xs text-[#FECDD3]">
        <svg class="animate-spin h-5 w-5 text-[#FECDD3]" viewBox="0 0 24 24" fill="none">
          <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
          <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
        </svg>
        <div>
          <p class="font-bold text-white">Gemini Multimodal AI is reading the image & solving...</p>
          <p class="text-[11px] text-gray-400">Extracting questions, text & formulas to generate direct answer via C++ Core</p>
        </div>
      </div>
    `;
  }

  try {
    const res = await fetch(`${API_BASE}/api/image/analyze`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        prompt: activePrompt,
        image: payloadImage,
        sessionId: state.activeSessionId || ''
      })
    });

    if (res.ok) {
      const data = await res.json();
      if (resultContainer) {
        let formatted = escapeHtml(data.analysis || '').replace(/\n/g, '<br/>');
        formatted = formatted.replace(/\*\*(.*?)\*\*/g, '<strong class="text-white font-semibold">$1</strong>');
        formatted = formatted.replace(/### (.*?)(<br\/>|$)/g, '<h4 class="text-xs font-bold text-[#D8FE43] mt-3 mb-1 uppercase tracking-wider">$1</h4>');
        formatted = formatted.replace(/## (.*?)(<br\/>|$)/g, '<h4 class="text-sm font-bold text-white mt-3 mb-1">$1</h4>');
        formatted = formatted.replace(/\* (.*?)(<br\/>|$)/g, '<li class="ml-3">$1</li>');

        resultContainer.innerHTML = `
          <div class="bg-[#121317] border border-gray-800 rounded-2xl p-4 text-xs text-gray-300 space-y-3">
            <div class="flex items-center justify-between pb-2 border-b border-gray-800">
              <span class="inline-flex items-center gap-1.5 px-2 py-0.5 rounded-full bg-[#FECDD3]/20 text-[#FECDD3] font-semibold text-[10px]">
                ⚡ Answer / Vision Result
              </span>
              <span class="text-[10px] text-gray-500 font-mono">${escapeHtml(data.model || 'Gemini 3.5 Flash-Lite')}</span>
            </div>

            <div class="leading-relaxed text-gray-300 max-h-72 overflow-y-auto pr-1">
              ${formatted}
            </div>

            ${data.suggestions && data.suggestions.length > 0 ? `
              <div class="pt-2 border-t border-gray-800/80">
                <span class="text-[11px] font-semibold text-white">Suggested Follow-ups:</span>
                <div class="flex flex-wrap gap-1.5 mt-1.5">
                  ${data.suggestions.map(s => `
                    <button type="button" onclick="continueImageInChat('${escapeHtml(s.replace(/'/g, "\\'"))}')" 
                            class="px-2.5 py-1 bg-[#1A1C22] hover:bg-[#282B34] text-gray-300 hover:text-white rounded-lg text-[10px] border border-gray-700/60 transition-colors">
                      💬 ${escapeHtml(s)}
                    </button>
                  `).join('')}
                </div>
              </div>
            ` : ''}

            <div class="pt-2 border-t border-gray-800 flex items-center justify-between">
              <button type="button" onclick="continueImageInChat()" class="px-3 py-1.5 bg-[#D8FE43] hover:bg-[#cbf536] text-black font-semibold text-[11px] rounded-lg transition-colors flex items-center gap-1.5 shadow">
                💬 Discuss in Chat
              </button>
              <button type="button" onclick="closeImageModal()" class="text-xs text-gray-400 hover:text-white transition-colors">
                Done
              </button>
            </div>
          </div>
        `;
      }
      fetchHistory();
    } else {
      const errData = await res.json().catch(() => ({}));
      if (resultContainer) {
        resultContainer.innerHTML = `
          <div class="p-3 bg-red-950/40 border border-red-900/60 rounded-xl text-red-300 text-xs">
            <strong>Analysis Failed:</strong> ${escapeHtml(errData.message || 'Error communicating with backend.')}
          </div>
        `;
      }
    }
  } catch (err) {
    if (resultContainer) {
      resultContainer.innerHTML = `<div class="p-3 bg-red-950/40 border border-red-900/60 rounded-xl text-red-300 text-xs">Could not connect to C++ backend server. Please verify server is running on port 8080.</div>`;
    }
  } finally {
    if (runBtn) {
      runBtn.disabled = false;
      runBtn.classList.remove('opacity-60', 'cursor-not-allowed');
    }
  }
}

function continueImageInChat(followUpPrompt = '') {
  closeImageModal();
  openChatModal();
  if (followUpPrompt) {
    const input = document.getElementById('chat-input-text');
    if (input) {
      input.value = followUpPrompt;
      sendChatMessage();
    }
  }
}

// 6. See All History Modal
function openSeeAllModal() {
  const modal = document.getElementById('history-modal');
  if (!modal) return;
  modal.classList.remove('hidden');
  modal.classList.add('flex');

  const list = document.getElementById('all-history-list');
  if (list) {
    if (!state.history || state.history.length === 0) {
      list.innerHTML = `<p class="text-gray-500 text-xs text-center py-6">No historical sessions recorded.</p>`;
    } else {
      list.innerHTML = state.history.map(item => `
        <div onclick="openHistoryItem('${item.id}'); closeSeeAllModal();" class="p-3 bg-[#141519] hover:bg-[#1c1e24] rounded-2xl flex items-center justify-between cursor-pointer border border-gray-800/40">
          <div>
            <div class="text-xs font-medium text-white">${escapeHtml(item.title || 'Session')}</div>
            <div class="text-[10px] text-gray-500 mt-0.5">${item.timestamp || ''} • ${item.type.toUpperCase()}</div>
          </div>
          <span class="text-xs text-lime-400">Open ↗</span>
        </div>
      `).join('');
    }
  }
}

function closeSeeAllModal() {
  const modal = document.getElementById('history-modal');
  if (modal) {
    modal.classList.add('hidden');
    modal.classList.remove('flex');
  }
}

// 7. Bind UI Events
function bindUIEvents() {
  // Chat with Bot card (first card in grid)
  const chatCard = document.querySelector('[data-purpose="bot-actions-grid"] article:first-child');
  if (chatCard) {
    chatCard.addEventListener('click', () => {
      state.activeSessionId = null;
      openChatModal();
    });
  }

  // Search by Image card (second card in grid)
  const imageCard = document.querySelector('[data-purpose="bot-actions-grid"] article:nth-child(2)');
  if (imageCard) {
    imageCard.addEventListener('click', () => {
      openImageModal();
    });
  }

  // See all history link
  const seeAllLink = document.querySelector('a[href="#see-all"]');
  if (seeAllLink) {
    seeAllLink.addEventListener('click', (e) => {
      e.preventDefault();
      openSeeAllModal();
    });
  }
}

// Utility: Escape HTML
function escapeHtml(str) {
  if (!str) return '';
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

