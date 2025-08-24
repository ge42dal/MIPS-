#include "debugger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace mips {

Debugger::Debugger() 
    : prev_pc_(0), prev_hi_(0), prev_lo_(0), running_(false), program_loaded_(false) {
    prev_registers_.fill(0);
}

bool Debugger::load_program(const std::string& assembly_file) {
    std::ifstream file(assembly_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << assembly_file << std::endl;
        return false;
    }
    
    std::string assembly_text((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();
    
    return load_program_from_string(assembly_text);
}

bool Debugger::load_program_from_string(const std::string& assembly_text) {
    try {
        // parse assembly
        assembly_lines_ = assembler_.parse_assembly(assembly_text);
        if (assembler_.has_errors()) {
            std::cerr << "Assembly errors:" << std::endl;
            for (const auto& error : assembler_.get_errors()) {
                std::cerr << "  " << error << std::endl;
            }
            return false;
        }
        
        // assemble to binary
        std::vector<uint8_t> binary = assembler_.assemble(assembly_lines_);
        if (assembler_.has_errors()) {
            std::cerr << "Assembly errors:" << std::endl;
            for (const auto& error : assembler_.get_errors()) {
                std::cerr << "  " << error << std::endl;
            }
            return false;
        }
        
        // load binary into CPU memory
        uint32_t main_address = assembler_.get_main_address();
        for (size_t i = 0; i < binary.size(); ++i) {
            cpu_.get_state().store_byte(main_address + i, binary[i]);
        }
        cpu_.get_state().set_pc(main_address);
        
        // build address-to-assembly mapping and labels
        address_to_assembly_.clear();
        labels_.clear();
        
        for (const auto& line : assembly_lines_) {
            if (!line.instruction.empty()) {
                address_to_assembly_[line.address] = line.instruction;
                if (!line.operands.empty()) {
                    address_to_assembly_[line.address] += " ";
                    for (size_t i = 0; i < line.operands.size(); ++i) {
                        if (i > 0) address_to_assembly_[line.address] += ", ";
                        address_to_assembly_[line.address] += line.operands[i];
                    }
                }
            }
            if (!line.label.empty()) {
                labels_[line.label] = line.address;
            }
        }
        
        // capture initial state
        capture_state();
        program_loaded_ = true;
        
        std::cout << "Program loaded successfully. Entry point: 0x" 
                  << std::hex << std::uppercase << assembler_.get_main_address() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading program: " << e.what() << std::endl;
        return false;
    }
}

void Debugger::run() {
    if (!program_loaded_) {
        std::cerr << "No program loaded. Use load_program() first." << std::endl;
        return;
    }
    
    running_ = true;
    std::cout << "MIPS Debugger - Type 'help' for commands" << std::endl;
    std::cout.flush();
    print_current_instruction();
    std::cout.flush();
    
    std::string input;
    while (running_ && std::getline(std::cin, input)) {
        if (input.empty()) continue;
        
        ParsedCommand cmd = parse_command(input);
        
        switch (cmd.type) {
            case DebugCommand::STEP:
                handle_step();
                break;
            case DebugCommand::REG:
                handle_reg(cmd.argument);
                break;
            case DebugCommand::MEM8:
                handle_mem8(cmd.address);
                break;
            case DebugCommand::MEM16:
                handle_mem16(cmd.address);
                break;
            case DebugCommand::MEM32:
                handle_mem32(cmd.address);
                break;
            case DebugCommand::BREAK:
                handle_break(cmd.argument);
                break;
            case DebugCommand::CONTINUE:
                handle_continue();
                break;
            case DebugCommand::HELP:
                handle_help();
                break;
            case DebugCommand::QUIT:
                running_ = false;
                break;
            case DebugCommand::INVALID:
                std::cout << "Invalid command. Type 'help' for available commands." << std::endl;
                break;
        }
        
        if (running_ && !cpu_.is_halted()) {
            print_prompt();
        } else if (cpu_.is_halted()) {
            std::cout << "Program halted." << std::endl;
            running_ = false;
        }
    }
}

void Debugger::handle_step() {
    if (cpu_.is_halted()) {
        std::cout << "Program has halted." << std::endl;
        return;
    }
    
    capture_state();
    cpu_.run_single_step();
    print_machine_state_changes();
    
    if (!cpu_.is_halted()) {
        print_current_instruction();
    } else {
        std::cout << "Program halted." << std::endl;
    }
}

void Debugger::handle_reg(const std::string& reg_name) {
    Register reg = parse_register_name(reg_name);
    if (static_cast<int>(reg) < 0 || static_cast<int>(reg) > 31) {
        std::cout << "Invalid register name: " << reg_name << std::endl;
        return;
    }
    
    uint32_t value = cpu_.get_state().get_register(reg);
    std::cout << reg_name << " = 0x" << std::hex << std::uppercase << std::setfill('0') 
              << std::setw(8) << value << " (" << std::dec << static_cast<int32_t>(value) << ")" << std::endl;
}

void Debugger::handle_mem8(uint32_t address) {
    try {
        uint8_t value = cpu_.get_state().load_byte(address);
        std::cout << "mem8[0x" << std::hex << std::uppercase << address << "] = 0x" 
                  << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(value) 
                  << " (" << std::dec << static_cast<int32_t>(value) << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Memory access error: " << e.what() << std::endl;
    }
}

void Debugger::handle_mem16(uint32_t address) {
    try {
        uint16_t value = cpu_.get_state().load_half(address);
        std::cout << "mem16[0x" << std::hex << std::uppercase << address << "] = 0x" 
                  << std::setfill('0') << std::setw(4) << value 
                  << " (" << std::dec << static_cast<int32_t>(static_cast<int16_t>(value)) << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Memory access error: " << e.what() << std::endl;
    }
}

void Debugger::handle_mem32(uint32_t address) {
    try {
        uint32_t value = cpu_.get_state().load_word(address);
        std::cout << "mem32[0x" << std::hex << std::uppercase << address << "] = 0x" 
                  << std::setfill('0') << std::setw(8) << value 
                  << " (" << std::dec << static_cast<int32_t>(value) << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Memory access error: " << e.what() << std::endl;
    }
}

void Debugger::handle_break(const std::string& label_or_address) {
    uint32_t address;
    
    // try to parse as label first
    auto label_it = labels_.find(label_or_address);
    if (label_it != labels_.end()) {
        address = label_it->second;
    } else {
        // try to parse as address
        try {
            address = parse_address(label_or_address);
        } catch (const std::exception& e) {
            std::cout << "Invalid label or address: " << label_or_address << std::endl;
            return;
        }
    }
    
    // add breakpoint
    if (std::find(breakpoints_.begin(), breakpoints_.end(), address) == breakpoints_.end()) {
        breakpoints_.push_back(address);
        std::cout << "Breakpoint set at 0x" << std::hex << std::uppercase << address << std::endl;
    } else {
        std::cout << "Breakpoint already exists at 0x" << std::hex << std::uppercase << address << std::endl;
    }
}

void Debugger::handle_continue() {
    if (cpu_.is_halted()) {
        std::cout << "Program has halted." << std::endl;
        return;
    }
    
    std::cout << "Continuing execution..." << std::endl;
    
    while (!cpu_.is_halted()) {
        capture_state();
        cpu_.run_single_step();
        
        uint32_t current_pc = cpu_.get_state().get_pc();
        if (is_at_breakpoint(current_pc)) {
            std::cout << "Breakpoint hit at 0x" << std::hex << std::uppercase << current_pc << std::endl;
            print_machine_state_changes();
            print_current_instruction();
            return;
        }
    }
    
    std::cout << "Program halted." << std::endl;
}

void Debugger::handle_help() {
    std::cout << "Available commands:" << std::endl;
    std::cout << "  step                    - Execute current instruction and move to next" << std::endl;
    std::cout << "  reg <register>          - Show register value (e.g., reg $t0, reg $ra)" << std::endl;
    std::cout << "  mem8 <address>          - Show 8-bit value at memory address" << std::endl;
    std::cout << "  mem16 <address>         - Show 16-bit value at memory address" << std::endl;
    std::cout << "  mem32 <address>         - Show 32-bit value at memory address" << std::endl;
    std::cout << "  break <label|address>   - Set breakpoint at label or address" << std::endl;
    std::cout << "  continue                - Continue execution until breakpoint or halt" << std::endl;
    std::cout << "  help                    - Show this help message" << std::endl;
    std::cout << "  quit                    - Exit debugger" << std::endl;
}

void Debugger::print_current_instruction() {
    uint32_t pc = cpu_.get_state().get_pc();
    std::string instruction = format_instruction_at_address(pc);
    
    std::cout << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) 
              << pc << ": " << instruction << std::endl;
}

void Debugger::print_machine_state_changes() {
    const MachineState& state = cpu_.get_state();
    bool changes_found = false;
    
    // Check register changes
    for (int i = 0; i < 32; ++i) {
        Register reg = static_cast<Register>(i);
        uint32_t current = state.get_register(reg);
        if (current != prev_registers_[i]) {
            if (!changes_found) {
                std::cout << "State changes:" << std::endl;
                changes_found = true;
            }
            std::cout << "  " << get_register_name(reg) << ": 0x" << std::hex << std::uppercase 
                      << std::setfill('0') << std::setw(8) << prev_registers_[i] 
                      << " -> 0x" << std::setw(8) << current << std::endl;
        }
    }
    
    // check PC change
    uint32_t current_pc = state.get_pc();
    if (current_pc != prev_pc_) {
        if (!changes_found) {
            std::cout << "State changes:" << std::endl;
            changes_found = true;
        }
        std::cout << "  PC: 0x" << std::hex << std::uppercase << std::setfill('0') 
                  << std::setw(8) << prev_pc_ << " -> 0x" << std::setw(8) << current_pc << std::endl;
    }
    
    if (!changes_found) {
        std::cout << "No state changes." << std::endl;
    }
}

ParsedCommand Debugger::parse_command(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    
    // convert to lowercase
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    ParsedCommand result;
    result.type = DebugCommand::INVALID;
    result.address = 0;
    
    if (cmd == "step" || cmd == "s") {
        result.type = DebugCommand::STEP;
    } else if (cmd == "reg" || cmd == "r") {
        result.type = DebugCommand::REG;
        iss >> result.argument;
    } else if (cmd == "mem8") {
        result.type = DebugCommand::MEM8;
        std::string addr_str;
        iss >> addr_str;
        try {
            result.address = parse_address(addr_str);
        } catch (const std::exception& e) {
            result.type = DebugCommand::INVALID;
        }
    } else if (cmd == "mem16") {
        result.type = DebugCommand::MEM16;
        std::string addr_str;
        iss >> addr_str;
        try {
            result.address = parse_address(addr_str);
        } catch (const std::exception& e) {
            result.type = DebugCommand::INVALID;
        }
    } else if (cmd == "mem32") {
        result.type = DebugCommand::MEM32;
        std::string addr_str;
        iss >> addr_str;
        try {
            result.address = parse_address(addr_str);
        } catch (const std::exception& e) {
            result.type = DebugCommand::INVALID;
        }
    } else if (cmd == "break" || cmd == "b") {
        result.type = DebugCommand::BREAK;
        iss >> result.argument;
    } else if (cmd == "continue" || cmd == "c") {
        result.type = DebugCommand::CONTINUE;
    } else if (cmd == "help" || cmd == "h") {
        result.type = DebugCommand::HELP;
    } else if (cmd == "quit" || cmd == "q") {
        result.type = DebugCommand::QUIT;
    }
    
    return result;
}

std::string Debugger::get_register_name(Register reg) {
    static const std::vector<std::string> names = {
        "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
    };
    
    int index = static_cast<int>(reg);
    if (index >= 0 && index < 32) {
        return names[index];
    }
    return "$invalid";
}

Register Debugger::parse_register_name(const std::string& name) {
    try {
        return string_to_register(name);
    } catch (const std::exception&) {
        return static_cast<Register>(255); // Invalid register
    }
}

uint32_t Debugger::parse_address(const std::string& addr_str) {
    if (addr_str.empty()) {
        throw std::invalid_argument("Empty address string");
    }
    
    // Handle hex addresses (0x prefix)
    if (addr_str.substr(0, 2) == "0x" || addr_str.substr(0, 2) == "0X") {
        return std::stoul(addr_str, nullptr, 16);
    }
    
    // Handle decimal addresses
    return std::stoul(addr_str, nullptr, 10);
}

void Debugger::capture_state() {
    const MachineState& state = cpu_.get_state();
    
    for (int i = 0; i < 32; ++i) {
        prev_registers_[i] = state.get_register(static_cast<Register>(i));
    }
    prev_pc_ = state.get_pc();
    prev_hi_ = state.get_hi();
    prev_lo_ = state.get_lo();
}

void Debugger::print_prompt() {
    std::cout << "> ";
    std::cout.flush();
}

std::string Debugger::format_instruction_at_address(uint32_t address) {
    auto it = address_to_assembly_.find(address);
    if (it != address_to_assembly_.end()) {
        return it->second;
    }
    
    // If no assembly found, try to decode the instruction from memory
    try {
        uint32_t instruction_word = cpu_.get_state().load_word(address);
        if (instruction_word == 0) {
            return "nop";
        }
        return "unknown instruction (0x" + std::to_string(instruction_word) + ")";
    } catch (const std::exception& e) {
        return "invalid memory access";
    }
}

bool Debugger::is_at_breakpoint(uint32_t pc) {
    return std::find(breakpoints_.begin(), breakpoints_.end(), pc) != breakpoints_.end();
}

} // namespace mips
