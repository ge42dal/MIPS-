#pragma once

#include "mips_core.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace mips {

// Assembly directive types
enum class DirectiveType {
    BYTE,
    HALF,
    WORD,
    ASCII,
    ASCIIZ,
    SPACE
};

// Assembly directive
struct Directive {
    DirectiveType type;
    std::vector<std::string> arguments;
    uint32_t size() const;
};

// Label information
struct Label {
    std::string name;
    uint32_t address;
};

// Assembly line (instruction or directive)
struct AssemblyLine {
    std::string label;
    std::string instruction;
    std::vector<std::string> operands;
    bool is_directive;
    uint32_t address;
    uint32_t size;
};

// Assembler class
class Assembler {
public:
    Assembler();
    
    // Parse assembly text
    std::vector<AssemblyLine> parse_assembly(const std::string& assembly_text);
    std::vector<AssemblyLine> parse_assembly(std::istream& input);
    
    // Assemble to binary
    std::vector<uint8_t> assemble(const std::vector<AssemblyLine>& lines);
    std::vector<uint8_t> assemble_text(const std::string& assembly_text);
    std::vector<uint8_t> assemble_stream(std::istream& input);
    
    // Get main label address
    uint32_t get_main_address() const { return main_address_; }
    
    // Error handling
    const std::vector<std::string>& get_errors() const { return errors_; }
    bool has_errors() const { return !errors_.empty(); }
    
private:
    std::unordered_map<std::string, uint32_t> labels_;
    std::unordered_map<std::string, uint32_t> instruction_opcodes_;
    std::unordered_map<std::string, uint32_t> instruction_functions_;
    std::unordered_map<std::string, InstructionCategory> instruction_categories_;
    std::vector<std::string> errors_;
    uint32_t main_address_;
    
    // Parsing helpers
    AssemblyLine parse_line(const std::string& line, uint32_t line_number);
    std::vector<std::string> tokenize(const std::string& line);
    std::string trim(const std::string& str);
    bool is_register(const std::string& token);
    bool is_immediate(const std::string& token);
    bool is_label(const std::string& token);
    
    // Assembly helpers
    uint32_t resolve_immediate(const std::string& imm_str, uint32_t current_address);
    uint32_t resolve_address(const std::string& label, uint32_t current_address);
    Instruction assemble_instruction(const AssemblyLine& line);
    std::vector<uint8_t> assemble_directive(const AssemblyLine& line);
    
    // Instruction encoding
    uint32_t encode_r_type(uint32_t opcode, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t function);
    uint32_t encode_i_type(uint32_t opcode, uint32_t rs, uint32_t rt, uint32_t immediate);
    uint32_t encode_j_type(uint32_t opcode, uint32_t address);
    
    // Initialize instruction tables
    void init_instruction_tables();
    
    // Error reporting
    void add_error(const std::string& error, uint32_t line_numbe = 0);
};

// Binary format I/O
class BinaryFormat {
public:
    // Write binary format
    static void write_binary(const std::vector<uint8_t>& data, std::ostream& output, uint32_t main_address = 0);
    static void write_binary_file(const std::vector<uint8_t>& data, const std::string& filename, uint32_t main_address = 0);
    
    // Read binary format
    static std::vector<uint8_t> read_binary(std::istream& input, uint32_t& main_address);
    static std::vector<uint8_t> read_binary_file(const std::string& filename, uint32_t& main_address);
};

} // namespace mips
