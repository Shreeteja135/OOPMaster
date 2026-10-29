// ============================================================================
// OOP MASTER: Vercel Cloud Serverless Chat Engine
// Matches the C++ retrieval engine with subtopic boosting, context awareness,
// typo tolerance, and multilingual support (EN, HI, MR).
// ============================================================================

const kbData = require('../data/kb_data.json');

function toLower(str) {
  return (str || '').toLowerCase();
}

function trim(str) {
  return (str || '').trim();
}

// Levenshtein distance for fuzzy typo matching
function levenshteinDistance(s1, s2) {
  const len1 = s1.length;
  const len2 = s2.length;
  if (len1 === 0) return len2;
  if (len2 === 0) return len1;

  const d = Array.from({ length: len1 + 1 }, () => new Array(len2 + 1).fill(0));
  for (let i = 0; i <= len1; i++) d[i][0] = i;
  for (let j = 0; j <= len2; j++) d[0][j] = j;

  for (let i = 1; i <= len1; i++) {
    for (let j = 1; j <= len2; j++) {
      const cost = s1[i - 1] === s2[j - 1] ? 0 : 1;
      d[i][j] = Math.min(
        d[i - 1][j] + 1,
        d[i][j - 1] + 1,
        d[i - 1][j - 1] + cost
      );
    }
  }
  return d[len1][len2];
}

// Normalize common typos and transliterations
function normalizeTypos(input) {
  let s = toLower(input);
  const typoList = [
    ['polymorfism', 'polymorphism'],
    ['polymorphsm', 'polymorphism'],
    ['polymorpism', 'polymorphism'],
    ['inheretance', 'inheritance'],
    ['inheratence', 'inheritance'],
    ['inharitance', 'inheritance'],
    ['inhertance', 'inheritance'],
    ['consctructor', 'constructor'],
    ['constructer', 'constructor'],
    ['constractor', 'constructor'],
    ['distructor', 'destructor'],
    ['destructer', 'destructor'],
    ['destructur', 'destructor'],
    ['encapsulashun', 'encapsulation'],
    ['encapsulaton', 'encapsulation'],
    ['encapslation', 'encapsulation'],
    ['abstracktion', 'abstraction'],
    ['abstruction', 'abstraction'],
    ['abstrack', 'abstraction'],
    ['interfce', 'interface'],
    ['intreface', 'interface'],
    ['soild', 'solid'],
    ['solide', 'solid'],
    ['singlton', 'singleton'],
    ['singletun', 'singleton'],
    ['memry', 'memory'],
    ['memori', 'memory'],
    ['virasat', 'inheritance'],
    ['varsa', 'inheritance'],
    ['bahuroopta', 'polymorphism'],
    ['bahuvidhikta', 'polymorphism'],
    ['prachhadan', 'encapsulation'],
    ['amoortikaran', 'abstraction'],
    ['varg', 'class']
  ];

  for (const [mis, cor] of typoList) {
    if (s.includes(mis)) {
      s = s.split(mis).join(cor);
    }
  }
  return s;
}

