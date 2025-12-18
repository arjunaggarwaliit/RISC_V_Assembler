#include <bits/stdc++.h>
using namespace std;

static const uint32_t textbaseaddr = 0x00000000u;
static const uint32_t databaseaddr = 0x10000000u;

struct LineInfo {
    string raw;
    bool isLabelOnly;
    bool inText;
    bool inData;
    uint32_t addr;
};

string linetrimmer(string s) {
    int start = s.find_first_not_of(" \t\n\r");
    int end = s.find_last_not_of(" \t\n\r");
    if (start == string::npos || end == string::npos) return "";
    return s.substr(start, end - start + 1);
}

bool checkimmediate(const string &s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    if (i < s.size() && s[i] == '0' && (i+1 < s.size()) && (s[i+1]=='x' || s[i+1]=='X')) return true;
    return isdigit(s[i]);
}

long datavalue(const string &token) {
    string t = token;
    if (!t.empty() && t.front() == '(' && t.back() == ')')
        t = t.substr(1, t.size() - 2);

    auto trim = [](string s) {
        int start = s.find_first_not_of(" \t\n");
        int end = s.find_last_not_of(" \t\n");
        if (start == string::npos || end == string::npos) return string("");
        return s.substr(start, end - start + 1);
    };
    t = trim(t);
    if (t.empty()) return 0;

    size_t i = 0;
    if (t[i] == '+' || t[i] == '-') i++;
    if (i < t.size() && t[i] == '0' && (i + 1 < t.size()) &&
        (t[i + 1] == 'x' || t[i + 1] == 'X')) {
        i += 2; // skip 0x
        while (i < t.size() && isxdigit(t[i])) i++;
    } else {
        while (i < t.size() && isdigit(t[i])) i++;
    }

    if (i != t.size()) 
    return 0;

    return stol(t, nullptr, 0);
}


string u32tohex32(uint32_t v) {
    std::ostringstream oss;
    oss << "0x" << uppercase << hex << setw(8) << setfill('0') << (v & 0xFFFFFFFFu);
    return oss.str();
}

void printformatter(ostream &out, uint32_t pc, uint32_t instrWord, const string &full_instruction) {
    out << "0x" << hex << uppercase << pc;
    out << " " << u32tohex32(instrWord) << " , " << full_instruction << " # ";
}

void encoding_R_format(ostream &out, map<string,string>&opcode, map<string,string>& funct3,
    map<string,string>& funct7, int rd, int rs1, int rs2, string instruction, string full_instruction, uint32_t pc) {
    uint32_t rep = 0;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    rep |= opcode_;
    rep |= ((rd & 0x1F) << 7);
    uint32_t funct3_ = stoul(funct3[instruction], nullptr, 2) & 0x7u;
    rep |= (funct3_ << 12);
    rep |= ((rs1 & 0x1F) << 15);
    rep |= ((rs2 & 0x1F) << 20);
    uint32_t funct7_ = stoul(funct7[instruction], nullptr, 2) & 0x7Fu;
    rep |= (funct7_ << 25);
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << funct3[instruction] << "-";
    out << funct7[instruction] << "-";
    out << bitset<5>(rd & 0x1F).to_string() << "-";
    out << bitset<5>(rs1 & 0x1F).to_string() << "-";
    out << bitset<5>(rs2 & 0x1F).to_string() << "-";
    out << "NULL\n";
}

