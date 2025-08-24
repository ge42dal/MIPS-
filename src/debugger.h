#pragma once

#include "mips_core.h"
#include "assembler.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace mips {

// debugger command types
enum class DebugCommand {
    STEP,
    REG,
    MEM8,
    MEM16,
    MEM32,
    BREAK,
    CONTINUE,
    QUIT,
    HELP,
    INVALID
};

// parsed command structure
struct ParsedCommand {
    DebugCommand type;
    std::string argument;
    uint32_t address;
};

// debugger class for stepping through MIPS programs
class Debugger {
public:
    Debugger();
    
    // load and prepare program for debugging
    bool load_program(const std::string& assembly_file);
    bool load_program_from_string(const std::string& assembly_text);
    
    // main debugging loop
    void run();
    
    // command handlers
    void handle_step();
    void handle_reg(const std::string& reg_name);
    void handle_mem8(uint32_t address);
    void handle_mem16(uint32_t address);
    void handle_mem32(uint32_t address);
    void handle_break(const std::string& label_or_address);
    void handle_continue();
    void handle_help();
    
    // state inspection
    void print_current_instruction();
    void print_machine_state_changes();
    
private:
    CPU cpu_;
    Assembler assembler_;
    std::vector<AssemblyLine> assembly_lines_;
    std::unordered_map<uint32_t, std::string> address_to_assembly_;
    std::unordered_map<std::string, uint32_t> labels_;
    std::vector<uint32_t> breakpoints_;
    
    // prev state for change detection
    std::array<uint32_t, 32> prev_registers_;
    uint32_t prev_pc_;
    uint32_t prev_hi_;
    uint32_t prev_lo_;
    
    bool running_;
    bool program_loaded_;
    
    // helper funcs
    ParsedCommand parse_command(const std::string& input);
    std::string get_register_name(Register reg);
    Register parse_register_name(const std::string& name);
    uint32_t parse_address(const std::string& addr_str);
    void capture_state();
    void print_prompt();
    std::string format_instruction_at_address(uint32_t address);
    bool is_at_breakpoint(uint32_t pc);
};

} // namespace mips