// Multi-language code snippets
function getMultiLangCode(recordId, lang) {
  const l = toLower(lang);

  if (recordId === 'OOP_CPP_CLASS_OBJECT_001') {
    if (l === 'python' || l === 'py') {
      return `class Student:\n    def __init__(self, name, roll_no):\n        self.name = name          # Attribute\n        self.roll_no = roll_no\n\n    def display(self):\n        print(f"Student: {self.name}, Roll: {self.roll_no}")\n\n# Object creation\ns1 = Student("Aarav", 101)\ns1.display()`;
    } else if (l === 'java') {
      return `class Student {\n    String name;\n    int rollNo;\n\n    Student(String name, int rollNo) {\n        this.name = name;\n        this.rollNo = rollNo;\n    }\n\n    void display() {\n        System.out.println("Student: " + name + ", Roll: " + rollNo);\n    }\n}\n\npublic class Main {\n    public static void main(String[] args) {\n        Student s1 = new Student("Aarav", 101);\n        s1.display();\n    }\n}`;
    } else if (l === 'csharp' || l === 'c#') {
      return `using System;\n\nclass Student {\n    public string Name { get; set; }\n    public int RollNo { get; set; }\n\n    public void Display() {\n        Console.WriteLine($"Student: {Name}, Roll: {RollNo}");\n    }\n}\n\nclass Program {\n    static void Main() {\n        Student s1 = new Student { Name = "Aarav", RollNo = 101 };\n        s1.Display();\n    }\n}`;
    } else if (l === 'javascript' || l === 'js' || l === 'typescript' || l === 'ts') {
      return `class Student {\n    constructor(name, rollNo) {\n        this.name = name;\n        this.rollNo = rollNo;\n    }\n\n    display() {\n        console.log(\`Student: \${this.name}, Roll: \${this.rollNo}\`);\n    }\n}\n\nconst s1 = new Student("Aarav", 101);\ns1.display();`;
    }
  } else if (recordId === 'OOP_CPP_INHERITANCE_001') {
    if (l === 'python' || l === 'py') {
      return `class Animal:\n    def eat(self):\n        print("Animal is eating...")\n\nclass Dog(Animal):  # Dog inherits Animal\n    def bark(self):\n        print("Dog is barking: Woof!")\n\nd = Dog()\nd.eat()   # Inherited from Animal\nd.bark()  # Defined in Dog`;
    } else if (l === 'java') {
      return `class Animal {\n    void eat() {\n        System.out.println("Animal is eating...");\n    }\n}\n\nclass Dog extends Animal {  // Dog inherits Animal\n    void bark() {\n        System.out.println("Dog is barking: Woof!");\n    }\n}\n\npublic class Main {\n    public static void main(String[] args) {\n        Dog d = new Dog();\n        d.eat();   // Inherited\n        d.bark();  // Derived method\n    }\n}`;
    } else if (l === 'csharp' || l === 'c#') {
      return `using System;\n\nclass Animal {\n    public void Eat() => Console.WriteLine("Animal is eating...");\n}\n\nclass Dog : Animal {\n    public void Bark() => Console.WriteLine("Dog is barking: Woof!");\n}\n\nclass Program {\n    static void Main() {\n        Dog d = new Dog();\n        d.Eat();\n        d.Bark();\n    }\n}`;
    } else if (l === 'javascript' || l === 'js' || l === 'typescript' || l === 'ts') {
      return `class Animal {\n    eat() {\n        console.log("Animal is eating...");\n    }\n}\n\nclass Dog extends Animal {\n    bark() {\n        console.log("Dog is barking: Woof!");\n    }\n}\n\nconst d = new Dog();\nd.eat();\nd.bark();`;
    }
  } else if (recordId === 'OOP_CORE_POLYMORPHISM_001') {
    if (l === 'python' || l === 'py') {
      return `class Shape:\n    def draw(self):\n        print("Drawing generic shape")\n\nclass Circle(Shape):\n    def draw(self):\n        print("Drawing a Circle ⭕")\n\nclass Square(Shape):\n    def draw(self):\n        print("Drawing a Square ⬛")\n\n# Polymorphic call\nshapes = [Circle(), Square()]\nfor s in shapes:\n    s.draw()`;
    } else if (l === 'java') {
      return `class Shape {\n    void draw() { System.out.println("Drawing shape"); }\n}\nclass Circle extends Shape {\n    @Override void draw() { System.out.println("Drawing Circle ⭕"); }\n}\nclass Square extends Shape {\n    @Override void draw() { System.out.println("Drawing Square ⬛"); }\n}\n\npublic class Main {\n    public static void main(String[] args) {\n        Shape[] shapes = { new Circle(), new Square() };\n        for (Shape s : shapes) s.draw();\n    }\n}`;
    } else if (l === 'csharp' || l === 'c#') {
      return `using System;\n\nclass Shape {\n    public virtual void Draw() => Console.WriteLine("Drawing shape");\n}\nclass Circle : Shape {\n    public override void Draw() => Console.WriteLine("Drawing Circle ⭕");\n}\nclass Square : Shape {\n    public override void Draw() => Console.WriteLine("Drawing Square ⬛");\n}\n\nclass Program {\n    static void Main() {\n        Shape[] shapes = { new Circle(), new Square() };\n        foreach (var s in shapes) s.Draw();\n    }\n}`;
    } else if (l === 'javascript' || l === 'js') {
      return `class Shape { draw() { console.log("Drawing shape"); } }\nclass Circle extends Shape { draw() { console.log("Drawing Circle ⭕"); } }\nclass Square extends Shape { draw() { console.log("Drawing Square ⬛"); } }\n\n[new Circle(), new Square()].forEach(s => s.draw());`;
    }
  } else if (recordId === 'OOP_CORE_ENCAPSULATION_001') {
    if (l === 'python' || l === 'py') {
      return `class BankAccount:\n    def __init__(self, initial_balance):\n        self.__balance = initial_balance  # Private attribute\n\n    def deposit(self, amount):\n        if amount > 0: self.__balance += amount\n\n    def get_balance(self):\n        return self.__balance\n\naccount = BankAccount(1000)\naccount.deposit(500)\nprint("Balance:", account.get_balance())`;
    } else if (l === 'java') {
      return `class BankAccount {\n    private double balance; // Hidden data\n\n    public BankAccount(double b) { this.balance = b; }\n    public void deposit(double amt) { if (amt > 0) balance += amt; }\n    public double getBalance() { return balance; }\n}\n\npublic class Main {\n    public static void main(String[] args) {\n        BankAccount acc = new BankAccount(1000);\n        acc.deposit(500);\n        System.out.println("Balance: " + acc.getBalance());\n    }\n}`;
    } else if (l === 'csharp' || l === 'c#') {
      return `using System;\n\nclass BankAccount {\n    private double _balance;\n    public BankAccount(double b) => _balance = b;\n    public void Deposit(double amt) { if (amt > 0) _balance += amt; }\n    public double Balance => _balance;\n}\n\nclass Program {\n    static void Main() {\n        var acc = new BankAccount(1000);\n        acc.Deposit(500);\n        Console.WriteLine($"Balance: {acc.Balance}");\n    }\n}`;
    } else if (l === 'javascript' || l === 'js') {
      return `class BankAccount {\n    #balance = 0; // Private field\n    constructor(b) { this.#balance = b; }\n    deposit(amt) { if (amt > 0) this.#balance += amt; }\n    getBalance() { return this.#balance; }\n}\n\nconst acc = new BankAccount(1000);\nacc.deposit(500);\nconsole.log("Balance:", acc.getBalance());`;
    }
  }

  return '';
}