void encoding_R_format_wrapper(ostream &out, int rd, int rs1, int rs2, string instruction, string full_instruction, uint32_t pc) {
    map<string, string> opcode = {
        {"add",  "0110011"},
        {"addw", "0111011"},
        {"and",  "0110011"},
        {"or",   "0110011"},
        {"sll",  "0110011"},
        {"slt",  "0110011"},
        {"sra",  "0110011"},
        {"srl",  "0110011"},
        {"sub",  "0110011"},
        {"subw", "0111011"},
        {"xor",  "0110011"},
        {"mul",  "0110011"},
        {"mulw", "0111011"},
        {"div",  "0110011"},
        {"divw", "0111011"},
        {"rem",  "0110011"},
        {"remw", "0111011"}
    };
    map<string, string> funct3 = {
        {"add",  "000"},
        {"addw", "000"},
        {"and",  "111"},
        {"or",   "110"},
        {"sll",  "001"},
        {"slt",  "010"},
        {"sra",  "101"},
        {"srl",  "101"},
        {"sub",  "000"},
        {"subw", "000"},
        {"xor",  "100"},
        {"mul",  "000"},
        {"mulw", "000"},
        {"div",  "100"},
        {"divw", "100"},
        {"rem",  "110"},
        {"remw", "110"}
    };
    map<string, string> funct7 = {
        {"add",  "0000000"},
        {"addw", "0000000"},
        {"and",  "0000000"},
        {"or",   "0000000"},
        {"sll",  "0000000"},
        {"slt",  "0000000"},
        {"sra",  "0100000"},
        {"srl",  "0000000"},
        {"sub",  "0100000"},
        {"subw", "0100000"},
        {"xor",  "0000000"},
        {"mul",  "0000001"},
        {"mulw", "0000001"},
        {"div",  "0000001"},
        {"divw", "0000001"},
        {"rem",  "0000001"},
        {"remw", "0000001"}
    };
    encoding_R_format(out, opcode, funct3, funct7, rd, rs1, rs2, instruction, full_instruction, pc);
}

void encoding_I_format(ostream &out, map<string,string>&opcode, map<string,string>& funct3, int rd, int rs1, int immediate,
    string instruction, string full_instruction, uint32_t pc){
    uint32_t rep = 0;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    rep |= opcode_;
    rep |= ((rd & 0x1F) << 7);
    uint32_t funct3_ = stoul(funct3[instruction], nullptr, 2) & 0x7u;
    rep |= (funct3_ << 12);
    rep |= ((rs1 & 0x1F) << 15);
    uint32_t imm12 = (uint32_t) immediate & 0xFFFu;
    rep |= (imm12 << 20);
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << funct3[instruction] << "-";
    out << "NULL-";
    out << bitset<5>(rd & 0x1F).to_string() << "-";
    out << bitset<5>(rs1 & 0x1F).to_string() << "-";
    out << "NULL-";
    out << bitset<12>(imm12).to_string() << "\n";
}

void encoding_I_format_wrapper(ostream &out, int rd, int rs1, int immediate, string instruction, string full_instruction, uint32_t pc){
    map<string, string> opcode = {
        {"addi", "0010011"},
        {"addiw", "0011011"},
        {"andi", "0010011"},
        {"ori", "0010011"},
        {"lb", "0000011"},
        {"lh", "0000011"},
        {"lw", "0000011"},
        {"ld", "0000011"},
        {"jalr", "1100111"}
    };
    map<string, string> funct3 = {
        {"addi", "000"},
        {"addiw", "000"},
        {"andi", "111"},
        {"ori", "110"},
        {"lb", "000"},
        {"lh", "001"},
        {"lw", "010"},
        {"ld", "011"},
        {"jalr", "000"}
    };
    encoding_I_format(out, opcode, funct3, rd, rs1, immediate, instruction, full_instruction, pc);
}

void encoding_S_format(ostream &out, map<string,string>&opcode, map<string,string>& funct3, int rs1, int rs2, int immediate,
    string instruction, string full_instruction, uint32_t pc){
    uint32_t imm = (uint32_t) immediate;
    uint32_t part1 = imm & 0x1F;
    uint32_t part2 = (imm >> 5) & 0x7F;
    uint32_t rep = 0;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    rep |= opcode_;
    rep |= (part1 << 7);
    uint32_t funct3_ = stoul(funct3[instruction], nullptr, 2) & 0x7u;
    rep |= (funct3_ << 12);
    rep |= ((rs1 & 0x1F) << 15);
    rep |= ((rs2 & 0x1F) << 20);
    rep |= (part2 << 25);
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << funct3[instruction] << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<5>(rs1 & 0x1F).to_string() << "-";
    out << bitset<5>(rs2 & 0x1F).to_string() << "-";
    out << bitset<12>(imm & 0xFFFu).to_string() << "\n";
}

