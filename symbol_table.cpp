#include <map>
#include <string>
#include <iostream>
#include <vector>
#include "symbol_table.h"

using namespace std;

// Add a label and its address to the symbol table
void SymbolTable::addLabel(const string& label, uint32_t address) {
    table[label] = address;
}

// Retrieve the address of a label
uint32_t SymbolTable::getAddress(const string& label) const {
    auto it = table.find(label);
    if (it != table.end()) {
        return it->second;
    }
    return 0xFFFFFFFF; // Return an invalid address if label not found
}

// Add a data value at a specific address
void SymbolTable::addData(uint32_t address, int val) {
    dataSection[address] = val;
}

// Retrieve a data value from a specific address
int SymbolTable::getData(uint32_t address) const {
    auto it = dataSection.find(address);
    if (it != dataSection.end()) {
        return it->second;
    }
    return -1; // Default value if data not found
}

// Add a global symbol
void SymbolTable::addGlobal(const string& symbol) {
    globalSymbols.insert(symbol);
}

// Check if a symbol is global
bool SymbolTable::isGlobal(const string& symbol) const {
    return globalSymbols.find(symbol) != globalSymbols.end();
}

// Add a constant definition
void SymbolTable::addConstant(const string& name, int value) {
    constants[name] = value;
}

// Retrieve a constant value
int SymbolTable::getConstant(const string& name) const {
    auto it = constants.find(name);
    if (it != constants.end()) {
        return it->second;
    }
    return 0; // Default if constant not found
}
