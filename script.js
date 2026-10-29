// ============================================================================
// OOP MASTER: INTERACTIVE MULTILINGUAL CHATBOT ENGINE
// Supports C++ WinSock2 API, typo handling, context awareness, and tap buttons
// ============================================================================

const state = {
  humanLang: "en",
  codeLang: "cpp",
  currentTopic: "",
  currentCategory: "",
  theme: localStorage.getItem("oopmaster_theme") || "dark",
};

// DOM Elements
const themeToggleBtn = document.getElementById("themeToggle");
const chatForm = document.getElementById("chatForm");
const chatInput = document.getElementById("chatInput");
const chatMessages = document.getElementById("chatMessages");
const humanChips = document.querySelectorAll(".human-chip");

// ----------------------------------------------------------------------------
// Theme Manager
// ----------------------------------------------------------------------------
function syncThemeVideos(theme) {
  const darkVideo = document.getElementById("bgVideoDark");
  const lightVideo = document.getElementById("bgVideoLight");

  if (theme === "light") {
    if (lightVideo) {
      lightVideo.play().catch(() => {});
    }
  } else {
    if (darkVideo) {
      darkVideo.play().catch(() => {});
    }
  }
}

function applyTheme(theme) {
  state.theme = theme;
  document.documentElement.setAttribute("data-theme", theme);
  localStorage.setItem("oopmaster_theme", theme);

  if (themeToggleBtn) {
    const icon = themeToggleBtn.querySelector(".theme-icon");
    const text = themeToggleBtn.querySelector(".theme-text");
    if (theme === "light") {
      if (icon) icon.textContent = "☀️";
      if (text) text.textContent = "Light Mode";
    } else {
      if (icon) icon.textContent = "🌙";
      if (text) text.textContent = "Dark Mode";
    }
  }

  syncThemeVideos(theme);
}

function setupThemeToggle() {
  applyTheme(state.theme);

  // Initialize playback for both videos on first user interaction or load
  const startVideos = () => {
    const darkVideo = document.getElementById("bgVideoDark");
    const lightVideo = document.getElementById("bgVideoLight");
    if (darkVideo) darkVideo.play().catch(() => {});
    if (lightVideo) lightVideo.play().catch(() => {});
    window.removeEventListener("click", startVideos);
  };
  window.addEventListener("click", startVideos, { once: true });

  if (themeToggleBtn) {
    themeToggleBtn.addEventListener("click", () => {
      const nextTheme = state.theme === "dark" ? "light" : "dark";
      applyTheme(nextTheme);
    });
  }
}

// ----------------------------------------------------------------------------
// Language Selector
// ----------------------------------------------------------------------------
function setupLanguagePicker() {
  humanChips.forEach((chip) => {
    chip.addEventListener("click", () => {
      state.humanLang = chip.dataset.humanLang;
      humanChips.forEach((c) => c.classList.toggle("active", c === chip));

      const langNames = { en: "English", hi: "हिन्दी", mr: "मराठी" };
      addChatMessage(
        `🌐 Language switched to <strong>${langNames[state.humanLang]}</strong>. You can ask questions in ${langNames[state.humanLang]} now!`,
        "bot"
      );

      // If we have an active topic, suggest re-explaining in this language
      if (state.currentTopic) {
        sendQuery(`Explain ${state.currentCategory}`, {
          lang: state.humanLang,
          topic: state.currentTopic,
        });
      }
    });
  });
}