void encoding_S_format_wrapper(ostream &out, int rs1, int rs2, int immediate, string instruction, string full_instruction, uint32_t pc){
    map<string, string> opcode = {
        {"sb", "0100011"},
        {"sh", "0100011"},
        {"sw", "0100011"},
        {"sd", "0100011"}
    };
    map<string, string> funct3 = {
        {"sb", "000"},
        {"sh", "001"},
        {"sw", "010"},
        {"sd", "011"}
    };
    encoding_S_format(out, opcode, funct3, rs1, rs2, immediate, instruction, full_instruction, pc);
}

void encoding_SB_format(ostream &out, map<string,string>& opcode, map<string,string>& funct3, int rs1, int rs2, int immediate,
    string instruction, string full_instruction, uint32_t pc) {
    uint32_t imm = (uint32_t) immediate;
    uint32_t imm_11 = (imm >> 11) & 0x1;
    uint32_t imm_4_1 = (imm >> 1) & 0xF;
    uint32_t imm_10_5 = (imm >> 5) & 0x3F;
    uint32_t imm_12 = (imm >> 12) & 0x1;
    uint32_t rep = 0;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    rep |= opcode_;
    rep |= (imm_11 << 7);
    rep |= (imm_4_1 << 8);
    uint32_t funct3_ = stoul(funct3[instruction], nullptr , 2) & 0x7u;
    rep |= (funct3_ << 12);
    rep |= ((rs1 & 0x1F) << 15);
    rep |= ((rs2 & 0x1F) << 20);
    rep |= (imm_10_5 << 25);
    rep |= (imm_12 << 31);
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << funct3[instruction] << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<5>(rs1 & 0x1F).to_string() << "-";
    out << bitset<5>(rs2 & 0x1F).to_string() << "-";
    out << bitset<13>(imm & 0x1FFFu).to_string() << "\n";
}

void encoding_SB_format_wrapper(ostream &out, int rs1, int rs2, int immediate, string instruction, string full_instruction, uint32_t pc) {
    map<string, string> opcode = {
        {"beq", "1100011"},
        {"bne", "1100011"},
        {"blt", "1100011"},
        {"bge", "1100011"}
    };
    map<string, string> funct3 = {
        {"beq", "000"},
        {"bne", "001"},
        {"blt", "100"},
        {"bge", "101"}
    };
    encoding_SB_format(out, opcode, funct3, rs1, rs2, immediate, instruction, full_instruction, pc);
}

void encoding_U_format(ostream &out, map<string,string>& opcode, int rd, int immediate, string instruction, string full_instruction, uint32_t pc) {
    uint32_t imm = (uint32_t) immediate;
    uint32_t rep = 0;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    uint32_t imm_upper = imm & 0xFFFFF000u;
    rep |= imm_upper;
    rep |= ((rd & 0x1F) << 7);
    rep |= opcode_;
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<5>(rd & 0x1F).to_string() << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<20>((imm >> 12) & 0xFFFFFu).to_string() << "\n";
}

void encoding_U_format_wrapper(ostream &out, int rd, int immediate, string instruction, string full_instruction, uint32_t pc) {
    map<string, string> opcode = {
        {"lui", "0110111"},
        {"auipc", "0010111"}
    };
    encoding_U_format(out, opcode, rd, immediate, instruction, full_instruction, pc);
}

void encoding_UJ_format(ostream &out, map<string,string>& opcode, int rd, int immediate,
    string instruction, string full_instruction, uint32_t pc) {
    uint32_t imm = (uint32_t) immediate;
    uint32_t opcode_ = stoul(opcode[instruction], nullptr, 2) & 0x7Fu;
    uint32_t rep = 0;
    uint32_t imm20   = (imm >> 20) & 0x1;
    uint32_t imm10_1 = (imm >> 1)  & 0x3FF;
    uint32_t imm11   = (imm >> 11) & 0x1;
    uint32_t imm19_12= (imm >> 12) & 0xFF;
    rep |= (opcode_ & 0x7F);
    rep |= ((rd & 0x1F) << 7);
    rep |= ((imm19_12 & 0xFF) << 12);
    rep |= ((imm11 & 0x1) << 20);
    rep |= ((imm10_1 & 0x3FF) << 21);
    rep |= ((imm20 & 0x1) << 31);
    printformatter(out, pc, rep, full_instruction);
    out << opcode[instruction] << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<5>(rd & 0x1F).to_string() << "-";
    out << "NULL-";
    out << "NULL-";
    out << bitset<21>(imm & 0x1FFFFFu).to_string() << "\n";
}

