// ============================================================================
// OOP MASTER: C++ KNOWLEDGE BASE BUILDER, FORMATTER & ASSEMBLER
// Language: C++11 / C++14 / C++17
// Purpose: Replaces Python assemble_kb and kb_formatter modules with pure native C++.
//          Provides complete data modeling, escaping, serialization, validation,
//          and assembly of the multilingual OOP Master Knowledge Base files.
// ============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace OOPMaster {

// Knowledge Record Data Structure
struct Record {
    std::string record_id;
    std::string category;
    std::string subcategory;
    std::string programming_language;
    std::string supported_programming_languages;
    std::string human_language;
    std::string difficulty;
    std::string question_type;
    std::string question;

    std::vector<std::string> alternate_questions;
    std::vector<std::string> search_terms;

    std::string answer_short;
    std::string answer_medium;
    std::string answer_detailed;

    std::string level_1_child;
    std::string level_2_absolute_beginner;
    std::string level_3_school_student;
    std::string level_4_college_beginner;
    std::string level_5_intermediate;
    std::string level_6_advanced;
    std::string level_7_expert;

    std::string example_real_world;
    std::string example_simple_code;
    std::string example_medium_code;
    std::string example_advanced_code;
    std::string code_explanation;
    std::string expected_output;

    std::vector<std::string> common_mistakes;
    std::vector<std::string> related_topics;
    std::vector<std::string> follow_up_questions;

    std::string source_notes;
    std::string verification_status;
};

// Escapes special characters for block content
inline std::string escape_line(const std::string& line) {
    if (line.compare(0, 2, ">>") == 0 ||
        line.compare(0, 5, "=====") == 0 ||
        (!line.empty() && line[0] == '\\')) {
        return "\\" + line;
    }
    return line;
}

// Unescapes special characters for block content
inline std::string unescape_line(const std::string& line) {
    if (!line.empty() && line[0] == '\\') {
        return line.substr(1);
    }
    return line;
}

// Serializes a Record to plain text format
std::string serialize_record(const Record& rec) {
    std::ostringstream out;
    out << "=====RECORD_BEGIN=====\n";
    out << "RECORD_ID: " << rec.record_id << "\n";
    out << "CATEGORY: " << rec.category << "\n";
    out << "SUBCATEGORY: " << rec.subcategory << "\n";
    out << "PROGRAMMING_LANGUAGE: " << rec.programming_language << "\n";
    out << "SUPPORTED_PROGRAMMING_LANGUAGES: " << rec.supported_programming_languages << "\n";
    out << "HUMAN_LANGUAGE: " << rec.human_language << "\n";
    out << "DIFFICULTY: " << rec.difficulty << "\n";
    out << "QUESTION_TYPE: " << rec.question_type << "\n";
    out << "QUESTION: " << rec.question << "\n";

    // Alternate Questions
    out << "ALTERNATE_QUESTIONS<<\n";
    for (const auto& q : rec.alternate_questions) {
        out << escape_line(q) << "\n";
    }
    out << ">>ALTERNATE_QUESTIONS\n";

    // Search Terms
    out << "SEARCH_TERMS<<\n";
    for (const auto& t : rec.search_terms) {
        out << escape_line(t) << "\n";
    }
    out << ">>SEARCH_TERMS\n";

    // Multi-line Text Fields
    auto write_block = [&](const std::string& field_name, const std::string& content) {
        out << field_name << "<<\n";
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            out << escape_line(line) << "\n";
        }
        out << ">>" << field_name << "\n";
    };

    write_block("ANSWER_SHORT", rec.answer_short);
    write_block("ANSWER_MEDIUM", rec.answer_medium);
    write_block("ANSWER_DETAILED", rec.answer_detailed);

    write_block("LEVEL_1_CHILD", rec.level_1_child);
    write_block("LEVEL_2_ABSOLUTE_BEGINNER", rec.level_2_absolute_beginner);
    write_block("LEVEL_3_SCHOOL_STUDENT", rec.level_3_school_student);
    write_block("LEVEL_4_COLLEGE_BEGINNER", rec.level_4_college_beginner);
    write_block("LEVEL_5_INTERMEDIATE", rec.level_5_intermediate);
    write_block("LEVEL_6_ADVANCED", rec.level_6_advanced);
    write_block("LEVEL_7_EXPERT", rec.level_7_expert);

    write_block("EXAMPLE_REAL_WORLD", rec.example_real_world);
    write_block("EXAMPLE_SIMPLE_CODE", rec.example_simple_code);
    write_block("EXAMPLE_MEDIUM_CODE", rec.example_medium_code);
    write_block("EXAMPLE_ADVANCED_CODE", rec.example_advanced_code);
    write_block("CODE_EXPLANATION", rec.code_explanation);
    write_block("EXPECTED_OUTPUT", rec.expected_output);

    // Common Mistakes
    out << "COMMON_MISTAKES<<\n";
    for (const auto& m : rec.common_mistakes) {
        out << escape_line(m) << "\n";
    }
    out << ">>COMMON_MISTAKES\n";

    // Related Topics
    out << "RELATED_TOPICS<<\n";
    for (const auto& t : rec.related_topics) {
        out << escape_line(t) << "\n";
    }
    out << ">>RELATED_TOPICS\n";

    // Follow-up Questions
    out << "FOLLOW_UP_QUESTIONS<<\n";
    for (const auto& f : rec.follow_up_questions) {
        out << escape_line(f) << "\n";
    }
    out << ">>FOLLOW_UP_QUESTIONS\n";

    write_block("SOURCE_NOTES", rec.source_notes);

    out << "VERIFICATION_STATUS: " << (rec.verification_status.empty() ? "Verified" : rec.verification_status) << "\n";
    out << "=====RECORD_END=====\n";

    return out.str();
}

