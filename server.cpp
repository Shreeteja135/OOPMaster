#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

struct KnowledgeRecord {
    std::string id;
    std::string category;
    std::string subcategory;
    std::string progLang;
    std::string humanLang;
    std::string difficulty;
    std::string questionType;
    std::string question;
    std::vector<std::string> alternateQuestions;
    std::vector<std::string> searchTerms;
    std::string answerShort;
    std::string answerMedium;
    std::string answerDetailed;
    std::string level1Child;
    std::string level2Beginner;
    std::string level3School;
    std::string level4College;
    std::string level5Intermediate;
    std::string level6Advanced;
    std::string level7Expert;
    std::string exampleRealWorld;
    std::string exampleSimpleCode;
    std::string exampleMediumCode;
    std::string exampleAdvancedCode;
    std::string codeExplanation;
    std::string expectedOutput;
    std::vector<std::string> commonMistakes;
    std::vector<std::string> relatedTopics;
    std::vector<std::string> followUpQuestions;
    std::string sourceNotes;
    std::string verificationStatus;
};

// Global in-memory knowledge store loaded by C++
std::vector<KnowledgeRecord> g_enRecords;
std::vector<KnowledgeRecord> g_hiRecords;
std::vector<KnowledgeRecord> g_mrRecords;

std::string toLower(const std::string& str) {
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Levenshtein distance for fuzzy typo matching
int levenshteinDistance(const std::string& s1, const std::string& s2) {
    size_t len1 = s1.size(), len2 = s2.size();
    if (len1 == 0) return (int)len2;
    if (len2 == 0) return (int)len1;

    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));
    for (size_t i = 0; i <= len1; ++i) d[i][0] = (int)i;
    for (size_t j = 0; j <= len2; ++j) d[0][j] = (int)j;

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = std::min(std::min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
        }
    }
    return d[len1][len2];
}

std::string urlDecode(const std::string& src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < (int)src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < (int)src.length()) {
                std::string hexStr = src.substr(i + 1, 2);
                std::istringstream iss(hexStr);
                if (iss >> std::hex >> ii) {
                    ch = static_cast<char>(ii);
                    ret += ch;
                    i += 2;
                } else {
                    ret += src[i];
                }
            } else {
                ret += src[i];
            }
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    o << "\\u" << std::hex << (int)(static_cast<unsigned char>(c));
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

bool parseKnowledgeFile(const std::string& filename, std::vector<KnowledgeRecord>& records) {
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;

    std::string line;
    KnowledgeRecord current;
    bool inRecord = false;
    std::string currentBlock = "";
    std::vector<std::string> blockLines;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (line[0] == '#' && !inRecord) continue;

        if (line == "=====RECORD_BEGIN=====") {
            inRecord = true;
            current = KnowledgeRecord();
            currentBlock = "";
            blockLines.clear();
            continue;
        }

        if (line == "=====RECORD_END=====") {
            if (inRecord) {
                records.push_back(current);
                inRecord = false;
            }
            continue;
        }

        if (!inRecord) continue;

        if (!currentBlock.empty()) {
            if (line == ">>" + currentBlock) {
                std::string blockContent = "";
                for (size_t i = 0; i < blockLines.size(); ++i) {
                    std::string bl = blockLines[i];
                    if (!bl.empty() && bl[0] == '\\') bl = bl.substr(1);
                    if (i > 0) blockContent += "\n";
                    blockContent += bl;
                }

                if (currentBlock == "ALTERNATE_QUESTIONS") {
                    for (const auto& bl : blockLines) current.alternateQuestions.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                } else if (currentBlock == "SEARCH_TERMS") {
                    for (const auto& bl : blockLines) current.searchTerms.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                } else if (currentBlock == "COMMON_MISTAKES") {
                    for (const auto& bl : blockLines) current.commonMistakes.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                } else if (currentBlock == "RELATED_TOPICS") {
                    for (const auto& bl : blockLines) current.relatedTopics.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                } else if (currentBlock == "FOLLOW_UP_QUESTIONS") {
                    for (const auto& bl : blockLines) current.followUpQuestions.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                } else if (currentBlock == "ANSWER_SHORT") current.answerShort = blockContent;
                else if (currentBlock == "ANSWER_MEDIUM") current.answerMedium = blockContent;
                else if (currentBlock == "ANSWER_DETAILED") current.answerDetailed = blockContent;
                else if (currentBlock == "LEVEL_1_CHILD") current.level1Child = blockContent;
                else if (currentBlock == "LEVEL_2_ABSOLUTE_BEGINNER") current.level2Beginner = blockContent;
                else if (currentBlock == "LEVEL_3_SCHOOL_STUDENT") current.level3School = blockContent;
                else if (currentBlock == "LEVEL_4_COLLEGE_BEGINNER") current.level4College = blockContent;
                else if (currentBlock == "LEVEL_5_INTERMEDIATE") current.level5Intermediate = blockContent;
                else if (currentBlock == "LEVEL_6_ADVANCED") current.level6Advanced = blockContent;
                else if (currentBlock == "LEVEL_7_EXPERT") current.level7Expert = blockContent;
                else if (currentBlock == "EXAMPLE_REAL_WORLD") current.exampleRealWorld = blockContent;
                else if (currentBlock == "EXAMPLE_SIMPLE_CODE") current.exampleSimpleCode = blockContent;
                else if (currentBlock == "EXAMPLE_MEDIUM_CODE") current.exampleMediumCode = blockContent;
                else if (currentBlock == "EXAMPLE_ADVANCED_CODE") current.exampleAdvancedCode = blockContent;
                else if (currentBlock == "CODE_EXPLANATION") current.codeExplanation = blockContent;
                else if (currentBlock == "EXPECTED_OUTPUT") current.expectedOutput = blockContent;
                else if (currentBlock == "SOURCE_NOTES") current.sourceNotes = blockContent;

                currentBlock = "";
                blockLines.clear();
            } else {
                blockLines.push_back(line);
            }
            continue;
        }

        if (line.size() > 2 && line.substr(line.size() - 2) == "<<") {
            currentBlock = line.substr(0, line.size() - 2);
            blockLines.clear();
            continue;
        }

        size_t colon = line.find(": ");
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 2);
            if (key == "RECORD_ID") current.id = val;
            else if (key == "CATEGORY") current.category = val;
            else if (key == "SUBCATEGORY") current.subcategory = val;
            else if (key == "PROGRAMMING_LANGUAGE") current.progLang = val;
            else if (key == "HUMAN_LANGUAGE") current.humanLang = val;
            else if (key == "DIFFICULTY") current.difficulty = val;
            else if (key == "QUESTION_TYPE") current.questionType = val;
            else if (key == "QUESTION") current.question = val;
            else if (key == "VERIFICATION_STATUS") current.verificationStatus = val;
        }
    }
    return true;
}

