#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include <string>
#include <cstdint>
#include <set>

class SymbolTable {
private:
    std::map<std::string, uint32_t> table; // Label -> Address
    std::map<uint32_t, int> dataSection;  // Address -> Data Value
    std::set<std::string> globalSymbols;  // Global symbols
    std::map<std::string, int> constants; // Constants

public:
    void addLabel(const std::string& label, uint32_t address);
    void addData(uint32_t address, int val);
    int getData(uint32_t address) const;
    void addGlobal(const std::string& symbol);
    bool isGlobal(const std::string& symbol) const;
    void addConstant(const std::string& name, int value);
    int getConstant(const std::string& name) const;
    uint32_t getAddress(const std::string& label) const;
};

#endif