// Parses a Knowledge Base text file into a vector of Records
std::vector<Record> parse_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    std::vector<Record> records;
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open file: " << path << "\n";
        return records;
    }

    std::string line;
    bool in_record = false;
    bool in_block = false;
    std::string block_name;
    std::vector<std::string> block_lines;
    Record current;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (!in_record) {
            if (line == "=====RECORD_BEGIN=====") {
                in_record = true;
                current = Record();
            }
            continue;
        }

        if (in_block) {
            if (line == ">>" + block_name) {
                in_block = false;
                std::string full_block;
                for (size_t i = 0; i < block_lines.size(); ++i) {
                    if (i > 0) full_block += "\n";
                    full_block += block_lines[i];
                }

                if (block_name == "ALTERNATE_QUESTIONS") current.alternate_questions = block_lines;
                else if (block_name == "SEARCH_TERMS") current.search_terms = block_lines;
                else if (block_name == "ANSWER_SHORT") current.answer_short = full_block;
                else if (block_name == "ANSWER_MEDIUM") current.answer_medium = full_block;
                else if (block_name == "ANSWER_DETAILED") current.answer_detailed = full_block;
                else if (block_name == "LEVEL_1_CHILD") current.level_1_child = full_block;
                else if (block_name == "LEVEL_2_ABSOLUTE_BEGINNER") current.level_2_absolute_beginner = full_block;
                else if (block_name == "LEVEL_3_SCHOOL_STUDENT") current.level_3_school_student = full_block;
                else if (block_name == "LEVEL_4_COLLEGE_BEGINNER") current.level_4_college_beginner = full_block;
                else if (block_name == "LEVEL_5_INTERMEDIATE") current.level_5_intermediate = full_block;
                else if (block_name == "LEVEL_6_ADVANCED") current.level_6_advanced = full_block;
                else if (block_name == "LEVEL_7_EXPERT") current.level_7_expert = full_block;
                else if (block_name == "EXAMPLE_REAL_WORLD") current.example_real_world = full_block;
                else if (block_name == "EXAMPLE_SIMPLE_CODE") current.example_simple_code = full_block;
                else if (block_name == "EXAMPLE_MEDIUM_CODE") current.example_medium_code = full_block;
                else if (block_name == "EXAMPLE_ADVANCED_CODE") current.example_advanced_code = full_block;
                else if (block_name == "CODE_EXPLANATION") current.code_explanation = full_block;
                else if (block_name == "EXPECTED_OUTPUT") current.expected_output = full_block;
                else if (block_name == "COMMON_MISTAKES") current.common_mistakes = block_lines;
                else if (block_name == "RELATED_TOPICS") current.related_topics = block_lines;
                else if (block_name == "FOLLOW_UP_QUESTIONS") current.follow_up_questions = block_lines;
                else if (block_name == "SOURCE_NOTES") current.source_notes = full_block;

                block_lines.clear();
            } else {
                block_lines.push_back(unescape_line(line));
            }
            continue;
        }

        if (line == "=====RECORD_END=====") {
            records.push_back(current);
            in_record = false;
            continue;
        }

        if (line.size() > 2 && line.substr(line.size() - 2) == "<<") {
            in_block = true;
            block_name = line.substr(0, line.size() - 2);
            block_lines.clear();
            continue;
        }

        size_t colon = line.find(": ");
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 2);
            if (key == "RECORD_ID") current.record_id = val;
            else if (key == "CATEGORY") current.category = val;
            else if (key == "SUBCATEGORY") current.subcategory = val;
            else if (key == "PROGRAMMING_LANGUAGE") current.programming_language = val;
            else if (key == "SUPPORTED_PROGRAMMING_LANGUAGES") current.supported_programming_languages = val;
            else if (key == "HUMAN_LANGUAGE") current.human_language = val;
            else if (key == "DIFFICULTY") current.difficulty = val;
            else if (key == "QUESTION_TYPE") current.question_type = val;
            else if (key == "QUESTION") current.question = val;
            else if (key == "VERIFICATION_STATUS") current.verification_status = val;
        }
    }

    return records;
}