// ----------------------------------------------------------------------------
// Chat UI Rendering & Interactive Action Chips
// ----------------------------------------------------------------------------
function addChatMessage(contentHtml, sender = "bot", extraData = null) {
  if (!chatMessages) return;

  const wrapper = document.createElement("div");
  wrapper.className = `message ${sender}-message`;

  const body = document.createElement("div");
  body.innerHTML = contentHtml;
  wrapper.appendChild(body);

  // If the bot returned quick topics or action chips, attach them
  if (sender === "bot" && extraData) {
    const chipContainer = document.createElement("div");
    chipContainer.className = "chat-action-group";

    // 1. Quick Topic Suggestions (on Greeting or Clarification)
    if (extraData.quick_topics && extraData.quick_topics.length > 0) {
      const label = document.createElement("div");
      label.className = "action-label";
      label.textContent = "Explore Topics:";
      chipContainer.appendChild(label);

      const row = document.createElement("div");
      row.className = "action-chips-row";
      extraData.quick_topics.forEach((t) => {
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "action-chip";
        btn.textContent = t.label;
        btn.addEventListener("click", () => {
          sendQuery(t.query || t.label);
        });
        row.appendChild(btn);
      });
      chipContainer.appendChild(row);
    }

    // 2. Interactive Mode & Depth Options (if a topic is active)
    if (extraData.record_id) {
      // Explanation Depth Chips
      const depthLabel = document.createElement("div");
      depthLabel.className = "action-label";
      depthLabel.textContent = "Explanation Depth:";
      chipContainer.appendChild(depthLabel);

      const depthRow = document.createElement("div");
      depthRow.className = "action-chips-row";

      const modes = [
        { label: "👶 Explain like I'm 5", query: "explain like a child", mode: "child" },
        { label: "⚡ Short Summary", query: "make it short", mode: "short" },
        { label: "💡 Simpler Example", query: "simpler example", mode: "simpler" },
        { label: "📖 Detailed Guide", query: "detailed explanation", mode: "detailed" },
      ];

      modes.forEach((m) => {
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "action-chip";
        btn.textContent = m.label;
        btn.addEventListener("click", () => {
          sendQuery(m.query, { mode: m.mode, topic: extraData.record_id });
        });
        depthRow.appendChild(btn);
      });
      chipContainer.appendChild(depthRow);

      // Programming Language Code Chips
      const langLabel = document.createElement("div");
      langLabel.className = "action-label";
      langLabel.textContent = "Code in Language:";
      chipContainer.appendChild(langLabel);

      const langRow = document.createElement("div");
      langRow.className = "action-chips-row";

      const codeLangs = [
        { label: "💻 C++", key: "cpp" },
        { label: "🐍 Python", key: "python" },
        { label: "☕ Java", key: "java" },
        { label: "🔷 C#", key: "csharp" },
        { label: "💛 JavaScript", key: "javascript" },
      ];

      codeLangs.forEach((l) => {
        const btn = document.createElement("button");
        btn.type = "button";
        btn.className = "action-chip lang-chip-btn";
        btn.textContent = l.label;
        btn.addEventListener("click", () => {
          state.codeLang = l.key;
          sendQuery(l.key, { code_lang: l.key, topic: extraData.record_id });
        });
        langRow.appendChild(btn);
      });
      chipContainer.appendChild(langRow);

      // 3. Follow-up Questions
      if (extraData.follow_up_questions && extraData.follow_up_questions.length > 0) {
        const followLabel = document.createElement("div");
        followLabel.className = "action-label";
        followLabel.textContent = "Related Questions:";
        chipContainer.appendChild(followLabel);

        const followRow = document.createElement("div");
        followRow.className = "action-chips-row";
        extraData.follow_up_questions.slice(0, 3).forEach((fq) => {
          const btn = document.createElement("button");
          btn.type = "button";
          btn.className = "action-chip";
          btn.textContent = fq;
          btn.addEventListener("click", () => {
            sendQuery(fq);
          });
          followRow.appendChild(btn);
        });
        chipContainer.appendChild(followRow);
      }
    }

    if (chipContainer.children.length > 0) {
      wrapper.appendChild(chipContainer);
    }
  }

  chatMessages.appendChild(wrapper);
  chatMessages.scrollTo({
    top: chatMessages.scrollHeight,
    behavior: "smooth",
  });
}