// Calculate match score with subtopic boosting
function scoreRecord(rec, query) {
  let score = 0;
  const qLower = normalizeTypos(toLower(query));
  const recQLower = toLower(rec.question || '');

  // Exact matches
  if (qLower === recQLower) score += 120;
  else if (recQLower.includes(qLower)) score += 60;

  for (const alt of rec.alternate_questions || []) {
    const altLower = toLower(alt);
    if (qLower === altLower) score += 90;
    else if (qLower.includes(altLower) || altLower.includes(qLower)) score += 45;
  }

  for (const term of rec.search_terms || []) {
    const termLower = toLower(term);
    if (qLower.includes(termLower)) {
      score += 35;
    } else if (qLower.length > 4 && termLower.length > 4) {
      if (levenshteinDistance(qLower, termLower) <= 2) score += 30;
    }
  }

  const catLower = toLower(rec.category || '');
  if (qLower.includes(catLower)) score += 20;

  const subcatLower = toLower(rec.subcategory || '');
  if (subcatLower && qLower.includes(subcatLower)) {
    score += 150;
  }

  // Specific subtopic keyword boosting
  const boostIfMatch = (token, targetRecId, bonus) => {
    if (qLower.includes(token) && (rec.record_id || '').includes(targetRecId)) {
      score += bonus;
    }
  };

  boostIfMatch('multilevel', 'MULTILEVEL', 250);
  boostIfMatch('multiple inheritance', 'MULTIPLE', 250);
  boostIfMatch('hierarchical', 'HIERARCHICAL', 250);
  boostIfMatch('hybrid', 'HYBRID', 250);
  boostIfMatch('diamond', 'HYBRID', 250);
  boostIfMatch('virtual inheritance', 'HYBRID', 250);
  boostIfMatch('virtual base', 'HYBRID', 250);
  boostIfMatch('private inheritance', 'ACCESS_MODES', 280);
  boostIfMatch('public inheritance', 'ACCESS_MODES', 220);
  boostIfMatch('protected inheritance', 'ACCESS_MODES', 220);
  boostIfMatch('access mode', 'ACCESS_MODES', 220);
  boostIfMatch('runtime polymorphism', 'RUNTIME', 280);
  boostIfMatch('dynamic polymorphism', 'RUNTIME', 280);
  boostIfMatch('dynamic binding', 'RUNTIME', 220);
  boostIfMatch('late binding', 'RUNTIME', 220);
  boostIfMatch('pure virtual', 'PURE_VIRTUAL', 280);
  boostIfMatch('abstract class', 'PURE_VIRTUAL', 180);
  if (!qLower.includes('pure virtual')) {
    boostIfMatch('virtual function', 'VIRTUAL_FUNCS', 220);
    boostIfMatch('virtual functions', 'VIRTUAL_FUNCS', 220);
  }
  boostIfMatch('vtable', 'VTABLES', 280);
  boostIfMatch('vtables', 'VTABLES', 280);
  boostIfMatch('vptr', 'VTABLES', 280);
  boostIfMatch('virtual table', 'VTABLES', 280);
  boostIfMatch('virtual method table', 'VTABLES', 280);
  boostIfMatch('compile-time polymorphism', 'COMPILE_TIME', 280);
  boostIfMatch('compile time polymorphism', 'COMPILE_TIME', 280);
  boostIfMatch('function overloading', 'COMPILE_TIME', 200);
  boostIfMatch('operator overloading', 'COMPILE_TIME', 200);
  boostIfMatch('access specifier', 'ACCESS_SPECIFIERS', 220);
  boostIfMatch('accessibility', 'ACCESS_SPECIFIERS', 220);
  boostIfMatch('friend function', 'FRIEND', 280);
  boostIfMatch('friend class', 'FRIEND', 280);
  boostIfMatch('static member', 'STATIC', 280);
  boostIfMatch('static data member', 'STATIC', 280);
  boostIfMatch('static function', 'STATIC', 220);
  boostIfMatch('copy constructor', 'CONSTRUCTOR_TYPES', 280);
  boostIfMatch('move constructor', 'CONSTRUCTOR_TYPES', 280);
  boostIfMatch('types of constructor', 'CONSTRUCTOR_TYPES', 280);
  boostIfMatch('types of constructors', 'CONSTRUCTOR_TYPES', 280);
  boostIfMatch('constructor types', 'CONSTRUCTOR_TYPES', 280);
  boostIfMatch('virtual destructor', 'VIRTUAL_DESTRUCTOR', 280);
  boostIfMatch('virtual destructors', 'VIRTUAL_DESTRUCTOR', 280);

  // Dampen generic overview records when a specific subtopic is queried
  if (rec.record_id === 'OOP_CPP_INHERITANCE_001') {
    if (
      qLower.includes('multilevel') ||
      qLower.includes('multiple') ||
      qLower.includes('hierarchical') ||
      qLower.includes('hybrid') ||
      qLower.includes('diamond') ||
      qLower.includes('private inheritance') ||
      qLower.includes('protected inheritance') ||
      qLower.includes('access mode')
    ) {
      score -= 200;
    }
  }
  if (rec.record_id === 'OOP_CORE_POLYMORPHISM_001') {
    if (
      qLower.includes('runtime') ||
      qLower.includes('dynamic') ||
      qLower.includes('virtual function') ||
      qLower.includes('pure virtual') ||
      qLower.includes('vtable') ||
      qLower.includes('vptr') ||
      qLower.includes('compile-time') ||
      qLower.includes('compile time') ||
      qLower.includes('overloading')
    ) {
      score -= 200;
    }
  }
  if (rec.record_id === 'OOP_CPP_CONSTRUCTOR_001') {
    if (
      qLower.includes('copy constructor') ||
      qLower.includes('move constructor') ||
      qLower.includes('types of constructor') ||
      qLower.includes('constructor types') ||
      qLower.includes('virtual destructor') ||
      qLower.includes('destructor')
    ) {
      score -= 200;
    }
  }

  return score;
}