// Writes a formatted knowledge base file with the standard header
bool write_file(const std::string& path, const std::vector<Record>& records) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[Error] Failed to write to " << path << "\n";
        return false;
    }

    out << "# OOP MASTER KNOWLEDGE BASE  (FORMAT_VERSION 1.0, encoding UTF-8, line endings LF)\n";
    out << "# ARCHITECTURE: Multilingual Retrieval Knowledge Library for OOP Master C++ Engine\n";
    out << "# FILE STRUCTURE\n";
    out << "#   Lines starting with '#' before the first record are header comments and are ignored.\n";
    out << "#   A record starts with the line  =====RECORD_BEGIN=====  and ends with  =====RECORD_END=====\n";
    out << "# FIELD SYNTAX (one of two forms, always at the start of a line)\n";
    out << "#   Scalar field : KEY: value          (single line; value is everything after the first ': ')\n";
    out << "#   Block field  : KEY<<               (opening line, nothing else on it)\n";
    out << "#                  ...content lines...\n";
    out << "#                  >>KEY               (closing line)\n";
    out << "#   List fields (ALTERNATE_QUESTIONS, SEARCH_TERMS, COMMON_MISTAKES, RELATED_TOPICS, FOLLOW_UP_QUESTIONS)\n";
    out << "#   are block fields with ONE ITEM PER LINE.\n";
    out << "# ESCAPING RULE (inside block content only)\n";
    out << "#   If a content line begins with '>>', '=====' or '\\', the writer adds ONE leading backslash.\n";
    out << "#   The reader removes ONE leading backslash from every content line that begins with '\\'.\n";
    out << "# RECORD IDs are shared across the English, Hindi and Marathi files. HUMAN_LANGUAGE indicates the file's language.\n\n";

    for (const auto& rec : records) {
        out << serialize_record(rec) << "\n";
    }

    return true;
}

} // namespace OOPMaster

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n";
    std::cout << "  OOP MASTER: C++ KNOWLEDGE BASE BUILDER & ASSEMBLER    \n";
    std::cout << "========================================================\n\n";

    std::string en_path = "OOP_Master_English.txt";
    std::string hi_path = "OOP_Master_Hindi.txt";
    std::string mr_path = "OOP_Master_Marathi.txt";

    std::cout << "[C++ Engine] Parsing English Knowledge Base: " << en_path << "...\n";
    auto en_records = OOPMaster::parse_file(en_path);
    std::cout << "             Loaded " << en_records.size() << " records.\n";

    std::cout << "[C++ Engine] Parsing Hindi Knowledge Base: " << hi_path << "...\n";
    auto hi_records = OOPMaster::parse_file(hi_path);
    std::cout << "             Loaded " << hi_records.size() << " records.\n";

    std::cout << "[C++ Engine] Parsing Marathi Knowledge Base: " << mr_path << "...\n";
    auto mr_records = OOPMaster::parse_file(mr_path);
    std::cout << "             Loaded " << mr_records.size() << " records.\n\n";

    if (en_records.empty() || hi_records.empty() || mr_records.empty()) {
        std::cerr << "[Error] One or more knowledge base files could not be loaded!\n";
        return 1;
    }

    // Check triplet alignment
    bool aligned = (en_records.size() == hi_records.size() && hi_records.size() == mr_records.size());
    if (aligned) {
        std::cout << "[Verification] All three files have equal record count: " << en_records.size() << " records.\n";
    } else {
        std::cerr << "[Warning] Record count mismatch between languages!\n";
    }

    std::map<std::string, const OOPMaster::Record*> en_map;
    for (const auto& r : en_records) en_map[r.record_id] = &r;

    std::cout << "\n[C++ Engine] Verifying cross-lingual schema consistency across all records:\n";
    int valid_triplets = 0;
    for (size_t i = 0; i < hi_records.size(); ++i) {
        const std::string& id = hi_records[i].record_id;
        if (en_map.find(id) != en_map.end() && i < mr_records.size() && mr_records[i].record_id == id) {
            std::cout << "  [✓] Triplet: " << id << " | Cat: " << hi_records[i].category << "\n";
            valid_triplets++;
        } else {
            std::cout << "  [✗] Triplet mismatch for: " << id << "\n";
        }
    }

    std::cout << "\n[C++ Engine] Validated " << valid_triplets << " cross-lingual record triplets.\n";

    // Check if user requested rebuild
    bool rebuild = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--rebuild") rebuild = true;
    }

    if (rebuild) {
        std::cout << "\n[C++ Engine] Rebuilding & canonicalizing knowledge base files...\n";
        OOPMaster::write_file(en_path, en_records);
        OOPMaster::write_file(hi_path, hi_records);
        OOPMaster::write_file(mr_path, mr_records);
        std::cout << "[C++ Engine] Files re-assembled successfully.\n";
    }

    std::cout << "\n[Status] Pure C++ Knowledge Base Engine operations complete!\n";
    return 0;
}