// Multi-language code snippets generator for key concepts
std::string getMultiLangCode(const std::string& recordId, const std::string& lang) {
    std::string l = toLower(lang);

    if (recordId == "OOP_CPP_CLASS_OBJECT_001") {
        if (l == "python" || l == "py") {
            return "class Student:\n"
                   "    def __init__(self, name, roll_no):\n"
                   "        self.name = name          # Attribute\n"
                   "        self.roll_no = roll_no\n\n"
                   "    def display(self):\n"
                   "        print(f\"Student: {self.name}, Roll: {self.roll_no}\")\n\n"
                   "# Object creation\n"
                   "s1 = Student(\"Aarav\", 101)\n"
                   "s1.display()";
        } else if (l == "java") {
            return "class Student {\n"
                   "    String name;\n"
                   "    int rollNo;\n\n"
                   "    Student(String name, int rollNo) {\n"
                   "        this.name = name;\n"
                   "        this.rollNo = rollNo;\n"
                   "    }\n\n"
                   "    void display() {\n"
                   "        System.out.println(\"Student: \" + name + \", Roll: \" + rollNo);\n"
                   "    }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        Student s1 = new Student(\"Aarav\", 101);\n"
                   "        s1.display();\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "class Student {\n"
                   "    public string Name { get; set; }\n"
                   "    public int RollNo { get; set; }\n\n"
                   "    public void Display() {\n"
                   "        Console.WriteLine($\"Student: {Name}, Roll: {RollNo}\");\n"
                   "    }\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        Student s1 = new Student { Name = \"Aarav\", RollNo = 101 };\n"
                   "        s1.Display();\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js" || l == "typescript" || l == "ts") {
            return "class Student {\n"
                   "    constructor(name, rollNo) {\n"
                   "        this.name = name;\n"
                   "        this.rollNo = rollNo;\n"
                   "    }\n\n"
                   "    display() {\n"
                   "        console.log(`Student: ${this.name}, Roll: ${this.rollNo}`);\n"
                   "    }\n"
                   "}\n\n"
                   "const s1 = new Student(\"Aarav\", 101);\n"
                   "s1.display();";
        }
    } else if (recordId == "OOP_CPP_INHERITANCE_001") {
        if (l == "python" || l == "py") {
            return "class Animal:\n"
                   "    def eat(self):\n"
                   "        print(\"Animal is eating...\")\n\n"
                   "class Dog(Animal):  # Dog inherits Animal\n"
                   "    def bark(self):\n"
                   "        print(\"Dog is barking: Woof!\")\n\n"
                   "d = Dog()\n"
                   "d.eat()   # Inherited from Animal\n"
                   "d.bark()  # Defined in Dog";
        } else if (l == "java") {
            return "class Animal {\n"
                   "    void eat() {\n"
                   "        System.out.println(\"Animal is eating...\");\n"
                   "    }\n"
                   "}\n\n"
                   "class Dog extends Animal {  // Dog inherits Animal\n"
                   "    void bark() {\n"
                   "        System.out.println(\"Dog is barking: Woof!\");\n"
                   "    }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        Dog d = new Dog();\n"
                   "        d.eat();   // Inherited\n"
                   "        d.bark();  // Derived method\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "class Animal {\n"
                   "    public void Eat() => Console.WriteLine(\"Animal is eating...\");\n"
                   "}\n\n"
                   "class Dog : Animal {  // Dog inherits Animal\n"
                   "    public void Bark() => Console.WriteLine(\"Dog is barking: Woof!\");\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        Dog d = new Dog();\n"
                   "        d.Eat();\n"
                   "        d.Bark();\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js" || l == "typescript" || l == "ts") {
            return "class Animal {\n"
                   "    eat() {\n"
                   "        console.log(\"Animal is eating...\");\n"
                   "    }\n"
                   "}\n\n"
                   "class Dog extends Animal {\n"
                   "    bark() {\n"
                   "        console.log(\"Dog is barking: Woof!\");\n"
                   "    }\n"
                   "}\n\n"
                   "const d = new Dog();\n"
                   "d.eat();\n"
                   "d.bark();";
        }
    } else if (recordId == "OOP_CORE_POLYMORPHISM_001") {
        if (l == "python" || l == "py") {
            return "class Shape:\n"
                   "    def draw(self):\n"
                   "        print(\"Drawing generic shape\")\n\n"
                   "class Circle(Shape):\n"
                   "    def draw(self):\n"
                   "        print(\"Drawing a Circle ⭕\")\n\n"
                   "class Square(Shape):\n"
                   "    def draw(self):\n"
                   "        print(\"Drawing a Square ⬛\")\n\n"
                   "# Polymorphic call\n"
                   "shapes = [Circle(), Square()]\n"
                   "for s in shapes:\n"
                   "    s.draw()";
        } else if (l == "java") {
            return "class Shape {\n"
                   "    void draw() { System.out.println(\"Drawing shape\"); }\n"
                   "}\n"
                   "class Circle extends Shape {\n"
                   "    @Override void draw() { System.out.println(\"Drawing Circle ⭕\"); }\n"
                   "}\n"
                   "class Square extends Shape {\n"
                   "    @Override void draw() { System.out.println(\"Drawing Square ⬛\"); }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        Shape[] shapes = { new Circle(), new Square() };\n"
                   "        for (Shape s : shapes) s.draw();\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "class Shape {\n"
                   "    public virtual void Draw() => Console.WriteLine(\"Drawing shape\");\n"
                   "}\n"
                   "class Circle : Shape {\n"
                   "    public override void Draw() => Console.WriteLine(\"Drawing Circle ⭕\");\n"
                   "}\n"
                   "class Square : Shape {\n"
                   "    public override void Draw() => Console.WriteLine(\"Drawing Square ⬛\");\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        Shape[] shapes = { new Circle(), new Square() };\n"
                   "        foreach (var s in shapes) s.Draw();\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js") {
            return "class Shape { draw() { console.log(\"Drawing shape\"); } }\n"
                   "class Circle extends Shape { draw() { console.log(\"Drawing Circle ⭕\"); } }\n"
                   "class Square extends Shape { draw() { console.log(\"Drawing Square ⬛\"); } }\n\n"
                   "[new Circle(), new Square()].forEach(s => s.draw());";
        }
    } else if (recordId == "OOP_CORE_ENCAPSULATION_001") {
        if (l == "python" || l == "py") {
            return "class BankAccount:\n"
                   "    def __init__(self, initial_balance):\n"
                   "        self.__balance = initial_balance  # Private attribute\n\n"
                   "    def deposit(self, amount):\n"
                   "        if amount > 0: self.__balance += amount\n\n"
                   "    def get_balance(self):\n"
                   "        return self.__balance\n\n"
                   "account = BankAccount(1000)\n"
                   "account.deposit(500)\n"
                   "print(\"Balance:\", account.get_balance())";
        } else if (l == "java") {
            return "class BankAccount {\n"
                   "    private double balance; // Hidden data\n\n"
                   "    public BankAccount(double b) { this.balance = b; }\n"
                   "    public void deposit(double amt) { if (amt > 0) balance += amt; }\n"
                   "    public double getBalance() { return balance; }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        BankAccount acc = new BankAccount(1000);\n"
                   "        acc.deposit(500);\n"
                   "        System.out.println(\"Balance: \" + acc.getBalance());\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "class BankAccount {\n"
                   "    private double _balance;\n"
                   "    public BankAccount(double b) => _balance = b;\n"
                   "    public void Deposit(double amt) { if (amt > 0) _balance += amt; }\n"
                   "    public double Balance => _balance;\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        var acc = new BankAccount(1000);\n"
                   "        acc.Deposit(500);\n"
                   "        Console.WriteLine($\"Balance: {acc.Balance}\");\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js") {
            return "class BankAccount {\n"
                   "    #balance = 0; // Private field\n"
                   "    constructor(b) { this.#balance = b; }\n"
                   "    deposit(amt) { if (amt > 0) this.#balance += amt; }\n"
                   "    getBalance() { return this.#balance; }\n"
                   "}\n\n"
                   "const acc = new BankAccount(1000);\n"
                   "acc.deposit(500);\n"
                   "console.log(\"Balance:\", acc.getBalance());";
        }
    } else if (recordId == "OOP_CORE_ABSTRACTION_001" || recordId == "OOP_CORE_INTERFACE_ABSTRACT_001") {
        if (l == "python" || l == "py") {
            return "from abc import ABC, abstractmethod\n\n"
                   "class PaymentGateway(ABC):\n"
                   "    @abstractmethod\n"
                   "    def pay(self, amount):\n"
                   "        pass\n\n"
                   "class CreditCardPayment(PaymentGateway):\n"
                   "    def pay(self, amount):\n"
                   "        print(f\"Processing credit card payment of ${amount}\")\n\n"
                   "p = CreditCardPayment()\n"
                   "p.pay(250)";
        } else if (l == "java") {
            return "interface PaymentGateway {\n"
                   "    void pay(double amount);\n"
                   "}\n\n"
                   "class CreditCardPayment implements PaymentGateway {\n"
                   "    public void pay(double amount) {\n"
                   "        System.out.println(\"Paid $\" + amount + \" via Credit Card\");\n"
                   "    }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        PaymentGateway p = new CreditCardPayment();\n"
                   "        p.pay(250);\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "interface IPaymentGateway {\n"
                   "    void Pay(double amount);\n"
                   "}\n\n"
                   "class CreditCardPayment : IPaymentGateway {\n"
                   "    public void Pay(double amount) => Console.WriteLine($\"Paid ${amount} via Card\");\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        IPaymentGateway p = new CreditCardPayment();\n"
                   "        p.Pay(250);\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js") {
            return "class PaymentGateway {\n"
                   "    pay(amount) {\n"
                   "        throw new Error(\"Method 'pay()' must be implemented.\");\n"
                   "    }\n"
                   "}\n\n"
                   "class CardPayment extends PaymentGateway {\n"
                   "    pay(amount) { console.log(`Paid $${amount} via Card`); }\n"
                   "}\n\n"
                   "new CardPayment().pay(250);";
        }
    } else if (recordId == "OOP_CPP_CONSTRUCTOR_001") {
        if (l == "python" || l == "py") {
            return "class FileHandler:\n"
                   "    def __init__(self, filename):\n"
                   "        print(f\"Opening file: {filename}\")\n"
                   "        self.filename = filename\n\n"
                   "    def __del__(self):\n"
                   "        print(f\"Cleaning up and closing {self.filename}\")\n\n"
                   "f = FileHandler(\"test.txt\")";
        } else if (l == "java") {
            return "class DatabaseConnection implements AutoCloseable {\n"
                   "    public DatabaseConnection() {\n"
                   "        System.out.println(\"Connection opened in Constructor\");\n"
                   "    }\n"
                   "    @Override public void close() {\n"
                   "        System.out.println(\"Connection closed\");\n"
                   "    }\n"
                   "}\n\n"
                   "public class Main {\n"
                   "    public static void main(String[] args) {\n"
                   "        try (DatabaseConnection db = new DatabaseConnection()) {\n"
                   "            System.out.println(\"Querying database...\");\n"
                   "        }\n"
                   "    }\n"
                   "}";
        } else if (l == "csharp" || l == "c#") {
            return "using System;\n\n"
                   "class Resource : IDisposable {\n"
                   "    public Resource() => Console.WriteLine(\"Resource acquired\");\n"
                   "    public void Dispose() => Console.WriteLine(\"Resource released\");\n"
                   "}\n\n"
                   "class Program {\n"
                   "    static void Main() {\n"
                   "        using (var r = new Resource()) {\n"
                   "            Console.WriteLine(\"Working with resource...\");\n"
                   "        }\n"
                   "    }\n"
                   "}";
        } else if (l == "javascript" || l == "js") {
            return "class Resource {\n"
                   "    constructor(name) {\n"
                   "        this.name = name;\n"
                   "        console.log(`Resource ${name} acquired`);\n"
                   "    }\n"
                   "    release() {\n"
                   "        console.log(`Resource ${this.name} released`);\n"
                   "    }\n"
                   "}\n\n"
                   "const r = new Resource(\"DB\");\n"
                   "r.release();";
        }
    }

    return "";
}

// Normalizes typos using phonetic & typo dictionaries
std::string normalizeTypos(const std::string& input) {
    std::string s = toLower(input);

    // Common Typo Maps
    static const std::vector<std::pair<std::string, std::string>> typoList = {
        {"polymorfism", "polymorphism"},
        {"polymorphsm", "polymorphism"},
        {"polymorpism", "polymorphism"},
        {"inheretance", "inheritance"},
        {"inheratence", "inheritance"},
        {"inharitance", "inheritance"},
        {"inhertance", "inheritance"},
        {"consctructor", "constructor"},
        {"constructer", "constructor"},
        {"constractor", "constructor"},
        {"distructor", "destructor"},
        {"destructer", "destructor"},
        {"destructur", "destructor"},
        {"encapsulashun", "encapsulation"},
        {"encapsulaton", "encapsulation"},
        {"encapslation", "encapsulation"},
        {"abstracktion", "abstraction"},
        {"abstruction", "abstraction"},
        {"abstrack", "abstraction"},
        {"interfce", "interface"},
        {"intreface", "interface"},
        {"soild", "solid"},
        {"solide", "solid"},
        {"singlton", "singleton"},
        {"singletun", "singleton"},
        {"memry", "memory"},
        {"memori", "memory"},
        {"virasat", "inheritance"},
        {"varsa", "inheritance"},
        {"bahuroopta", "polymorphism"},
        {"bahuvidhikta", "polymorphism"},
        {"prachhadan", "encapsulation"},
        {"amoortikaran", "abstraction"},
        {"varg", "class"}
    };

    for (const auto& item : typoList) {
        size_t pos = s.find(item.first);
        if (pos != std::string::npos) {
            s.replace(pos, item.first.length(), item.second);
        }
    }
    return s;
}

// C++ Retrieval engine score calculation with fuzzy matching and subtopic boosting
int scoreRecord(const KnowledgeRecord& rec, const std::string& query) {
    int score = 0;
    std::string qLower = normalizeTypos(toLower(query));
    std::string recQLower = toLower(rec.question);

    // Exact matches
    if (qLower == recQLower) score += 120;
    else if (recQLower.find(qLower) != std::string::npos) score += 60;

    for (const auto& alt : rec.alternateQuestions) {
        std::string altLower = toLower(alt);
        if (qLower == altLower) score += 90;
        else if (qLower.find(altLower) != std::string::npos || altLower.find(qLower) != std::string::npos) {
            score += 45;
        }
    }

    for (const auto& term : rec.searchTerms) {
        std::string termLower = toLower(term);
        if (qLower.find(termLower) != std::string::npos) {
            score += 35;
        } else if (qLower.length() > 4 && termLower.length() > 4) {
            // Levenshtein fuzzy match
            if (levenshteinDistance(qLower, termLower) <= 2) {
                score += 30;
            }
        }
    }

    std::string catLower = toLower(rec.category);
    if (qLower.find(catLower) != std::string::npos) score += 20;

    std::string subcatLower = toLower(rec.subcategory);
    if (!subcatLower.empty() && qLower.find(subcatLower) != std::string::npos) {
        score += 150;
    }

    // Specific subtopic keyword boosting to ensure specific questions match specific records
    auto boostIfMatch = [&](const std::string& token, const std::string& targetRecId, int bonus) {
        if (qLower.find(token) != std::string::npos && rec.id.find(targetRecId) != std::string::npos) {
            score += bonus;
        }
    };

    boostIfMatch("multilevel", "MULTILEVEL", 250);
    boostIfMatch("multiple inheritance", "MULTIPLE", 250);
    boostIfMatch("hierarchical", "HIERARCHICAL", 250);
    boostIfMatch("hybrid", "HYBRID", 250);
    boostIfMatch("diamond", "HYBRID", 250);
    boostIfMatch("virtual inheritance", "HYBRID", 250);
    boostIfMatch("virtual base", "HYBRID", 250);
    boostIfMatch("private inheritance", "ACCESS_MODES", 280);
    boostIfMatch("public inheritance", "ACCESS_MODES", 220);
    boostIfMatch("protected inheritance", "ACCESS_MODES", 220);
    boostIfMatch("access mode", "ACCESS_MODES", 220);
    boostIfMatch("runtime polymorphism", "RUNTIME", 280);
    boostIfMatch("dynamic polymorphism", "RUNTIME", 280);
    boostIfMatch("dynamic binding", "RUNTIME", 220);
    boostIfMatch("late binding", "RUNTIME", 220);
    boostIfMatch("pure virtual", "PURE_VIRTUAL", 280);
    boostIfMatch("abstract class", "PURE_VIRTUAL", 180);
    if (qLower.find("pure virtual") == std::string::npos) {
        boostIfMatch("virtual function", "VIRTUAL_FUNCS", 220);
        boostIfMatch("virtual functions", "VIRTUAL_FUNCS", 220);
    }
    boostIfMatch("vtable", "VTABLES", 280);
    boostIfMatch("vtables", "VTABLES", 280);
    boostIfMatch("vptr", "VTABLES", 280);
    boostIfMatch("virtual table", "VTABLES", 280);
    boostIfMatch("virtual method table", "VTABLES", 280);
    boostIfMatch("compile-time polymorphism", "COMPILE_TIME", 280);
    boostIfMatch("compile time polymorphism", "COMPILE_TIME", 280);
    boostIfMatch("function overloading", "COMPILE_TIME", 200);
    boostIfMatch("operator overloading", "COMPILE_TIME", 200);
    boostIfMatch("access specifier", "ACCESS_SPECIFIERS", 220);
    boostIfMatch("accessibility", "ACCESS_SPECIFIERS", 220);
    boostIfMatch("friend function", "FRIEND", 280);
    boostIfMatch("friend class", "FRIEND", 280);
    boostIfMatch("static member", "STATIC", 280);
    boostIfMatch("static data member", "STATIC", 280);
    boostIfMatch("static function", "STATIC", 220);
    boostIfMatch("copy constructor", "CONSTRUCTOR_TYPES", 280);
    boostIfMatch("move constructor", "CONSTRUCTOR_TYPES", 280);
    boostIfMatch("types of constructor", "CONSTRUCTOR_TYPES", 280);
    boostIfMatch("types of constructors", "CONSTRUCTOR_TYPES", 280);
    boostIfMatch("constructor types", "CONSTRUCTOR_TYPES", 280);
    boostIfMatch("virtual destructor", "VIRTUAL_DESTRUCTOR", 280);
    boostIfMatch("virtual destructors", "VIRTUAL_DESTRUCTOR", 280);

    // Dampen generic overview records when a specific subtopic is queried
    if (rec.id == "OOP_CPP_INHERITANCE_001") {
        if (qLower.find("multilevel") != std::string::npos ||
            qLower.find("multiple") != std::string::npos ||
            qLower.find("hierarchical") != std::string::npos ||
            qLower.find("hybrid") != std::string::npos ||
            qLower.find("diamond") != std::string::npos ||
            qLower.find("private inheritance") != std::string::npos ||
            qLower.find("protected inheritance") != std::string::npos ||
            qLower.find("access mode") != std::string::npos) {
            score -= 200;
        }
    }
    if (rec.id == "OOP_CORE_POLYMORPHISM_001") {
        if (qLower.find("runtime") != std::string::npos ||
            qLower.find("dynamic") != std::string::npos ||
            qLower.find("virtual function") != std::string::npos ||
            qLower.find("pure virtual") != std::string::npos ||
            qLower.find("vtable") != std::string::npos ||
            qLower.find("vptr") != std::string::npos ||
            qLower.find("compile-time") != std::string::npos ||
            qLower.find("compile time") != std::string::npos ||
            qLower.find("overloading") != std::string::npos) {
            score -= 200;
        }
    }
    if (rec.id == "OOP_CPP_CONSTRUCTOR_001") {
        if (qLower.find("copy constructor") != std::string::npos ||
            qLower.find("move constructor") != std::string::npos ||
            qLower.find("types of constructor") != std::string::npos ||
            qLower.find("constructor types") != std::string::npos ||
            qLower.find("virtual destructor") != std::string::npos ||
            qLower.find("destructor") != std::string::npos) {
            score -= 200;
        }
    }

    return score;
}

// Check for general conversational queries (Greetings, How are you, Age, Identity, Creator, etc.)
std::string handleGenericChat(const std::string& rawQuery, const std::string& curHuman) {
    std::string s = " " + trim(toLower(rawQuery)) + " ";
    for (char& c : s) {
        if (c == '?' || c == '!' || c == '.' || c == ',' || c == '\'') c = ' ';
    }

    // Safety guard: If query contains any core OOP technical keywords, NEVER treat as chitchat!
    static const std::vector<std::string> oopTerms = {
        "polymorphism", "inheritance", "class", "classes", "object", "objects",
        "virtual", "vtable", "vptr", "abstract", "interface", "encapsulation",
        "abstraction", "constructor", "destructor", "specifier", "friend", "static",
        "solid", "pattern", "patterns", "overload", "override", "binding", "dispatch",
        "diamond", "multilevel", "hierarchical", "hybrid", "private", "protected", "public"
    };
    for (const auto& term : oopTerms) {
        if (s.find(term) != std::string::npos) {
            return ""; // Technical OOP query!
        }
    }

    auto hasWord = [&](const std::vector<std::string>& patterns) -> bool {
        for (const auto& p : patterns) {
            std::string spaced = " " + p + " ";
            if (s.find(spaced) != std::string::npos || s == spaced || s.find(p) != std::string::npos) {
                // If single short word like "hi", require word boundary
                if (p.length() <= 3) {
                    if (s.find(spaced) != std::string::npos || s == spaced) return true;
                } else {
                    return true;
                }
            }
        }
        return false;
    };

    // 1. How are you
    if (hasWord({"how are", "hoaw are", "how r u", "how do you", "kaise ho", "kasa ahes", "kashi ahes", "kase ahat", "kya haal"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Greeting\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मैं बहुत बढ़िया हूँ और अपने तेज़ C++ इंजन पर पूरी गति से चल रहा हूँ! 🚀\\n\\nआपकी कोडिंग कैसी चल रही है? आज आप कौन सा OOP टॉपिक सीखना चाहते हैं?\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मी अगदी मजेत आहे आणि माझ्या C++ इंजिनवर वेगाने कार्यरत आहे! 🚀\\n\\nआपला कोडिंगचा अभ्यास कसा चालू आहे? आज कोणती OOP संकल्पना शिकायची आहे?\",\n";
        } else {
            json << "  \"answer\": \"I am doing fantastic and running at peak performance on my native C++ engine! 🚀\\n\\nHow is your coding journey going today? Which Object-Oriented Programming concept shall we tackle?\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"🧬 Inheritance\", \"query\": \"What is inheritance?\"},\n"
             << "    {\"label\": \"🎭 Polymorphism\", \"query\": \"What is polymorphism?\"},\n"
             << "    {\"label\": \"📦 Encapsulation\", \"query\": \"What is encapsulation?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 2. Age
    if (hasWord({"age", "umar", "vay", "how old"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"About Me\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मानवीय वर्षों में तो मेरा जन्म हाल ही में C++ कोड से हुआ है! 👶\\n\\nलेकिन ज्ञान के मामले में, मेरी मेमोरी में 1967 (Simula 67) और Smalltalk-80 से लेकर आधुनिक C++20 और Python 3.12 तक के पूरे OOP इतिहास का गहरा ज्ञान समाहित है! ⏳\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मानवी वयाच्या दृष्टीने मी नुकताच C++ मध्ये कंपाइल झालो आहे! 👶\\n\\nपरंतु ज्ञानाच्या बाबतीत, माझ्या मेमरीमध्ये 1967 पासूनच्या (Simula 67 ते आधुनिक C++ आणि Java) सर्व OOP संकल्पनांचे सखोल ज्ञान आहे! ⏳\",\n";
        } else {
            json << "  \"answer\": \"In human years, I was compiled just recently! 👶\\n\\nIn computer science terms, however, my knowledge base spans over 55+ years of Object-Oriented Programming history—from Simula 67 and Smalltalk-80 to modern C++20, Python 3.12, and Java 21! ⏳\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"⏳ History of OOP\", \"query\": \"What is the history of object-oriented programming?\"},\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"📐 SOLID Principles\", \"query\": \"What are SOLID principles?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 3. Do you know me / Who am I
    if (hasWord({"know me", "know u", "who am i", "jaante ho", "olkhtos ka", "olkhtat ka"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"About You\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"आप एक होनहार और जिज्ञासु सॉफ़्टवेयर डेवलपर हैं जो Object-Oriented Programming में महारत हासिल कर रहे हैं! 🌟\\n\\nआपकी पूर्ण निजता (privacy) सुरक्षित रखने के लिए मैं कोई व्यक्तिगत डेटा स्टोर नहीं करता, लेकिन जब भी आप कोडिंग करेंगे, मैं आपका सबसे वफ़ादार मार्गदर्शक बना रहूँगा। 😊\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"आपण एक हुशार आणि उत्साही सॉफ्टवेअर डेवलपर आहात जे Object-Oriented Programming शिकत आहात! 🌟\\n\\nआपल्या गोपनीयतेच्या (privacy) रक्षणासाठी मी वैयक्तिक माहिती साठवत नाही, पण आपल्या कोडिंग प्रवासात मदत करण्यासाठी मी नेहमी तयार आहे. 😊\",\n";
        } else {
            json << "  \"answer\": \"You are an ambitious developer on an exciting journey to master Object-Oriented Programming! 🌟\\n\\nTo guarantee your privacy, I don't store personal profiling data, but I recognize your dedication to writing clean, professional code. I'm here whenever you need an explanation or code snippet! 😊\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"🧬 Inheritance\", \"query\": \"What is inheritance?\"},\n"
             << "    {\"label\": \"🎭 Polymorphism\", \"query\": \"What is polymorphism?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 4. Who are you / What is your name
    if (hasWord({"who are you", "who r u", "what is your name", "what's your name", "whats your name", "what is ur name", "aap kaun ho", "tu kon ahes", "aaple naav", "tumhara naam"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Identity\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मैं <strong>OOP Master</strong> हूँ! 🤖\\n\\nमैं एक C++ संचालित AI शिक्षक हूँ जो C++, Python, Java, C#, और JavaScript में Object-Oriented Programming की अवधारणाओं को बच्चों से लेकर विशेषज्ञों तक के स्तर पर समझाता हूँ।\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मी <strong>OOP Master</strong> आहे! 🤖\\n\\nमी एक C++ वर चालणारा AI मार्गदर्शक आहे जो C++, Python, Java, C# आणि JavaScript मध्ये OOP च्या संकल्पना अगदी सोप्या पद्धतीने शिकवतो.\",\n";
        } else {
            json << "  \"answer\": \"I am <strong>OOP Master</strong>! 🤖\\n\\nI am a dedicated AI coding tutor powered by an ultra-fast, native C++ retrieval engine. I can explain any Object-Oriented concept from child analogies to compiler-level vtables in 17+ languages!\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"🧬 Inheritance\", \"query\": \"What is inheritance?\"},\n"
             << "    {\"label\": \"⚡ 23 Design Patterns\", \"query\": \"What are design patterns?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 5. Who made you / Who created you
    if (hasWord({"who made you", "who created you", "who built you", "tumhe kisne banaya", "tula koni banavle", "your creator", "who developed you"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Origin\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मुझे <strong>OOP Master</strong> प्रोजेक्ट के लिए एक कस्टम C++ वेब सर्वर और इन-मेमोरी नॉलेज इंजन के साथ विकसित किया गया है ताकि छात्रों और डेवलपर्स को सबसे सटीक OOP मार्गदर्शन मिल सके! 💻⚙️\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मला <strong>OOP Master</strong> प्रोजेक्टसाठी एका कस्टम C++ वेब सर्व्हर आणि इन-मेमरी नॉलेज इंजिनद्वारे तयार केले गेले आहे! 💻⚙️\",\n";
        } else {
            json << "  \"answer\": \"I was created for the <strong>OOP Master</strong> project, architected with a native C++ WinSock2 web server and high-performance in-memory search engine to deliver instant, multilingual OOP guidance! 💻⚙️\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"⚙️ Constructors & RAII\", \"query\": \"What is a constructor?\"},\n"
             << "    {\"label\": \"💾 Memory Management\", \"query\": \"How does memory management work in OOP?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 6. Thank you
    if (hasWord({"thank you", "thanks", "thx", "thank u", "dhanyawad", "shukriya", "aabhar", "dhanyavaad"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Appreciation\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"आपका बहुत-बहुत स्वागत है! 😊 कोडिंग का अभ्यास जारी रखें और नए प्रोजेक्ट्स बनाते रहें। जब भी कोई सवाल हो, मैं यहीं हूँ! 💻✨\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"आपले मनापासून स्वागत! 😊 कोडिंगचा सराव चालू ठेवा. कोणतीही शंका असल्यास नक्की विचारा! 💻✨\",\n";
        } else {
            json << "  \"answer\": \"You are very welcome! 😊 Keep practicing and building great software. Feel free to ask anytime you encounter an OOP challenge! 💻✨\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"📐 SOLID Principles\", \"query\": \"What are SOLID principles?\"},\n"
             << "    {\"label\": \"⚡ Design Patterns\", \"query\": \"What are design patterns?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 7. Goodbye
    if (hasWord({"bye", "goodbye", "good bye", "alvida", "see you", "tata", "cya", "bye bye"})) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Farewell\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"अलविदा! 👋 आपकी कोडिंग यात्रा शानदार रहे। जब भी OOP का कोई नया सवाल हो, वापस ज़रूर आएँ!\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"निरोप! 👋 आपला कोडिंग प्रवास यशस्वी होवो. पुन्हा भेटूया!\",\n";
        } else {
            json << "  \"answer\": \"Goodbye! 👋 Have a great and productive coding session. Come back whenever you want to explore more OOP!\",\n";
        }
        json << "  \"quick_topics\": []\n}";
        return json.str();
    }

    // 8. Capabilities: "What can you do?", "What u can do", in all possible forms (English, Hindi, Marathi, Typos)
    bool isCapabilities = false;
    static const std::vector<std::string> capPatterns = {
        // English direct forms & typos
        "what can you do", "what can u do", "what u can do", "wat can you do", "wat can u do",
        "wat u can do", "waht can you do", "waht can u do", "waht u can do", "wht can you do",
        "wht can u do", "whaat can you do", "what do you do", "what do u do", "what u do",
        "wat do you do", "what can you answer", "what can u answer", "what questions can you answer",
        "what questions can u answer", "what question can you answer", "what question can u answer",
        "what can i ask", "what can i ask you", "what can i ask u", "what do you know", "what do u know",
        "what u know", "what are your capabilities", "your capabilities", "capabilities",
        "what are your features", "features", "how can you help", "how can u help",
        "how can you help me", "how can u help me", "how to use", "how to use this",
        "tell me what you can do", "tell me what u can do", "what is oop master",
        "what can this chatbot do", "what can this bot do", "help me", "help",
        // Hindi / Hinglish forms
        "kya kar sakte ho", "tum kya kar sakte ho", "aap kya kar sakte ho", "kya kya kar sakte ho",
        "kya kar sakti ho", "kya pooch sakte", "kya puch sakte", "kya poochh sakte", "tumhe kya aata hai",
        "aapko kya aata hai", "tum kya jaante ho", "kaise help karoge", "meri kya help karoge",
        // Marathi / Marathish forms
        "kay karu shaktos", "tu kay karu shaktos", "tumhi kay karu shakta", "kay karu shakta",
        "tula kay yeta", "tumhala kay yeta", "tula kay mahiti", "tumhala kay mahiti",
        "kay vicharu shakto", "mi kay vicharu", "kasli madat", "kashi madat"
    };

    if (hasWord(capPatterns)) {
        isCapabilities = true;
    } else {
        bool hasQuestionWord = (s.find("what") != std::string::npos || s.find("waht") != std::string::npos ||
                                s.find("wat") != std::string::npos || s.find("wht") != std::string::npos ||
                                s.find("kya") != std::string::npos || s.find("kay") != std::string::npos);
        bool hasVerb = (s.find("can") != std::string::npos || s.find("do") != std::string::npos ||
                        s.find("know") != std::string::npos || s.find("ask") != std::string::npos ||
                        s.find("help") != std::string::npos || s.find("answer") != std::string::npos ||
                        s.find("kar") != std::string::npos || s.find("yeta") != std::string::npos ||
                        s.find("vichar") != std::string::npos);
        bool hasSubject = (s.find("you") != std::string::npos || s.find(" u ") != std::string::npos ||
                           s.find("tum") != std::string::npos || s.find("aap") != std::string::npos ||
                           s.find(" tu ") != std::string::npos || s.find("tumhi") != std::string::npos ||
                           s.find(" i ") != std::string::npos || s.find("me") != std::string::npos);

        if (hasQuestionWord && hasVerb && hasSubject) {
            isCapabilities = true;
        }
    }

    if (isCapabilities) {
        std::ostringstream json;
        json << "{\n  \"status\": \"general\",\n  \"category\": \"Capabilities\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मैं Object-Oriented Programming (OOP) से जुड़े इन सभी सवालों के उत्तर दे सकता हूँ:\\n\\n"
                 << "1. 🏛️ <strong>OOP के 4 स्तंभ</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.\\n"
                 << "2. 🏗️ <strong>मूल अवधारणाएँ</strong>: Classes, Objects, Constructors, Destructors, RAII.\\n"
                 << "3. 📐 <strong>सॉफ्टवेयर डिज़ाइन</strong>: SOLID सिद्धांत (SRP, OCP, LSP, ISP, DIP), Composition बनाम Inheritance.\\n"
                 << "4. ⚡ <strong>23 Design Patterns</strong>: Singleton, Factory, Observer, Strategy, Adapter आदि।\\n"
                 << "5. 💻 <strong>कई भाषाओं में कोड</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.\\n"
                 << "6. 🧒 <strong>अलग-अलग स्तर</strong>: आप 'छोटा उत्तर दो', '5 साल के बच्चे की तरह समझाओ', या 'सरल उदाहरण दो' कह सकते हैं!\\n"
                 << "7. 🌐 <strong>3 भाषाएँ</strong>: English, हिन्दी, और मराठी।\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मी Object-Oriented Programming संदर्भातील खालील सर्व प्रश्नांची उत्तरे देऊ शकतो:\\n\\n"
                 << "1. 🏛️ <strong>OOP चे 4 मुख्य स्तंभ</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.\\n"
                 << "2. 🏗️ <strong>मूलभूत संकल्पना</strong>: Classes, Objects, Constructors, Destructors, RAII.\\n"
                 << "3. 📐 <strong>सॉफ्टवेअर डिझाईन</strong>: SOLID नियम, Composition विरूद्ध Inheritance.\\n"
                 << "4. ⚡ <strong>23 Design Patterns</strong>: Singleton, Factory, Observer, Strategy इत्यादी.\\n"
                 << "5. 💻 <strong>विविध भाषांमधील कोड</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.\\n"
                 << "6. 🧒 <strong>विविध पातळ्यांवर स्पष्टीकरण</strong>: आपण 'थोडक्यात सांगा', 'लहान मुलासारखे समजावून सांगा', किंवा 'सोपे उदाहरण द्या' विचारू शकता!\\n"
                 << "7. 🌐 <strong>3 भाषा</strong>: English, हिन्दी, आणि मराठी।\",\n";
        } else {
            json << "  \"answer\": \"Here is everything I can answer and do for you:\\n\\n"
                 << "1. 🏛️ <strong>The 4 Pillars of OOP</strong>: Encapsulation, Abstraction, Inheritance, Polymorphism.\\n"
                 << "2. 🏗️ <strong>Core Building Blocks</strong>: Classes, Objects, Constructors, Destructors, RAII, Copy/Move semantics.\\n"
                 << "3. 📐 <strong>Software Design Principles</strong>: SOLID Principles (SRP, OCP, LSP, ISP, DIP), Object Relationships (Composition vs Inheritance, Association, Aggregation).\\n"
                 << "4. ⚡ <strong>23 GoF Design Patterns</strong>: Creational (Singleton, Factory, Builder), Structural (Adapter, Decorator, Facade), Behavioral (Observer, Strategy, Command).\\n"
                 << "5. 💾 <strong>Memory & Lifetimes</strong>: Stack vs Heap, Smart Pointers, Virtual Destructors, Garbage Collection, Object Slicing.\\n"
                 << "6. 💻 <strong>Multi-Language Code</strong>: C++, Python, Java, C#, JavaScript, TypeScript, Rust, Go.\\n"
                 << "7. 🧒 <strong>Adaptive Explanations</strong>: Ask me to <em>'make it short'</em>, <em>'explain like I am 5'</em>, or <em>'give a simpler example'</em>.\\n"
                 << "8. 🌐 <strong>Multilingual</strong>: Ask in English, हिन्दी (Hindi), or मराठी (Marathi)!\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"🏛️ 4 Pillars of OOP\", \"query\": \"What are the four pillars of OOP?\"},\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"🧬 Inheritance\", \"query\": \"What is inheritance?\"},\n"
             << "    {\"label\": \"🎭 Polymorphism\", \"query\": \"What is polymorphism?\"},\n"
             << "    {\"label\": \"📐 SOLID Principles\", \"query\": \"What are SOLID principles?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    // 9. Standard Greetings
    static const std::vector<std::string> greetings = {
        "hi", "hello", "hey", "hola", "namaste", "pranam", "kasa ahes", "kasa kay",
        "good morning", "good afternoon", "good evening", "hii", "heyy", "namaskar",
        "start"
    };
    if (hasWord(greetings)) {
        std::ostringstream json;
        json << "{\n  \"status\": \"greeting\",\n  \"category\": \"Welcome\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"नमस्ते! 👋 मैं <strong>OOP Master</strong> हूँ, आपका Object-Oriented Programming AI शिक्षक।\\n\\nआप OOP की कौन सी अवधारणा सीखना चाहते हैं? नीचे दिए गए किसी विषय पर क्लिक करें या अपना प्रश्न पूछें:\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"नमस्कार! 👋 मी <strong>OOP Master</strong> आहे, आपला Object-Oriented Programming AI मार्गदर्शक।\\n\\nआपण OOP ची कोणती संकल्पना शिकू इच्छिता? खालीलपैकी एका विषयावर टॅप करा किंवा आपला प्रश्न विचारा:\",\n";
        } else {
            json << "  \"answer\": \"Hello! 👋 I am <strong>OOP Master</strong>, your dedicated Object-Oriented Programming AI tutor.\\n\\nWhich OOP concept would you like to explore today? Tap a topic below or type any question:\",\n";
        }
        json << "  \"quick_topics\": [\n"
             << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n"
             << "    {\"label\": \"🧬 Inheritance\", \"query\": \"What is inheritance?\"},\n"
             << "    {\"label\": \"🎭 Polymorphism\", \"query\": \"What is polymorphism?\"},\n"
             << "    {\"label\": \"📦 Encapsulation\", \"query\": \"What is encapsulation?\"},\n"
             << "    {\"label\": \"🛡️ Abstraction\", \"query\": \"What is abstraction?\"},\n"
             << "    {\"label\": \"⚙️ Constructors\", \"query\": \"What is a constructor?\"},\n"
             << "    {\"label\": \"📐 SOLID Principles\", \"query\": \"What are SOLID principles?\"},\n"
             << "    {\"label\": \"⚡ Design Patterns\", \"query\": \"What are design patterns?\"}\n"
             << "  ]\n}";
        return json.str();
    }

    return "";
}

// Check for "make it short" intent
bool isShortIntent(const std::string& q) {
    std::string s = toLower(q);
    return (s == "short" || s.find("make it short") != std::string::npos ||
            s.find("chota") != std::string::npos || s.find("brief") != std::string::npos ||
            s.find("summary") != std::string::npos || s.find("thodkyat") != std::string::npos ||
            s.find("in short") != std::string::npos);
}

// Check for "explain like I am 5 / child" intent
bool isChildIntent(const std::string& q) {
    std::string s = toLower(q);
    return (s.find("child") != std::string::npos || s.find("5 year") != std::string::npos ||
            s.find("5 yr") != std::string::npos || s.find("eli5") != std::string::npos ||
            s.find("five") != std::string::npos || s.find(" 5 ") != std::string::npos ||
            s.find("like im 5") != std::string::npos || s.find("like i am 5") != std::string::npos ||
            s.find("like i'm 5") != std::string::npos || s.find("like i'm five") != std::string::npos ||
            s.find("like im five") != std::string::npos || s.find("bacche") != std::string::npos ||
            s.find("lahan mul") != std::string::npos || s.find("simplest") != std::string::npos ||
            s.find("chilfd") != std::string::npos || s.find("chield") != std::string::npos);
}

// Check for "I don't understand / simpler" intent
bool isSimplerIntent(const std::string& q) {
    std::string s = toLower(q);
    return (s.find("dont understand") != std::string::npos || s.find("don't understand") != std::string::npos ||
            s.find("samajh nahi") != std::string::npos || s.find("nahi samjla") != std::string::npos ||
            s.find("confused") != std::string::npos || s.find("simpler") != std::string::npos ||
            s.find("easy example") != std::string::npos || s.find("real world") != std::string::npos);
}

// Check for "detailed" intent
bool isDetailedIntent(const std::string& q) {
    std::string s = toLower(q);
    return (s.find("detailed") != std::string::npos || s.find("deep") != std::string::npos ||
            s.find("in depth") != std::string::npos || s.find("vistar") != std::string::npos ||
            s.find("savistar") != std::string::npos || s.find("advance") != std::string::npos);
}

// Check if query is just a programming language name
std::string detectProgLangOnly(const std::string& q) {
    std::string s = trim(toLower(q));
    if (s == "c++" || s == "cpp" || s == "in c++") return "cpp";
    if (s == "python" || s == "py" || s == "in python") return "python";
    if (s == "java" || s == "in java") return "java";
    if (s == "c#" || s == "csharp" || s == "in c#") return "csharp";
    if (s == "javascript" || s == "js" || s == "in javascript") return "javascript";
    return "";
}

// C++ Retrieval and answer synthesis
std::string retrieveAnswer(const std::string& rawQuery, const std::string& humanLang,
                           std::string mode, std::string progLang, const std::string& currentTopic) {
    std::string userQuery = trim(rawQuery);
    std::string qLowerGlobal = toLower(userQuery);

    const std::vector<KnowledgeRecord>* dataset = &g_enRecords;
    std::string curHuman = "en";
    if (humanLang == "hi" || humanLang == "hindi") { dataset = &g_hiRecords; curHuman = "hi"; }
    else if (humanLang == "mr" || humanLang == "marathi") { dataset = &g_mrRecords; curHuman = "mr"; }

    // Dynamic detection of human language from query text if explicitly asked in question
    if (qLowerGlobal.find("in marathi") != std::string::npos || qLowerGlobal.find("marathi madhye") != std::string::npos || qLowerGlobal.find("marathit") != std::string::npos) {
        dataset = &g_mrRecords;
        curHuman = "mr";
    } else if (qLowerGlobal.find("in hindi") != std::string::npos || qLowerGlobal.find("hindi mein") != std::string::npos || qLowerGlobal.find("hindi me") != std::string::npos) {
        dataset = &g_hiRecords;
        curHuman = "hi";
    } else if (qLowerGlobal.find("in english") != std::string::npos) {
        dataset = &g_enRecords;
        curHuman = "en";
    }

    // Dynamic detection of programming language from query text
    if (qLowerGlobal.find("c++") != std::string::npos || qLowerGlobal.find("in cpp") != std::string::npos) progLang = "cpp";
    else if (qLowerGlobal.find("python") != std::string::npos) progLang = "python";
    else if (qLowerGlobal.find("java") != std::string::npos && qLowerGlobal.find("javascript") == std::string::npos) progLang = "java";
    else if (qLowerGlobal.find("c#") != std::string::npos || qLowerGlobal.find("csharp") != std::string::npos) progLang = "csharp";
    else if (qLowerGlobal.find("javascript") != std::string::npos || qLowerGlobal.find("typescript") != std::string::npos) progLang = "javascript";

    // 1. Check for general chitchat & greetings first
    std::string generalReply = handleGenericChat(userQuery, curHuman);
    if (!generalReply.empty()) {
        return generalReply;
    }

    // 2. Check for Contextual Modifiers with an active topic
    const KnowledgeRecord* targetRec = nullptr;

    if (!currentTopic.empty()) {
        for (const auto& rec : *dataset) {
            if (rec.id == currentTopic) {
                targetRec = &rec;
                break;
            }
        }
        // If not found in current dataset, look across all datasets to recover ID
        if (targetRec == nullptr) {
            for (const auto& rec : g_enRecords) {
                if (rec.id == currentTopic) { targetRec = &rec; break; }
            }
        }
    }

    // Check if query is just a programming language switch (e.g. "C++", "Python", "Java")
    std::string detectedLang = detectProgLangOnly(userQuery);
    if (!detectedLang.empty()) {
        progLang = detectedLang;
        if (targetRec != nullptr) {
            // Return code for the target topic in requested language!
            std::string codeSnippet = getMultiLangCode(targetRec->id, progLang);
            if (codeSnippet.empty()) codeSnippet = targetRec->exampleSimpleCode;

            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"success\",\n";
            json << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n";
            json << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n";
            json << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            json << "  \"answer\": \"Here is the <strong>" << escapeJson(progLang) << "</strong> implementation for <strong>" << escapeJson(targetRec->category) << "</strong>:\",\n";
            json << "  \"code_simple\": \"" << escapeJson(codeSnippet) << "\",\n";
            json << "  \"expected_output\": \"" << escapeJson(targetRec->expectedOutput) << "\",\n";
            json << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n";
            json << "}";
            return json.str();
        } else {
            // No active topic, ask user which topic they want in this language
            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"greeting\",\n";
            json << "  \"category\": \"Language Selected: " << escapeJson(progLang) << "\",\n";
            json << "  \"answer\": \"Great! I am ready to provide <strong>" << escapeJson(progLang) << "</strong> code. Which concept would you like to see?\",\n";
            json << "  \"quick_topics\": [\n";
            json << "    {\"label\": \"Classes & Objects in " << escapeJson(progLang) << "\", \"query\": \"Classes and objects in " << escapeJson(progLang) << "\"},\n";
            json << "    {\"label\": \"Inheritance in " << escapeJson(progLang) << "\", \"query\": \"Inheritance in " << escapeJson(progLang) << "\"},\n";
            json << "    {\"label\": \"Polymorphism in " << escapeJson(progLang) << "\", \"query\": \"Polymorphism in " << escapeJson(progLang) << "\"},\n";
            json << "    {\"label\": \"Encapsulation in " << escapeJson(progLang) << "\", \"query\": \"Encapsulation in " << escapeJson(progLang) << "\"}\n";
            json << "  ]\n";
            json << "}";
            return json.str();
        }
    }

    // Check contextual modifiers on existing topic
    if (targetRec != nullptr) {
        // A. Explain in another human language for the active topic
        if (qLowerGlobal.find("in marathi") != std::string::npos || qLowerGlobal == "marathi" || qLowerGlobal == "marathit sanga") {
            for (const auto& r : g_mrRecords) {
                if (r.id == targetRec->id) {
                    std::ostringstream json;
                    json << "{\n"
                         << "  \"status\": \"success\",\n"
                         << "  \"record_id\": \"" << escapeJson(r.id) << "\",\n"
                         << "  \"category\": \"" << escapeJson(r.category) << "\",\n"
                         << "  \"question\": \"" << escapeJson(r.question) << "\",\n"
                         << "  \"answer\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_short\": \"" << escapeJson(r.answerShort) << "\",\n"
                         << "  \"answer_medium\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_detailed\": \"" << escapeJson(r.answerDetailed) << "\",\n"
                         << "  \"child_analogy\": \"" << escapeJson(r.level1Child) << "\",\n"
                         << "  \"real_world\": \"" << escapeJson(r.exampleRealWorld) << "\",\n"
                         << "  \"code_simple\": \"" << escapeJson(r.exampleSimpleCode) << "\",\n"
                         << "  \"expected_output\": \"" << escapeJson(r.expectedOutput) << "\"\n"
                         << "}";
                    return json.str();
                }
            }
        }
        if (qLowerGlobal.find("in hindi") != std::string::npos || qLowerGlobal == "hindi" || qLowerGlobal == "hindi me samjhao") {
            for (const auto& r : g_hiRecords) {
                if (r.id == targetRec->id) {
                    std::ostringstream json;
                    json << "{\n"
                         << "  \"status\": \"success\",\n"
                         << "  \"record_id\": \"" << escapeJson(r.id) << "\",\n"
                         << "  \"category\": \"" << escapeJson(r.category) << "\",\n"
                         << "  \"question\": \"" << escapeJson(r.question) << "\",\n"
                         << "  \"answer\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_short\": \"" << escapeJson(r.answerShort) << "\",\n"
                         << "  \"answer_medium\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_detailed\": \"" << escapeJson(r.answerDetailed) << "\",\n"
                         << "  \"child_analogy\": \"" << escapeJson(r.level1Child) << "\",\n"
                         << "  \"real_world\": \"" << escapeJson(r.exampleRealWorld) << "\",\n"
                         << "  \"code_simple\": \"" << escapeJson(r.exampleSimpleCode) << "\",\n"
                         << "  \"expected_output\": \"" << escapeJson(r.expectedOutput) << "\"\n"
                         << "}";
                    return json.str();
                }
            }
        }
        if (qLowerGlobal.find("in english") != std::string::npos || qLowerGlobal == "english") {
            for (const auto& r : g_enRecords) {
                if (r.id == targetRec->id) {
                    std::ostringstream json;
                    json << "{\n"
                         << "  \"status\": \"success\",\n"
                         << "  \"record_id\": \"" << escapeJson(r.id) << "\",\n"
                         << "  \"category\": \"" << escapeJson(r.category) << "\",\n"
                         << "  \"question\": \"" << escapeJson(r.question) << "\",\n"
                         << "  \"answer\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_short\": \"" << escapeJson(r.answerShort) << "\",\n"
                         << "  \"answer_medium\": \"" << escapeJson(r.answerMedium) << "\",\n"
                         << "  \"answer_detailed\": \"" << escapeJson(r.answerDetailed) << "\",\n"
                         << "  \"child_analogy\": \"" << escapeJson(r.level1Child) << "\",\n"
                         << "  \"real_world\": \"" << escapeJson(r.exampleRealWorld) << "\",\n"
                         << "  \"code_simple\": \"" << escapeJson(r.exampleSimpleCode) << "\",\n"
                         << "  \"expected_output\": \"" << escapeJson(r.expectedOutput) << "\"\n"
                         << "}";
                    return json.str();
                }
            }
        }

        // B. "explain again" / "once more" / "repeat"
        if (qLowerGlobal.find("again") != std::string::npos || qLowerGlobal.find("once more") != std::string::npos ||
            qLowerGlobal.find("repeat") != std::string::npos || qLowerGlobal.find("dobara") != std::string::npos ||
            qLowerGlobal.find("phir se") != std::string::npos || qLowerGlobal.find("punha") != std::string::npos) {
            std::ostringstream json;
            json << "{\n"
                 << "  \"status\": \"success\",\n"
                 << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n"
                 << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n"
                 << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            if (curHuman == "hi") {
                json << "  \"answer\": \"🔄 <strong>" << escapeJson(targetRec->category) << " का पुनः स्पष्टीकरण</strong>:\\n\\n" << escapeJson(targetRec->answerDetailed) << "\",\n";
            } else if (curHuman == "mr") {
                json << "  \"answer\": \"🔄 <strong>" << escapeJson(targetRec->category) << " चे पुन्हा स्पष्टीकरण</strong>:\\n\\n" << escapeJson(targetRec->answerDetailed) << "\",\n";
            } else {
                json << "  \"answer\": \"🔄 <strong>Re-explaining " << escapeJson(targetRec->category) << " in depth</strong>:\\n\\n" << escapeJson(targetRec->answerDetailed) << "\",\n";
            }
            json << "  \"code_simple\": \"" << escapeJson(targetRec->exampleSimpleCode) << "\",\n"
                 << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n"
                 << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n"
                 << "}";
            return json.str();
        }

        // C. "give another example" / "another example" / "one more example"
        if (qLowerGlobal.find("another example") != std::string::npos || qLowerGlobal.find("more example") != std::string::npos ||
            qLowerGlobal.find("aur udaharan") != std::string::npos || qLowerGlobal.find("dusra udaharan") != std::string::npos ||
            qLowerGlobal.find("dusre udaharan") != std::string::npos || qLowerGlobal.find("aankhi udaharan") != std::string::npos) {
            std::string extraCode = targetRec->exampleMediumCode.empty() ? targetRec->exampleSimpleCode : targetRec->exampleMediumCode;
            std::ostringstream json;
            json << "{\n"
                 << "  \"status\": \"success\",\n"
                 << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n"
                 << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n"
                 << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            if (curHuman == "hi") {
                json << "  \"answer\": \"💡 <strong>" << escapeJson(targetRec->category) << " का एक और व्यावहारिक उदाहरण</strong>:\\n\\n" << escapeJson(targetRec->codeExplanation) << "\",\n";
            } else if (curHuman == "mr") {
                json << "  \"answer\": \"💡 <strong>" << escapeJson(targetRec->category) << " चे आणखी एक प्रात्यक्षिक उदाहरण</strong>:\\n\\n" << escapeJson(targetRec->codeExplanation) << "\",\n";
            } else {
                json << "  \"answer\": \"💡 <strong>Here is another practical example for " << escapeJson(targetRec->category) << "</strong>:\\n\\n" << escapeJson(targetRec->codeExplanation) << "\",\n";
            }
            json << "  \"code_simple\": \"" << escapeJson(extraCode) << "\",\n"
                 << "  \"expected_output\": \"" << escapeJson(targetRec->expectedOutput) << "\",\n"
                 << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n"
                 << "}";
            return json.str();
        }

        if (isShortIntent(userQuery)) {
            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"success\",\n";
            json << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n";
            json << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n";
            json << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            json << "  \"answer\": \"⚡ <strong>Short Summary (" << escapeJson(targetRec->category) << ")</strong>: " << escapeJson(targetRec->answerShort) << "\",\n";
            json << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n";
            json << "}";
            return json.str();
        }

        if (isChildIntent(userQuery)) {
            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"success\",\n";
            json << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n";
            json << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n";
            json << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            json << "  \"answer\": \"🧒 <strong>Explained for a 5-Year-Old (" << escapeJson(targetRec->category) << ")</strong>:\\n\\n" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n";
            json << "}";
            return json.str();
        }

        if (isSimplerIntent(userQuery)) {
            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"success\",\n";
            json << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n";
            json << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n";
            json << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            json << "  \"answer\": \"💡 <strong>Simpler Real-World Example (" << escapeJson(targetRec->category) << ")</strong>:\\n\\n" << escapeJson(targetRec->exampleRealWorld) << "\",\n";
            json << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n";
            json << "}";
            return json.str();
        }

        if (isDetailedIntent(userQuery)) {
            std::ostringstream json;
            json << "{\n";
            json << "  \"status\": \"success\",\n";
            json << "  \"record_id\": \"" << escapeJson(targetRec->id) << "\",\n";
            json << "  \"category\": \"" << escapeJson(targetRec->category) << "\",\n";
            json << "  \"question\": \"" << escapeJson(targetRec->question) << "\",\n";
            json << "  \"answer\": \"📖 <strong>In-Depth Technical Guide (" << escapeJson(targetRec->category) << ")</strong>:\\n\\n" << escapeJson(targetRec->answerDetailed) << "\",\n";
            json << "  \"child_analogy\": \"" << escapeJson(targetRec->level1Child) << "\",\n";
            json << "  \"real_world\": \"" << escapeJson(targetRec->exampleRealWorld) << "\"\n";
            json << "}";
            return json.str();
        }
    }

    // 3. Search and Retrieval across Knowledge Records
    int bestScore = 0;
    const KnowledgeRecord* bestRec = nullptr;

    for (const auto& rec : *dataset) {
        int s = scoreRecord(rec, userQuery);
        if (s > bestScore) {
            bestScore = s;
            bestRec = &rec;
        }
    }

    // Honest fallback if no good match is found
    if (bestScore <= 20 || bestRec == nullptr) {
        std::ostringstream json;
        json << "{\n";
        json << "  \"status\": \"not_found\",\n";
        json << "  \"category\": \"Honest Fallback\",\n";
        if (curHuman == "hi") {
            json << "  \"answer\": \"मुझे क्षमा करें, आपका विशिष्ट प्रश्न ('<strong>" << escapeJson(userQuery) << "</strong>') वर्तमान OOP Master नॉलेज बेस में सीधे शामिल नहीं है। गलत या मनगढ़ंत जानकारी देने के बजाय मैं पूरी ईमानदारी से यह स्पष्ट करता हूँ।\\n\\nक्या आप हमारे सत्यापित और विस्तृत OOP विषयों में से किसी के बारे में जानना चाहते हैं?\",\n";
        } else if (curHuman == "mr") {
            json << "  \"answer\": \"मला क्षमा करा, आपला विशिष्ट प्रश्न ('<strong>" << escapeJson(userQuery) << "</strong>') सध्या OOP Master च्या नॉलेज बेसमध्ये उपलब्ध नाही. चुकीची माहिती देण्यापेक्षा मी प्रामाणिकपणे हे स्पष्ट करतो.\\n\\nआपण खालीलपैकी कोणत्याही सत्यापित OOP विषयाची सविस्तर माहिती पाहू इच्छिता का?\",\n";
        } else {
            json << "  \"answer\": \"I apologize, but your specific query ('<strong>" << escapeJson(userQuery) << "</strong>') is not currently covered in the OOP Master knowledge base. Rather than inventing an unverified response, I provide an honest fallback.\\n\\nWould you like to explore one of our thoroughly verified Object-Oriented Programming concepts below?\",\n";
        }
        json << "  \"quick_topics\": [\n";
        json << "    {\"label\": \"🏗️ Classes & Objects\", \"query\": \"What is a class and object?\"},\n";
        json << "    {\"label\": \"🧬 Multilevel Inheritance\", \"query\": \"What is multilevel inheritance in C++?\"},\n";
        json << "    {\"label\": \"💎 Diamond Problem\", \"query\": \"What is the diamond problem and virtual inheritance?\"},\n";
        json << "    {\"label\": \"🎭 Runtime Polymorphism\", \"query\": \"What is runtime polymorphism?\"},\n";
        json << "    {\"label\": \"⚡ Virtual Functions & Vtables\", \"query\": \"Why do we use virtual functions?\"},\n";
        json << "    {\"label\": \"🔒 Private Inheritance\", \"query\": \"What is private inheritance?\"},\n";
        json << "    {\"label\": \"📐 SOLID Principles\", \"query\": \"What are SOLID principles?\"}\n";
        json << "  ]\n";
        json << "}";
        return json.str();
    }

    // Determine answer text based on mode
    std::string chosenAnswer = bestRec->answerMedium;
    if (mode == "short" || isShortIntent(userQuery)) chosenAnswer = bestRec->answerShort;
    else if (mode == "child" || isChildIntent(userQuery)) chosenAnswer = bestRec->level1Child;
    else if (mode == "simpler" || isSimplerIntent(userQuery)) chosenAnswer = bestRec->exampleRealWorld;
    else if (mode == "detailed" || isDetailedIntent(userQuery)) chosenAnswer = bestRec->answerDetailed;

    // Get code snippet according to requested programming language
    std::string codeSnippet = getMultiLangCode(bestRec->id, progLang);
    if (codeSnippet.empty()) {
        codeSnippet = bestRec->exampleSimpleCode;
    }

    std::ostringstream json;
    json << "{\n";
    json << "  \"status\": \"success\",\n";
    json << "  \"record_id\": \"" << escapeJson(bestRec->id) << "\",\n";
    json << "  \"category\": \"" << escapeJson(bestRec->category) << "\",\n";
    json << "  \"question\": \"" << escapeJson(bestRec->question) << "\",\n";
    json << "  \"answer\": \"" << escapeJson(chosenAnswer) << "\",\n";
    json << "  \"answer_short\": \"" << escapeJson(bestRec->answerShort) << "\",\n";
    json << "  \"answer_medium\": \"" << escapeJson(bestRec->answerMedium) << "\",\n";
    json << "  \"answer_detailed\": \"" << escapeJson(bestRec->answerDetailed) << "\",\n";
    json << "  \"child_analogy\": \"" << escapeJson(bestRec->level1Child) << "\",\n";
    json << "  \"real_world\": \"" << escapeJson(bestRec->exampleRealWorld) << "\",\n";
    json << "  \"code_simple\": \"" << escapeJson(codeSnippet) << "\",\n";
    json << "  \"code_lang\": \"" << escapeJson(progLang) << "\",\n";
    json << "  \"expected_output\": \"" << escapeJson(bestRec->expectedOutput) << "\",\n";
    json << "  \"follow_up_questions\": [";
    for (size_t i = 0; i < bestRec->followUpQuestions.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << escapeJson(bestRec->followUpQuestions[i]) << "\"";
    }
    json << "]\n";
    json << "}";

    return json.str();
}

std::string readFileContent(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void handleClient(SOCKET clientSocket) {
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';
    std::string req(buffer);

    std::istringstream reqStream(req);
    std::string method, path, proto;
    reqStream >> method >> path >> proto;

    std::string responseHeader;
    std::string responseBody;
    std::string contentType = "text/html; charset=utf-8";

    // Route Handling
    if (path == "/" || path == "/index.html") {
        responseBody = readFileContent("index.html");
        contentType = "text/html; charset=utf-8";
    } else if (path == "/styles.css") {
        responseBody = readFileContent("styles.css");
        contentType = "text/css; charset=utf-8";
    } else if (path == "/script.js") {
        responseBody = readFileContent("script.js");
        contentType = "application/javascript; charset=utf-8";
    } else if (path == "/OOP_Master_English.txt") {
        responseBody = readFileContent("OOP_Master_English.txt");
        contentType = "text/plain; charset=utf-8";
    } else if (path == "/OOP_Master_Hindi.txt") {
        responseBody = readFileContent("OOP_Master_Hindi.txt");
        contentType = "text/plain; charset=utf-8";
    } else if (path == "/OOP_Master_Marathi.txt") {
        responseBody = readFileContent("OOP_Master_Marathi.txt");
        contentType = "text/plain; charset=utf-8";
    } else if (path == "/dark-bg.mp4" || path == "/gemini_generated_video_7c22567c.mp4") {
        responseBody = readFileContent("dark-bg.mp4");
        if (responseBody.empty()) responseBody = readFileContent("gemini_generated_video_7c22567c.mp4");
        contentType = "video/mp4";
    } else if (path == "/light-bg.mp4") {
        responseBody = readFileContent("light-bg.mp4");
        contentType = "video/mp4";
    } else if (path.size() >= 4 && path.substr(path.size() - 4) == ".mp4") {
        std::string fname = urlDecode(path.substr(1));
        responseBody = readFileContent(fname);
        contentType = "video/mp4";
    } else if (path.rfind("/api/chat", 0) == 0) {
        std::string query = "";
        std::string lang = "en";
        std::string mode = "medium";
        std::string codeLang = "cpp";
        std::string topic = "";

        size_t qPos = path.find('?');
        if (qPos != std::string::npos) {
            std::string qString = path.substr(qPos + 1);
            std::istringstream paramStream(qString);
            std::string token;
            while (std::getline(paramStream, token, '&')) {
                size_t eq = token.find('=');
                if (eq != std::string::npos) {
                    std::string key = token.substr(0, eq);
                    std::string val = urlDecode(token.substr(eq + 1));
                    if (key == "q") query = val;
                    else if (key == "lang") lang = val;
                    else if (key == "mode") mode = val;
                    else if (key == "code_lang") codeLang = val;
                    else if (key == "topic") topic = val;
                }
            }
        }
        responseBody = retrieveAnswer(query, lang, mode, codeLang, topic);
        contentType = "application/json; charset=utf-8";
    } else if (path == "/api/stats") {
        std::ostringstream ss;
        ss << "{\n"
           << "  \"status\": \"running\",\n"
           << "  \"server\": \"OOP Master Native C++ Engine\",\n"
           << "  \"english_records\": " << g_enRecords.size() << ",\n"
           << "  \"hindi_records\": " << g_hiRecords.size() << ",\n"
           << "  \"marathi_records\": " << g_mrRecords.size() << ",\n"
           << "  \"total_records\": " << (g_enRecords.size() + g_hiRecords.size() + g_mrRecords.size()) << "\n"
           << "}";
        responseBody = ss.str();
        contentType = "application/json; charset=utf-8";
    } else {
        responseBody = "<h1>404 Not Found</h1>";
        contentType = "text/html; charset=utf-8";
    }

    std::ostringstream head;
    if (responseBody.empty() && path != "/" && path != "/index.html") {
        head << "HTTP/1.1 404 Not Found\r\n"
             << "Content-Length: 0\r\n"
             << "Connection: close\r\n\r\n";
        std::string h = head.str();
        send(clientSocket, h.c_str(), (int)h.size(), 0);
        closesocket(clientSocket);
        return;
    }

    head << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: " << contentType << "\r\n"
         << "Content-Length: " << responseBody.size() << "\r\n"
         << "Accept-Ranges: bytes\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "Connection: close\r\n\r\n";

    std::string headerStr = head.str();
    send(clientSocket, headerStr.c_str(), (int)headerStr.size(), 0);

    if (method != "HEAD") {
        // Send payload in chunks to prevent socket buffer truncation on large binary video files
        const char* dataPtr = responseBody.data();
        size_t totalBytes = responseBody.size();
        size_t totalSent = 0;
        while (totalSent < totalBytes) {
            int chunkSize = (int)std::min<size_t>(65536, totalBytes - totalSent);
            int sent = send(clientSocket, dataPtr + totalSent, chunkSize, 0);
            if (sent <= 0) break;
            totalSent += sent;
        }
    }
    closesocket(clientSocket);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    std::cout << "========================================================\n";
    std::cout << "  OOP MASTER: C++ KNOWLEDGE ENGINE & WEB SERVER        \n";
    std::cout << "========================================================\n\n";

    std::cout << "[1/3] Parsing English Knowledge Base (OOP_Master_English.txt)...\n";
    if (parseKnowledgeFile("OOP_Master_English.txt", g_enRecords)) {
        std::cout << "      -> Successfully loaded " << g_enRecords.size() << " records.\n";
    } else {
        std::cerr << "      -> [Error] Could not load English file!\n";
    }

    std::cout << "[2/3] Parsing Hindi Knowledge Base (OOP_Master_Hindi.txt)...\n";
    if (parseKnowledgeFile("OOP_Master_Hindi.txt", g_hiRecords)) {
        std::cout << "      -> Successfully loaded " << g_hiRecords.size() << " records.\n";
    } else {
        std::cerr << "      -> [Error] Could not load Hindi file!\n";
    }

    std::cout << "[3/3] Parsing Marathi Knowledge Base (OOP_Master_Marathi.txt)...\n";
    if (parseKnowledgeFile("OOP_Master_Marathi.txt", g_mrRecords)) {
        std::cout << "      -> Successfully loaded " << g_mrRecords.size() << " records.\n";
    } else {
        std::cerr << "      -> [Error] Could not load Marathi file!\n";
    }

    std::cout << "\n[C++ Engine] Total records in-memory: "
              << (g_enRecords.size() + g_hiRecords.size() + g_mrRecords.size()) << "\n";

    // Initialize WinSock
    WSADATA wsaData;
    int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsResult != 0) {
        std::cerr << "WSAStartup failed with error: " << wsResult << "\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    BOOL optval = TRUE;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(port);

    if (bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "\n>>> OOP MASTER C++ SERVER RUNNING LIVE ON http://localhost:" << port << " <<<\n";
    std::cout << "Ready to process multilingual user queries with in-memory scoring engine...\n\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            handleClient(clientSocket);
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