// ----------------------------------------------------------------------------
// Capabilities query detector (all possible forms & typos)
// ----------------------------------------------------------------------------
function checkCapabilitiesQuery(text, lang) {
  const s = " " + text.toLowerCase().replace(/[^a-z0-9 ]/g, " ") + " ";
  
  const capKeywords = [
    "what can you do", "what can u do", "what u can do", "wat can you do", "wat can u do",
    "wat u can do", "waht can you do", "waht can u do", "waht u can do", "wht can you do",
    "wht can u do", "what do you do", "what do u do", "what u do", "wat do you do",
    "what can you answer", "what can u answer", "what questions can you answer",
    "what questions can u answer", "what question can you answer", "what question can u answer",
    "what can i ask", "what can i ask you", "what can i ask u", "what do you know", "what do u know",
    "what u know", "what are your capabilities", "your capabilities", "capabilities",
    "what are your features", "features", "how can you help", "how can u help",
    "how can you help me", "how can u help me", "how to use", "how to use this",
    "tell me what you can do", "tell me what u can do", "what is oop master",
    "what can this chatbot do", "what can this bot do", "help me", "help",
    "kya kar sakte ho", "tum kya kar sakte ho", "aap kya kar sakte ho", "kya kya kar sakte ho",
    "kya kar sakti ho", "kya pooch sakte", "kya puch sakte", "kya poochh sakte", "tumhe kya aata hai",
    "aapko kya aata hai", "tum kya jaante ho", "kaise help karoge", "meri kya help karoge",
    "kay karu shaktos", "tu kay karu shaktos", "tumhi kay karu shakta", "kay karu shakta",
    "tula kay yeta", "tumhala kay yeta", "tula kay mahiti", "tumhala kay mahiti",
    "kay vicharu shakto", "mi kay vicharu", "kasli madat", "kashi madat"
  ];

  let matched = capKeywords.some((p) => s.includes(p));

  if (!matched) {
    const hasQ = s.includes("what") || s.includes("waht") || s.includes("wat") || s.includes("wht") || s.includes("kya") || s.includes("kay");
    const hasV = s.includes("can") || s.includes("do") || s.includes("know") || s.includes("ask") || s.includes("help") || s.includes("answer") || s.includes("kar") || s.includes("yeta");
    const hasS = s.includes("you") || s.includes(" u ") || s.includes("tum") || s.includes("aap") || s.includes(" tu ") || s.includes(" i ") || s.includes("me");
    if (hasQ && hasV && hasS) {
      matched = true;
    }
  }

  if (matched) {
    if (lang === "hi") {
      return {
        answer: "मैं Object-Oriented Programming (OOP) से जुड़े इन सभी सवालों के उत्तर दे सकता हूँ:<br><br>" +
                "1. 🏛️ <strong>OOP के 4 स्तंभ</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.<br>" +
                "2. 🏗️ <strong>मूल अवधारणाएँ</strong>: Classes, Objects, Constructors, Destructors, RAII.<br>" +
                "3. 📐 <strong>सॉफ्टवेयर डिज़ाइन</strong>: SOLID सिद्धांत (SRP, OCP, LSP, ISP, DIP), Composition बनाम Inheritance.<br>" +
                "4. ⚡ <strong>23 Design Patterns</strong>: Singleton, Factory, Observer, Strategy, Adapter आदि।<br>" +
                "5. 💻 <strong>कई भाषाओं में कोड</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.<br>" +
                "6. 🧒 <strong>अलग-अलग स्तर</strong>: आप 'छोटा उत्तर दो', '5 साल के बच्चे की तरह समझाओ', या 'सरल उदाहरण दो' कह सकते हैं!<br>" +
                "7. 💬 <strong>सामान्य बातचीत</strong>: Hello, Kaise ho, Umar kya hai, आदि भी पूछ सकते हैं!<br>" +
                "8. 🌐 <strong>3 भाषाएँ</strong>: English, हिन्दी, और मराठी।",
        quick_topics: [
          { label: "🏛️ 4 Pillars of OOP", query: "What are the four pillars of OOP?" },
          { label: "🏗️ Classes & Objects", query: "What is a class and object?" },
          { label: "🧬 Inheritance", query: "What is inheritance?" },
          { label: "🎭 Polymorphism", query: "What is polymorphism?" },
          { label: "📐 SOLID Principles", query: "What are SOLID principles?" }
        ]
      };
    } else if (lang === "mr") {
      return {
        answer: "मी Object-Oriented Programming संदर्भातील खालील सर्व प्रश्नांची उत्तरे देऊ शकतो:<br><br>" +
                "1. 🏛️ <strong>OOP चे 4 मुख्य स्तंभ</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.<br>" +
                "2. 🏗️ <strong>मूलभूत संकल्पना</strong>: Classes, Objects, Constructors, Destructors, RAII.<br>" +
                "3. 📐 <strong>सॉफ्टवेअर डिझाईन</strong>: SOLID नियम, Composition विरूद्ध Inheritance.<br>" +
                "4. ⚡ <strong>23 Design Patterns</strong>: Singleton, Factory, Observer, Strategy इत्यादी.<br>" +
                "5. 💻 <strong>विविध भाषांमधील कोड</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.<br>" +
                "6. 🧒 <strong>विविध पातळ्यांवर स्पष्टीकरण</strong>: आपण 'थोडक्यात सांगा', 'लहान मुलासारखे समजावून सांगा', किंवा 'सोपे उदाहरण द्या' विचारू शकता!<br>" +
                "7. 💬 <strong>सामान्य संवाद</strong>: Hello, Kasa ahes, Vay kay ahe, इत्यादी विचारू शकता!<br>" +
                "8. 🌐 <strong>3 भाषा</strong>: English, हिन्दी, आणि मराठी।",
        quick_topics: [
          { label: "🏛️ 4 Pillars of OOP", query: "What are the four pillars of OOP?" },
          { label: "🏗️ Classes & Objects", query: "What is a class and object?" },
          { label: "🧬 Inheritance", query: "What is inheritance?" },
          { label: "🎭 Polymorphism", query: "What is polymorphism?" },
          { label: "📐 SOLID Principles", query: "What are SOLID principles?" }
        ]
      };
    } else {
      return {
        answer: "Here is everything I can answer and do for you in <strong>OOP Master</strong>:<br><br>" +
                "1. 🏛️ <strong>The 4 Pillars of OOP</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.<br>" +
                "2. 🏗️ <strong>Core Building Blocks</strong>: Classes, Objects, Constructors, Destructors, RAII, Copy/Move semantics.<br>" +
                "3. 📐 <strong>Software Design Principles</strong>: SOLID Principles (SRP, OCP, LSP, ISP, DIP), Object Relationships (Composition vs Inheritance, Association, Aggregation).<br>" +
                "4. ⚡ <strong>23 GoF Design Patterns</strong>: Creational (Singleton, Factory, Builder), Structural (Adapter, Decorator, Facade), Behavioral (Observer, Strategy, Command).<br>" +
                "5. 💾 <strong>Memory & Lifetimes</strong>: Stack vs Heap, Smart Pointers, Virtual Destructors, Garbage Collection, Object Slicing.<br>" +
                "6. 💻 <strong>Multi-Language Code</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.<br>" +
                "7. 🧒 <strong>Adaptive Explanations</strong>: Ask me to <em>'make it short'</em>, <em>'explain like I am 5'</em>, or <em>'give a simpler example'</em>.<br>" +
                "8. 💬 <strong>General Chitchat</strong>: Say 'hello', ask 'how are you', 'what is your age', or 'do you know me' anytime!<br>" +
                "9. 🌐 <strong>Multilingual</strong>: Ask in English, हिन्दी (Hindi), or मराठी (Marathi)!",
        quick_topics: [
          { label: "🏛️ 4 Pillars of OOP", query: "What are the four pillars of OOP?" },
          { label: "🏗️ Classes & Objects", query: "What is a class and object?" },
          { label: "🧬 Inheritance", query: "What is inheritance?" },
          { label: "🎭 Polymorphism", query: "What is polymorphism?" },
          { label: "📐 SOLID Principles", query: "What are SOLID principles?" }
        ]
      };
    }
  }

  return null;
}

