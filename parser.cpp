#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include "parser.h"
#include "symbol_table.h"
#include "converter.h"

using namespace std;

// Helper function to trim whitespace from a string
string trimWhitespace(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

// Helper function to remove comments (anything after #)
string removeComments(const string& str) {
    size_t commentPos = str.find('#');
    if (commentPos != string::npos) {
        return str.substr(0, commentPos);
    }
    return str;
}

string decimalToHex(int decimal) {
    stringstream ss;
    ss << hex << decimal;  
    return ss.str();
}

// Function to parse instructions
void parseInstructionFields(const string& line,string& line_name, string& opcode, string& format, string& rd, string& rs1, string& rs2, string& immediate) {
    line_name=line;
    istringstream iss(line);
    iss >> opcode;  // Read the opcode first

    // Identify instruction format while parsing
    if (opcode == "add" || opcode == "sub" || opcode == "xor" || 
        opcode == "or" || opcode == "and" || opcode == "sll" || 
        opcode == "slt" || opcode == "sra" || opcode == "srl" || 
        opcode == "mul" || opcode == "div" || opcode == "rem") {
        
        format = "R"; // R-format
        iss >> rd >> rs1 >> rs2;
        if (!rd.empty() && rd.back() == ',') rd.pop_back();
        if (!rs1.empty() && rs1.back() == ',') rs1.pop_back();
        if (!rs2.empty() && rs2.back() == ',') rs2.pop_back();
    } 
    else if (opcode == "addi" || opcode == "andi" || opcode == "ori") {
        format = "I"; // I-format
        iss >> rd >> rs1 >> immediate; 
        if (!rd.empty() && rd.back() == ',') rd.pop_back();
        if (!rs1.empty() && rs1.back() == ',') rs1.pop_back();
    } 
    else if (opcode == "lb" || opcode == "lh" || opcode == "lw" || opcode == "ld" || opcode == "jalr") {
        format = "I"; // I-format for load instructions
        string rdStr, addressStr;
        iss >> rdStr >> addressStr;
        if (!rdStr.empty() && rdStr.back() == ',') rdStr.pop_back();

        size_t openBracket = addressStr.find('(');
        size_t closeBracket = addressStr.find(')');

        if (openBracket != string::npos && closeBracket != string::npos) {
            immediate = addressStr.substr(0, openBracket);
            rs1 = addressStr.substr(openBracket + 1, closeBracket - openBracket - 1);
        }
        rd = rdStr;
    }
    else if (opcode == "sb" || opcode == "sh" || opcode == "sw" || opcode == "sd") {
        format = "S"; // S-format
        string rs2Str, addressStr;
        iss >> rs2Str >> addressStr;
        if (!rs2Str.empty() && rs2Str.back() == ',') rs2Str.pop_back();

        size_t openBracket = addressStr.find('(');
        size_t closeBracket = addressStr.find(')');

        if (openBracket != string::npos && closeBracket != string::npos) {
            immediate = addressStr.substr(0, openBracket);
            rs1 = addressStr.substr(openBracket + 1, closeBracket - openBracket - 1);
        }
        rs2 = rs2Str;
    }
    else if (opcode == "beq" || opcode == "bne" || opcode == "blt" || opcode == "bge") {
        format = "SB"; // SB-format
        iss >> rs1 >> rs2 >> immediate;
        cout << "Immediate: " << immediate << endl;
        if (!rs1.empty() && rs1.back() == ',') rs1.pop_back();
        if (!rs2.empty() && rs2.back() == ',') rs2.pop_back();
    }
    else if (opcode == "lui" || opcode == "auipc") {
        format = "U"; // U-format
        iss >> rd >> immediate;
        if(immediate[1]!='x'){
            //convert decimal to hex
            immediate=decimalToHex(stoi(immediate));
            immediate="0x"+immediate;
            //cout<<"Immediate: "<<immediate<<endl;
        }
        immediate = to_string(stoi(immediate, nullptr, 16)); // Convert hex to decimal
        if (!rd.empty() && rd.back() == ',') rd.pop_back();
    }
    else if (opcode == "jal") {
        format = "UJ"; // UJ-format
        iss >> rd >> immediate;
        if (!rd.empty() && rd.back() == ',') rd.pop_back();
    }
}

// Function to handle assembler directives
void processDirective(const string& directive, istringstream& iss, SymbolTable& symbolTable, uint32_t& dataAddress) {
    if (directive == ".text") {
      //  cout << "[DEBUG] Switching to TEXT section." << endl;
    } 
    else if (directive == ".data") {
      //  cout << "[DEBUG] Switching to DATA section." << endl;
    } 
    else if (directive == ".word") {
        int value;
        while (iss >> value) {
           // cout << "[DEBUG] Storing word value: " << value << " at address 0x" << hex << dataAddress << endl;
            symbolTable.addData(dataAddress, value);
            dataAddress += 4;
        }
    } 
    else if (directive == ".half") {
        int value;
        while (iss >> value) {
           // cout << "[DEBUG] Storing halfword value: " << value << " at address 0x" << hex << dataAddress << endl;
            symbolTable.addData(dataAddress, value & 0xFFFF);
            dataAddress += 2;
        }
    }
    else if (directive == ".byte") {
        int value;
        while (iss >> value) {
           // cout << "[DEBUG] Storing byte value: " << value << " at address 0x" << hex << dataAddress << endl;
            symbolTable.addData(dataAddress, value & 0xFF);
            dataAddress += 1;
        }
    }
    else if (directive == ".dword") {
        int64_t value;
        while (iss >> value) {
           // cout << "[DEBUG] Storing double word (dword) value: " << value << " at address 0x" << hex << dataAddress << endl;
            symbolTable.addData(dataAddress, value & 0xFFFFFFFFFFFFFFFF);
            dataAddress += 8;
        }
    }
    else if (directive == ".asciiz") {
        string str;
        getline(iss, str);
        str = trimWhitespace(str);
        if (str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2); // Remove quotes
        }
        for (char ch : str) {
            symbolTable.addData(dataAddress, ch);
            dataAddress++;
        }
        symbolTable.addData(dataAddress, 0); // Null terminator
        dataAddress++;
    }
    else if (directive == ".globl") {
        string symbol;
        iss >> symbol;
        //cout << "[DEBUG] Declared global symbol: " << symbol << endl;
    }
}