void encoding_UJ_format_wrapper(ostream &out, int rd, int immediate, string instruction, string full_instruction, uint32_t pc) {
    map<string, string> opcode = {
        {"jal", "1101111"}
    };
    encoding_UJ_format(out, opcode, rd, immediate, instruction, full_instruction, pc);
}

void process_text_instruction(ostream &out, const string &line, uint32_t pc,
    unordered_map<string,uint32_t> &labelAddress) {
    string readline = line;
    for (char &c : readline) if (c == ',') c = ' ';
    stringstream ss(readline);
    string instr;
    ss >> instr;
    if (instr.empty()) return;
    auto getReg = [&](const string &s)->int {
        string t = linetrimmer(s);
        if (t.empty()) return 0;
        if (t[0]=='x' || t[0]=='X') return stoi(t.substr(1));
        return stoi(t);
    };
    string a, b, c;
    ss >> a >> b >> c;
    set<string> R_type = {"add","addw","and","or","sll","slt","sra","srl","sub","subw","xor","mul","mulw","div","divw","rem","remw"};
    set<string> I_type = {"addi","addiw","andi","ori","lb","lh","lw","ld","jalr"};
    set<string> S_type = {"sb","sh","sw","sd"};
    set<string> SB_type = {"beq","bne","blt","bge"};
    set<string> U_type = {"lui","auipc"};
    set<string> UJ_type = {"jal"};
        if (R_type.count(instr)) {
            int rd = getReg(a);
            int rs1 = getReg(b);
            int rs2 = getReg(c);
            encoding_R_format_wrapper(out, rd, rs1, rs2, instr, line, pc);
        } else if (I_type.count(instr)) {
            int rd = getReg(a);
            int rs1 = getReg(b);
            long imm = 0;
            if (c.size()>0 && !checkimmediate(c)) {
                if (labelAddress.count(c)) imm = (long)labelAddress[c];
                else imm = 0;
            } else {
                imm = datavalue(c);
            }
            encoding_I_format_wrapper(out, rd, rs1, (int)imm, instr, line, pc);
        } else if (S_type.count(instr)) {
            int rs2 = getReg(a);
            int rs1 = getReg(b);
            long imm = 0;
            if (!checkimmediate(c)) {
                if (labelAddress.count(c)) imm = (long)labelAddress[c];
                else imm = 0;
            } else imm = datavalue(c);
            encoding_S_format_wrapper(out, rs1, rs2, (int)imm, instr, line, pc);
        } else if (SB_type.count(instr)) {
            int rs1 = getReg(a);
            int rs2 = getReg(b);
            long imm = 0;
            if (!checkimmediate(c)) {
                if (labelAddress.count(c)) {
                    imm = (long)labelAddress[c] - (long)pc;
                } else imm = 0;
            } else imm = datavalue(c);
            encoding_SB_format_wrapper(out, rs1, rs2, (int)imm, instr, line, pc);
        } else if (U_type.count(instr)) {
            int rd = getReg(a);
            long imm = 0;
            if (!checkimmediate(b)) {
                if (labelAddress.count(b)) imm = (long)labelAddress[b];
                else imm = 0;
            } else imm = datavalue(b);
            encoding_U_format_wrapper(out, rd, (int)imm, instr, line, pc);
        } else if (UJ_type.count(instr)) {
            int rd = getReg(a);
            long imm = 0;
            if (!checkimmediate(b)) {
                if (labelAddress.count(b)) {
                    imm = (long)labelAddress[b] - (long)pc;
                } else imm = 0;
            } else imm = datavalue(b);
            encoding_UJ_format_wrapper(out, rd, (int)imm, instr, line, pc);
        }
}

void write_data_value(ostream &out, uint32_t addr, uint64_t value, int sizeBytes) {
    out << "0x" << hex << uppercase << addr << " " ;
    out << "0x" << uppercase << hex << setw(sizeBytes*2) << setfill('0') << (value & ((sizeBytes==8)?0xFFFFFFFFFFFFFFFFull: ((1ULL<<(sizeBytes*8)) - 1)))<< "\n";
}