// ----------------------------------------------------------------------------
// Query Execution to C++ Native Backend
// ----------------------------------------------------------------------------
async function sendQuery(queryText, options = {}) {
  const q = queryText.trim();
  if (!q) return;

  // Add user bubble
  addChatMessage(escapeHtml(q), "user");

  const humanLang = options.lang || state.humanLang || "en";
  const codeLang = options.code_lang || state.codeLang || "cpp";
  const topic = options.topic || state.currentTopic || "";
  const mode = options.mode || "medium";

  // Check capabilities query in all possible forms upfront
  const capData = checkCapabilitiesQuery(q, humanLang);
  if (capData) {
    addChatMessage(capData.answer, "bot", capData);
    return;
  }

  try {
    const url = `/api/chat?q=${encodeURIComponent(q)}&lang=${humanLang}&mode=${mode}&code_lang=${codeLang}&topic=${encodeURIComponent(topic)}`;
    const res = await fetch(url);

    if (res.ok) {
      const data = await res.json();

      // Update current topic context
      if (data.record_id) {
        state.currentTopic = data.record_id;
        state.currentCategory = data.category || "";
      }

      let formatted = data.answer || "";
      formatted = formatted.replace(/\n/g, "<br>");

      // Append code snippet if provided
      if (data.code_simple && data.code_simple.trim().length > 0) {
        formatted += `<br><br><div style="font-size: 0.8rem; font-weight: 600; color: var(--primary); margin-bottom: 4px;">Code Example (${data.code_lang || codeLang}):</div><pre style="background: rgba(0,0,0,0.38); padding: 12px; border-radius: 8px; overflow-x: auto; font-family: 'JetBrains Mono', monospace; font-size: 0.88rem; border: 1px solid rgba(158,180,214,0.18);"><code>${escapeHtml(data.code_simple)}</code></pre>`;
      }

      // Add bot reply with interactive chips
      addChatMessage(formatted, "bot", data);
      return;
    }
  } catch (err) {
    console.log("C++ native server offline, using client fallback:", err);
  }

  // Fallback
  addChatMessage(
    "I'm here to help with Object-Oriented Programming! Try asking: <em>'What is inheritance?'</em>, <em>'Explain polymorphism like I'm 5'</em>, or <em>'Show encapsulation in Python'</em>.",
    "bot",
    {
      quick_topics: [
        { label: "Classes & Objects", query: "What is a class and object?" },
        { label: "Inheritance", query: "What is inheritance?" },
        { label: "Polymorphism", query: "What is polymorphism?" },
        { label: "Encapsulation", query: "What is encapsulation?" },
      ],
    }
  );
}