// Modify parseFile to recognize directives
bool parseFile(const string& filename, vector<Instruction>& instructions, SymbolTable& symbolTable, bool firstPass) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    string line;
    uint32_t address = 0; // Instruction memory address
    uint32_t dataAddress = 0x10000000; // Data section starts from here

    while (getline(inFile, line)) {
        line = removeComments(line);
        line = trimWhitespace(line);
        if (line.empty()) continue;

        size_t colonPos = line.find(':');

        if (colonPos != string::npos) {
            string label = trimWhitespace(line.substr(0, colonPos));

            if (!label.empty() && !isdigit(label[0])) {
                if (firstPass) {
                    symbolTable.addLabel(label, address);
                    //cout << "[DEBUG] Stored Label: '" << label << "' at Address: 0x" << hex << address << endl;
                }
            } else {
                cerr << "Error: Invalid label '" << label << "' - Labels cannot start with numbers!" << endl;
                return false;
            }

            if (colonPos + 1 < line.size()) {
                line = trimWhitespace(line.substr(colonPos + 1));
            } else {
                continue;
            }
        }

        istringstream iss(line);
        string firstWord;
        iss >> firstWord;

        if (firstWord[0] == '.') {  // Handle assembler directives
            processDirective(firstWord, iss, symbolTable, dataAddress);
        } 
        else if (!firstPass) {
            Instruction instr;
            parseInstructionFields(line, instr.line_name,instr.opcode, instr.format, instr.rd, instr.rs1, instr.rs2, instr.immediate);

            if(instr.opcode=="auipc"){
                instr.immediate=to_string(stoi(instr.immediate)+address);
               
            }

            if ((instr.format == "SB" || instr.format == "UJ") && !instr.immediate.empty()) {
                //cout<<"[DEBUG] Immediate: "<<instr.immediate<<endl;
                if (!isdigit(instr.immediate[0])) {
                    uint32_t labelAddress = symbolTable.getAddress(instr.immediate);
                    //cout<<"[DEBUG] Label Address: "<<labelAddress<<endl;
                    if (labelAddress != 0xFFFFFFFF) {
                        uint32_t offset = (labelAddress - address);
                        //cout<<"[DEBUG] Calculated Offset: "<<offset<<endl;
                        instr.immediate = to_string(offset);
                    } else {
                        cerr << "Error: Label '" << instr.immediate << "' not found in symbol table." << endl;
                        return false;
                    }
                }
            }

            instructions.push_back(instr);
        }

        address += 4; 
    }

    inFile.close();
    return true;
}