int main() {
    ifstream asmFile("input.asm");
    vector<string> lines;
    string instruction;
    while (getline(asmFile, instruction)) {
        size_t comment = instruction.find('#');
        if (comment != string::npos)
            instruction = instruction.substr(0, comment);
        instruction = linetrimmer(instruction);
        if (instruction.empty())
            continue;
        lines.push_back(instruction);
    }
    asmFile.close();
    unordered_map<string, uint32_t> addresslabel;
    uint32_t textaddr = textbaseaddr;
    uint32_t dataddr = databaseaddr;
    bool textseg = false, dataseg = false;
    for (string &line : lines) {
        if (line == ".text") {
            textseg = true;
            dataseg = false;
            continue;
        }
        if (line == ".data") {
            dataseg = true;
            textseg = false;
            continue;
        }
        size_t labelfind = line.find(':');
        if (labelfind != string::npos) {
            string label = linetrimmer(line.substr(0, labelfind));
            if (textseg)
                addresslabel[label] = textaddr;
            else if (dataseg)
                addresslabel[label] = dataddr;
            line = linetrimmer(line.substr(labelfind + 1));
            if (line.empty())
                continue;
        }
        if (textseg) {
            textaddr += 4;
        } else if (dataseg) {
            stringstream ss(line);
            string directive;
            ss >> directive;
            if (directive == ".byte") dataddr += 1;
            else if (directive == ".half") dataddr += 2;
            else if (directive == ".word") dataddr += 4;
            else if (directive == ".dword") dataddr += 8;
            else if (directive == ".asciz") {
                string rest;
                getline(ss, rest);
                rest = linetrimmer(rest);
                if (!rest.empty() && rest.front() == '"' && rest.back() == '"')
                    dataddr += (rest.size() - 2) + 1;
            }
        }
    }
    ofstream out("output.mc");
    textseg = false;
    dataseg = false;
    textaddr = textbaseaddr;
    dataddr = databaseaddr;
    for (auto &line : lines) {
        if (line.empty())
            continue;
        if (line == ".text") {
            textseg = true;
            dataseg = false;
            continue;
        }
        if (line == ".data") {
            dataseg = true;
            textseg = false;
            continue;
        }
        if (textseg) {
            process_text_instruction(out, line, textaddr, addresslabel);
            textaddr += 4;
        } else if (dataseg) {
            stringstream ss(line);
            string directive;
            ss >> directive;
            if (directive == ".byte") {
                string value;
                while (ss >> value) {
                    long v = datavalue(value);
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32((uint32_t)(v & 0xFF)) << " , " << line << "\n";
                    dataddr += 1;
                }
            } else if (directive == ".half") {
                string value;
                while (ss >> value) {
                    long v = datavalue(value);
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32((uint32_t)(v & 0xFFFF)) << " , " << line << "\n";
                    dataddr += 2;
                }
            } else if (directive == ".word") {
                string value;
                while (ss >> value) {
                    long v = datavalue(value);
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32((uint32_t)v) << " , " << line << "\n";
                    dataddr += 4;
                }
            } else if (directive == ".dword") {
                string value;
                while (ss >> value) {
                    long long v = stoll(value, nullptr, 0);
                    uint32_t low = (uint32_t)(v & 0xFFFFFFFFULL);
                    uint32_t high = (uint32_t)((v >> 32) & 0xFFFFFFFFULL);
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32(low) << " , " << line << "\n";
                    dataddr += 4;
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32(high) << " , " << line << "\n";
                    dataddr += 4;
                }
            } else if (directive == ".asciz") {
                string str;
                getline(ss, str);
                str = linetrimmer(str);
                if (!str.empty() && str.front() == '"' && str.back() == '"')
                    str = str.substr(1, str.size() - 2);
                for (char c : str) {
                    out << "0x" << hex << uppercase << dataddr << " "
                        << u32tohex32((uint32_t)(unsigned char)c)
                        << " , " << line << "\n";
                    dataddr++;
                }
                out << "0x" << hex << uppercase << dataddr << " "
                    << u32tohex32(0) << " , " << line << "\n";
                dataddr++;
            }
        }
    }
    out << "0x" << hex << uppercase << textaddr << " <END_OF_TEXT>\n";
    out.close();
    cout << "Assembled successfully.\n";
    return 0;
}