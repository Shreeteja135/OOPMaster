#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

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

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool parseKnowledgeFile(const std::string& filename, std::vector<KnowledgeRecord>& records) {
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return false;
    }

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
                // End of block
                std::string blockContent = "";
                for (size_t i = 0; i < blockLines.size(); ++i) {
                    std::string bl = blockLines[i];
                    if (!bl.empty() && bl[0] == '\\') bl = bl.substr(1);
                    if (i > 0) blockContent += "\n";
                    blockContent += bl;
                }

                if (currentBlock == "ALTERNATE_QUESTIONS") {
                    for (const auto& bl : blockLines) {
                        current.alternateQuestions.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                    }
                } else if (currentBlock == "SEARCH_TERMS") {
                    for (const auto& bl : blockLines) {
                        current.searchTerms.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                    }
                } else if (currentBlock == "COMMON_MISTAKES") {
                    for (const auto& bl : blockLines) {
                        current.commonMistakes.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                    }
                } else if (currentBlock == "RELATED_TOPICS") {
                    for (const auto& bl : blockLines) {
                        current.relatedTopics.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                    }
                } else if (currentBlock == "FOLLOW_UP_QUESTIONS") {
                    for (const auto& bl : blockLines) {
                        current.followUpQuestions.push_back(bl[0] == '\\' ? bl.substr(1) : bl);
                    }
                } else if (currentBlock == "ANSWER_SHORT") {
                    current.answerShort = blockContent;
                } else if (currentBlock == "ANSWER_MEDIUM") {
                    current.answerMedium = blockContent;
                } else if (currentBlock == "ANSWER_DETAILED") {
                    current.answerDetailed = blockContent;
                } else if (currentBlock == "LEVEL_1_CHILD") {
                    current.level1Child = blockContent;
                } else if (currentBlock == "LEVEL_2_ABSOLUTE_BEGINNER") {
                    current.level2Beginner = blockContent;
                } else if (currentBlock == "LEVEL_3_SCHOOL_STUDENT") {
                    current.level3School = blockContent;
                } else if (currentBlock == "LEVEL_4_COLLEGE_BEGINNER") {
                    current.level4College = blockContent;
                } else if (currentBlock == "LEVEL_5_INTERMEDIATE") {
                    current.level5Intermediate = blockContent;
                } else if (currentBlock == "LEVEL_6_ADVANCED") {
                    current.level6Advanced = blockContent;
                } else if (currentBlock == "LEVEL_7_EXPERT") {
                    current.level7Expert = blockContent;
                } else if (currentBlock == "EXAMPLE_REAL_WORLD") {
                    current.exampleRealWorld = blockContent;
                } else if (currentBlock == "EXAMPLE_SIMPLE_CODE") {
                    current.exampleSimpleCode = blockContent;
                } else if (currentBlock == "EXAMPLE_MEDIUM_CODE") {
                    current.exampleMediumCode = blockContent;
                } else if (currentBlock == "EXAMPLE_ADVANCED_CODE") {
                    current.exampleAdvancedCode = blockContent;
                } else if (currentBlock == "CODE_EXPLANATION") {
                    current.codeExplanation = blockContent;
                } else if (currentBlock == "EXPECTED_OUTPUT") {
                    current.expectedOutput = blockContent;
                } else if (currentBlock == "SOURCE_NOTES") {
                    current.sourceNotes = blockContent;
                }

                currentBlock = "";
                blockLines.clear();
            } else {
                blockLines.push_back(line);
            }
        } else {
            if (line.size() >= 2 && line.substr(line.size() - 2) == "<<") {
                currentBlock = line.substr(0, line.size() - 2);
                blockLines.clear();
            } else {
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
        }
    }
    return true;
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  OOP MASTER KNOWLEDGE BASE C++ VALIDATION ENGINE  \n";
    std::cout << "====================================================\n";

    const char* files[] = {
        "OOP_Master_English.txt",
        "OOP_Master_Hindi.txt",
        "OOP_Master_Marathi.txt"
    };

    for (int i = 0; i < 3; ++i) {
        std::vector<KnowledgeRecord> records;
        std::cout << "\nParsing " << files[i] << " with native C++ parser..." << std::endl;
        if (!parseKnowledgeFile(files[i], records)) {
            std::cerr << "Failed to parse " << files[i] << std::endl;
            return 1;
        }

        std::cout << "  -> Successfully parsed " << records.size() << " records.\n";
        for (size_t r = 0; r < records.size(); ++r) {
            const auto& rec = records[r];
            std::cout << "  [" << (r + 1) << "] ID: " << rec.id 
                      << " | Category: " << rec.category 
                      << " | Lang: " << rec.humanLang
                      << " | Code: " << (rec.exampleSimpleCode.empty() ? "NO" : "YES")
                      << " | Output: " << (rec.expectedOutput.empty() ? "NO" : "YES")
                      << std::endl;
            if (rec.id.empty() || rec.answerShort.empty() || rec.level1Child.empty()) {
                std::cerr << "ERROR: Incomplete fields in record " << rec.id << std::endl;
                return 1;
            }
        }
    }

    std::cout << "\nSUCCESS: All 3 files parsed and validated with C++ parser!\n";
    return 0;
}