// Chitchat handler
function handleGenericChat(rawQuery, curHuman) {
  let s = ' ' + trim(toLower(rawQuery)) + ' ';
  s = s.replace(/[?!.,']/g, ' ');

  const oopTerms = [
    'polymorphism', 'inheritance', 'class', 'classes', 'object', 'objects',
    'virtual', 'vtable', 'vptr', 'abstract', 'interface', 'encapsulation',
    'abstraction', 'constructor', 'destructor', 'specifier', 'friend', 'static',
    'solid', 'pattern', 'patterns', 'overload', 'override', 'binding', 'dispatch',
    'diamond', 'multilevel', 'hierarchical', 'hybrid', 'private', 'protected', 'public'
  ];
  for (const term of oopTerms) {
    if (s.includes(term)) return null;
  }

  const hasWord = (patterns) => {
    for (const p of patterns) {
      const spaced = ' ' + p + ' ';
      if (p.length <= 3) {
        if (s.includes(spaced) || s.trim() === p) return true;
      } else {
        if (s.includes(spaced) || s.includes(p)) return true;
      }
    }
    return false;
  };

  // How are you
  if (hasWord(['how are', 'hoaw are', 'how r u', 'how do you', 'kaise ho', 'kasa ahes', 'kashi ahes', 'kase ahat', 'kya haal'])) {
    let answer = 'I am doing fantastic and running at peak performance on Vercel Serverless Engine! 🚀\n\nHow is your coding journey going today? Which Object-Oriented Programming concept shall we tackle?';
    if (curHuman === 'hi') {
      answer = 'मैं बहुत बढ़िया हूँ और पूरी गति से चल रहा हूँ! 🚀\n\nआपकी कोडिंग कैसी चल रही है? आज आप कौन सा OOP टॉपिक सीखना चाहते हैं?';
    } else if (curHuman === 'mr') {
      answer = 'मी अगदी मजेत आहे आणि वेगाने कार्यरत आहे! 🚀\n\nआपला कोडिंगचा अभ्यास कसा चालू आहे? आज कोणती OOP संकल्पना शिकायची आहे?';
    }
    return {
      status: 'general',
      category: 'Greeting',
      answer,
      quick_topics: [
        { label: '🏗️ Classes & Objects', query: 'What is a class and object?' },
        { label: '🧬 Inheritance', query: 'What is inheritance?' },
        { label: '🎭 Polymorphism', query: 'What is polymorphism?' },
        { label: '📦 Encapsulation', query: 'What is encapsulation?' }
      ]
    };
  }

  // Age
  if (hasWord(['age', 'umar', 'vay', 'how old'])) {
    let answer = 'In human years, I was compiled just recently! 👶\n\nIn computer science terms, however, my knowledge base spans over 55+ years of Object-Oriented Programming history—from Simula 67 to modern C++20, Python 3.12, and Java 21! ⏳';
    if (curHuman === 'hi') {
      answer = 'मानवीय वर्षों में तो मेरा जन्म हाल ही में हुआ है! 👶\n\nलेकिन ज्ञान के मामले में, मेरी मेमोरी में 1967 (Simula 67) से लेकर आधुनिक C++20 और Python 3.12 तक के पूरे OOP इतिहास का गहरा ज्ञान समाहित है! ⏳';
    } else if (curHuman === 'mr') {
      answer = 'मानवी वयाच्या दृष्टीने मी नुकताच कंपाइल झालो आहे! 👶\n\nपरंतु ज्ञानाच्या बाबतीत, माझ्या मेमरीमध्ये 1967 पासूनच्या सर्व OOP संकल्पनांचे सखोल ज्ञान आहे! ⏳';
    }
    return {
      status: 'general',
      category: 'About Me',
      answer,
      quick_topics: [
        { label: '⏳ History of OOP', query: 'What is the history of object-oriented programming?' },
        { label: '🏗️ Classes & Objects', query: 'What is a class and object?' },
        { label: '📐 SOLID Principles', query: 'What are SOLID principles?' }
      ]
    };
  }

  // Do you know me
  if (hasWord(['know me', 'know u', 'who am i', 'jaante ho', 'olkhtos ka', 'olkhtat ka'])) {
    let answer = 'You are an ambitious developer on an exciting journey to master Object-Oriented Programming! 🌟\n\nTo guarantee your privacy, I don\'t store personal profiling data, but I recognize your dedication to writing clean, professional code. I\'m here whenever you need an explanation or code snippet! 😊';
    if (curHuman === 'hi') {
      answer = 'आप एक होनहार और जिज्ञासु सॉफ़्टवेयर डेवलपर हैं जो Object-Oriented Programming में महारत हासिल कर रहे हैं! 🌟\n\nआपकी पूर्ण निजता सुरक्षित रखने के लिए मैं कोई व्यक्तिगत डेटा स्टोर नहीं करता, लेकिन जब भी आप कोडिंग करेंगे, मैं आपका सबसे वफ़ादार मार्गदर्शक बना रहूँगा। 😊';
    } else if (curHuman === 'mr') {
      answer = 'आपण एक हुशार आणि उत्साही सॉफ्टवेअर डेवलपर आहात जे Object-Oriented Programming शिकत आहात! 🌟\n\nआपल्या गोपनीयतेच्या रक्षणासाठी मी वैयक्तिक माहिती साठवत नाही, पण आपल्या कोडिंग प्रवासात मदत करण्यासाठी मी नेहमी तयार आहे. 😊';
    }
    return {
      status: 'general',
      category: 'About You',
      answer,
      quick_topics: [
        { label: '🏗️ Classes & Objects', query: 'What is a class and object?' },
        { label: '🧬 Inheritance', query: 'What is inheritance?' },
        { label: '🎭 Polymorphism', query: 'What is polymorphism?' }
      ]
    };
  }

  // Standard Greetings
  const greetings = ['hi', 'hello', 'hey', 'hola', 'namaste', 'pranam', 'kasa ahes', 'kasa kay', 'good morning', 'good afternoon', 'good evening', 'hii', 'heyy', 'namaskar', 'start'];
  if (hasWord(greetings)) {
    let answer = 'Hello! 👋 I am <strong>OOP Master</strong>, your dedicated Object-Oriented Programming AI tutor.\n\nWhich OOP concept would you like to explore today? Tap a topic below or type any question:';
    if (curHuman === 'hi') {
      answer = 'नमस्ते! 👋 मैं <strong>OOP Master</strong> हूँ, आपका Object-Oriented Programming AI शिक्षक।\n\nआप OOP की कौन सी अवधारणा सीखना चाहते हैं? नीचे दिए गए किसी विषय पर क्लिक करें या अपना प्रश्न पूछें:';
    } else if (curHuman === 'mr') {
      answer = 'नमस्कार! 👋 मी <strong>OOP Master</strong> आहे, आपला Object-Oriented Programming AI मार्गदर्शक।\n\nआपण OOP ची कोणती संकल्पना शिकू इच्छिता? खालीलपैकी एका विषयावर टॅप करा किंवा आपला प्रश्न विचारा:';
    }
    return {
      status: 'greeting',
      category: 'Welcome',
      answer,
      quick_topics: [
        { label: '🏗️ Classes & Objects', query: 'What is a class and object?' },
        { label: '🧬 Inheritance', query: 'What is inheritance?' },
        { label: '🎭 Polymorphism', query: 'What is polymorphism?' },
        { label: '📦 Encapsulation', query: 'What is encapsulation?' },
        { label: '🛡️ Abstraction', query: 'What is abstraction?' },
        { label: '⚙️ Constructors', query: 'What is a constructor?' },
        { label: '📐 SOLID Principles', query: 'What are SOLID principles?' }
      ]
    };
  }

  return null;
}

// Detect language switch keyword
function detectProgLangOnly(text) {
  const s = ' ' + toLower(text) + ' ';
  if (s.includes(' c++ ') || s.includes(' cpp ')) return 'cpp';
  if (s.includes(' python ') || s.includes(' py ')) return 'python';
  if (s.includes(' java ') && !s.includes('javascript')) return 'java';
  if (s.includes(' c# ') || s.includes(' csharp ')) return 'csharp';
  if (s.includes(' javascript ') || s.includes(' js ') || s.includes(' typescript ') || s.includes(' ts ')) return 'javascript';
  return '';
}

// Main serverless handler
module.exports = function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');
  res.setHeader('Content-Type', 'application/json; charset=utf-8');

  if (req.method === 'OPTIONS') {
    res.status(200).end();
    return;
  }

  const query = req.query.q || '';
  let humanLang = req.query.lang || 'en';
  let mode = req.query.mode || 'medium';
  let progLang = req.query.code_lang || 'cpp';
  const currentTopic = req.query.topic || '';

  const userQuery = trim(query);
  const qLowerGlobal = toLower(userQuery);

  let dataset = kbData.en || [];
  let curHuman = 'en';

  if (humanLang === 'hi' || humanLang === 'hindi') {
    dataset = kbData.hi || [];
    curHuman = 'hi';
  } else if (humanLang === 'mr' || humanLang === 'marathi') {
    dataset = kbData.mr || [];
    curHuman = 'mr';
  }

  // Dynamic language detection from question text
  if (qLowerGlobal.includes('in marathi') || qLowerGlobal.includes('marathi madhye') || qLowerGlobal.includes('marathit')) {
    dataset = kbData.mr || [];
    curHuman = 'mr';
  } else if (qLowerGlobal.includes('in hindi') || qLowerGlobal.includes('hindi mein') || qLowerGlobal.includes('hindi me')) {
    dataset = kbData.hi || [];
    curHuman = 'hi';
  } else if (qLowerGlobal.includes('in english')) {
    dataset = kbData.en || [];
    curHuman = 'en';
  }

  // Dynamic programming language detection
  if (qLowerGlobal.includes('c++') || qLowerGlobal.includes('in cpp')) progLang = 'cpp';
  else if (qLowerGlobal.includes('python')) progLang = 'python';
  else if (qLowerGlobal.includes('java') && !qLowerGlobal.includes('javascript')) progLang = 'java';
  else if (qLowerGlobal.includes('c#') || qLowerGlobal.includes('csharp')) progLang = 'csharp';
  else if (qLowerGlobal.includes('javascript') || qLowerGlobal.includes('typescript')) progLang = 'javascript';

  // 1. Check general chitchat
  const generalReply = handleGenericChat(userQuery, curHuman);
  if (generalReply) {
    res.status(200).json(generalReply);
    return;
  }

  // 2. Contextual Topic modifiers
  let targetRec = null;
  if (currentTopic) {
    targetRec = dataset.find((r) => r.record_id === currentTopic) || null;
    if (!targetRec) {
      targetRec = (kbData.en || []).find((r) => r.record_id === currentTopic) || null;
    }
  }

  // If query is just a programming language name
  const detectedLang = detectProgLangOnly(userQuery);
  if (detectedLang) {
    progLang = detectedLang;
    if (targetRec) {
      let codeSnippet = getMultiLangCode(targetRec.record_id, progLang);
      if (!codeSnippet) codeSnippet = targetRec.example_simple_code || '';
      res.status(200).json({
        status: 'success',
        record_id: targetRec.record_id,
        category: targetRec.category,
        question: targetRec.question,
        answer: `Here is the <strong>${progLang}</strong> implementation for <strong>${targetRec.category}</strong>:`,
        code_simple: codeSnippet,
        expected_output: targetRec.expected_output || '',
        child_analogy: targetRec.level_1_child || '',
        real_world: targetRec.example_real_world || ''
      });
      return;
    }
  }

  // Contextual modifiers with an active topic
  if (targetRec) {
    if (qLowerGlobal.includes('again') || qLowerGlobal.includes('repeat') || qLowerGlobal.includes('once more') || qLowerGlobal.includes('phir se') || qLowerGlobal.includes('punha')) {
      let ans = `🔄 <strong>Re-explaining ${targetRec.category} in depth</strong>:\n\n${targetRec.answer_detailed || targetRec.answer_medium}`;
      if (curHuman === 'hi') ans = `🔄 <strong>${targetRec.category} का पुनः स्पष्टीकरण</strong>:\n\n${targetRec.answer_detailed || targetRec.answer_medium}`;
      else if (curHuman === 'mr') ans = `🔄 <strong>${targetRec.category} चे पुन्हा स्पष्टीकरण</strong>:\n\n${targetRec.answer_detailed || targetRec.answer_medium}`;
      res.status(200).json({
        status: 'success',
        record_id: targetRec.record_id,
        category: targetRec.category,
        question: targetRec.question,
        answer: ans,
        code_simple: targetRec.example_simple_code || '',
        child_analogy: targetRec.level_1_child || '',
        real_world: targetRec.example_real_world || ''
      });
      return;
    }

    if (qLowerGlobal.includes('short') || mode === 'short') {
      res.status(200).json({
        status: 'success',
        record_id: targetRec.record_id,
        category: targetRec.category,
        question: targetRec.question,
        answer: `⚡ <strong>Short Summary (${targetRec.category})</strong>: ${targetRec.answer_short}`,
        child_analogy: targetRec.level_1_child || '',
        real_world: targetRec.example_real_world || ''
      });
      return;
    }

    if (qLowerGlobal.includes('child') || qLowerGlobal.includes('5 year') || qLowerGlobal.includes('5-year') || mode === 'child') {
      res.status(200).json({
        status: 'success',
        record_id: targetRec.record_id,
        category: targetRec.category,
        question: targetRec.question,
        answer: `🧒 <strong>Explained for a 5-Year-Old (${targetRec.category})</strong>:\n\n${targetRec.level_1_child}`,
        child_analogy: targetRec.level_1_child || '',
        real_world: targetRec.example_real_world || ''
      });
      return;
    }

    if (qLowerGlobal.includes('another example') || qLowerGlobal.includes('more example')) {
      const extraCode = targetRec.example_medium_code || targetRec.example_simple_code || '';
      res.status(200).json({
        status: 'success',
        record_id: targetRec.record_id,
        category: targetRec.category,
        question: targetRec.question,
        answer: `💡 <strong>Here is another practical example for ${targetRec.category}</strong>:\n\n${targetRec.code_explanation || ''}`,
        code_simple: extraCode,
        expected_output: targetRec.expected_output || '',
        real_world: targetRec.example_real_world || ''
      });
      return;
    }
  }

  // 3. Search and Retrieval across Knowledge Records
  let bestScore = 0;
  let bestRec = null;

  for (const rec of dataset) {
    const s = scoreRecord(rec, userQuery);
    if (s > bestScore) {
      bestScore = s;
      bestRec = rec;
    }
  }

  // Honest fallback
  if (bestScore <= 20 || !bestRec) {
    let fallbackText = `I apologize, but your specific query ('<strong>${userQuery}</strong>') is not currently covered in the OOP Master knowledge base. Rather than inventing an unverified response, I provide an honest fallback.\n\nWould you like to explore one of our thoroughly verified Object-Oriented Programming concepts below?`;
    if (curHuman === 'hi') {
      fallbackText = `मुझे क्षमा करें, आपका विशिष्ट प्रश्न ('<strong>${userQuery}</strong>') वर्तमान OOP Master नॉलेज बेस में सीधे शामिल नहीं है। गलत या मनगढ़ंत जानकारी देने के बजाय मैं पूरी ईमानदारी से यह स्पष्ट करता हूँ।\n\nक्या आप हमारे सत्यापित और विस्तृत OOP विषयों में से किसी के बारे में जानना चाहते हैं?`;
    } else if (curHuman === 'mr') {
      fallbackText = `मला क्षमा करा, आपला विशिष्ट प्रश्न ('<strong>${userQuery}</strong>') सध्या OOP Master च्या नॉलेज बेसमध्ये उपलब्ध नाही. चुकीची माहिती देण्यापेक्षा मी प्रामाणिकपणे हे स्पष्ट करतो.\n\nआपण खालीलपैकी कोणत्याही सत्यापित OOP विषयाची सविस्तर माहिती पाहू इच्छिता का?`;
    }

    res.status(200).json({
      status: 'not_found',
      category: 'Honest Fallback',
      answer: fallbackText,
      quick_topics: [
        { label: '🏗️ Classes & Objects', query: 'What is a class and object?' },
        { label: '🧬 Multilevel Inheritance', query: 'What is multilevel inheritance in C++?' },
        { label: '💎 Diamond Problem', query: 'What is the diamond problem and virtual inheritance?' },
        { label: '🎭 Runtime Polymorphism', query: 'What is runtime polymorphism?' },
        { label: '⚡ Virtual Functions & Vtables', query: 'Why do we use virtual functions?' },
        { label: '🔒 Private Inheritance', query: 'What is private inheritance?' },
        { label: '📐 SOLID Principles', query: 'What are SOLID principles?' }
      ]
    });
    return;
  }

  // Determine answer by mode
  let chosenAnswer = bestRec.answer_medium || '';
  if (mode === 'short' || qLowerGlobal.includes('short')) chosenAnswer = bestRec.answer_short || chosenAnswer;
  else if (mode === 'child' || qLowerGlobal.includes('child') || qLowerGlobal.includes('5 year')) chosenAnswer = bestRec.level_1_child || chosenAnswer;
  else if (mode === 'simpler' || qLowerGlobal.includes('simpler')) chosenAnswer = bestRec.example_real_world || chosenAnswer;
  else if (mode === 'detailed' || qLowerGlobal.includes('detailed')) chosenAnswer = bestRec.answer_detailed || chosenAnswer;

  let codeSnippet = getMultiLangCode(bestRec.record_id, progLang);
  if (!codeSnippet) {
    codeSnippet = bestRec.example_simple_code || '';
  }

  res.status(200).json({
    status: 'success',
    record_id: bestRec.record_id,
    category: bestRec.category,
    question: bestRec.question,
    answer: chosenAnswer,
    answer_short: bestRec.answer_short,
    answer_medium: bestRec.answer_medium,
    answer_detailed: bestRec.answer_detailed,
    child_analogy: bestRec.level_1_child,
    real_world: bestRec.example_real_world,
    code_simple: codeSnippet,
    code_lang: progLang,
    expected_output: bestRec.expected_output,
    follow_up_questions: bestRec.follow_up_questions || []
  });
};