function handleChatSubmit(event) {
  event.preventDefault();
  const question = chatInput.value.trim();
  if (!question) return;

  chatInput.value = "";
  sendQuery(question);
}

function escapeHtml(str) {
  return str
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

// ----------------------------------------------------------------------------
// Initial Setup
// ----------------------------------------------------------------------------
function init() {
  setupThemeToggle();
  setupLanguagePicker();
  if (chatForm) {
    chatForm.addEventListener("submit", handleChatSubmit);
  }

  // Initial welcome chips
  const firstBotMessage = document.querySelector(".bot-message");
  if (firstBotMessage) {
    const chipContainer = document.createElement("div");
    chipContainer.className = "chat-action-group";

    const label = document.createElement("div");
    label.className = "action-label";
    label.textContent = "Start with a topic:";
    chipContainer.appendChild(label);

    const row = document.createElement("div");
    row.className = "action-chips-row";
    const initialTopics = [
      { label: "🏗️ Classes & Objects", query: "What is a class and object?" },
      { label: "🧬 Inheritance", query: "What is inheritance?" },
      { label: "🎭 Polymorphism", query: "What is polymorphism?" },
      { label: "📦 Encapsulation", query: "What is encapsulation?" },
      { label: "🛡️ Abstraction", query: "What is abstraction?" },
      { label: "⚙️ Constructors", query: "What is a constructor?" },
      { label: "📐 SOLID Principles", query: "What are SOLID principles?" },
    ];

    initialTopics.forEach((t) => {
      const btn = document.createElement("button");
      btn.type = "button";
      btn.className = "action-chip";
      btn.textContent = t.label;
      btn.addEventListener("click", () => {
        sendQuery(t.query);
      });
      row.appendChild(btn);
    });

    chipContainer.appendChild(row);
    firstBotMessage.appendChild(chipContainer);
  }
}

document.addEventListener("DOMContentLoaded", init);
