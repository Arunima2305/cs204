#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip> // for hex formatting
#include "parser.h"
#include "converter.h"
#include "symbol_table.h"

using namespace std;

string func(Instruction instr){
    string s="";
    s+=instr.opcode+'-'+instr.func3+'-'+instr.func7+'-'+instr.rd+'-'+instr.rs1+'-'+instr.rs2+'-'+instr.immediate;
    return s;
}


int main() {
    // Create a symbol table instance
    SymbolTable symbolTable;
    
    // Vector to store parsed instructions
    vector<Instruction> instructions;

    // Open input file
    string inputFilename = "input.asm";

    // Pass 1: Collect labels and directives
    if (!parseFile(inputFilename, instructions, symbolTable, true)) {  // First pass (label collection)
        cerr << "Error: Failed in Pass 1 (Label Collection)." << endl;
        return 1;
    }
    
    // Pass 2: Parse instructions again for final conversion
    if (!parseFile(inputFilename, instructions, symbolTable, false)) {  // Second pass (actual conversion)
        cerr << "Error: Failed in Pass 2 (Instruction Parsing)." << endl;
        return 1;
    }

    // Open output file
    ofstream outFile("output.mc");
    if (!outFile.is_open()) {
        cerr << "Error: Could not open output file." << endl;
        return 1;
    }

    uint32_t address = 0x10000000; // Start of data segment
    
    // Write machine code for data section
    int val = symbolTable.getData(address);
    while(val!=-1) {
        outFile << "0x" << hex << address++ << " 0x" << setfill('0') << setw(8) << val << " # Data" << endl;
        val = symbolTable.getData(address);
    }

    address = 0x0000000;
    // Write machine code for instructions
    for ( Instruction& instr : instructions) {
        uint32_t machineCode = convertToMachineCode(instr, symbolTable);
        string s=func(instr);
        // Print and write to file in required format
        outFile << "0x" << hex << address << " "
                << "0x" << setfill('0') << setw(8) << machineCode << " , "<<instr.line_name<< " # "
                << s << endl;

        address += 4; // Increment address (each instruction is 4 bytes)
    }

    cout << "Successfully converted input.asm to output.mc with directives!" << endl;
    outFile.close();
    return 0;
}
