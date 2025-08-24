#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <iostream>
#include <memory>

namespace mips {

// MIPS register defs
enum class Register : uint8_t {
    ZERO = 0, AT = 1, V0 = 2, V1 = 3,
    A0 = 4, A1 = 5, A2 = 6, A3 = 7,
    T0 = 8, T1 = 9, T2 = 10, T3 = 11,
    T4 = 12, T5 = 13, T6 = 14, T7 = 15,
    S0 = 16, S1 = 17, S2 = 18, S3 = 19,
    S4 = 20, S5 = 21, S6 = 22, S7 = 23,
    T8 = 24, T9 = 25, K0 = 26, K1 = 27,
    GP = 28, SP = 29, S8 = 30, RA = 31
};

// instruction formats
enum class InstructionType {
    R_TYPE,     // reg format
    I_TYPE,     // imm format
    J_TYPE      // jump format
};

// instruction categories
enum class InstructionCategory {
    ARITH_LOGIC,
    DIV_MULT,
    SHIFT,
    SHIFT_REG,
    JUMP_REG,
    MOVE_FROM,
    MOVE_TO,
    ARITH_LOGIC_IMM,
    LOAD_IMM,
    BRANCH,
    BRANCH_ZERO,
    LOAD_STORE,
    JUMP,
    TRAP
};

// Machine state class
class MachineState {
public:
    static constexpr uint64_t MEMORY_SIZE = 0x100000000ULL; // 4GB
    static constexpr size_t NUM_REGISTERS = 32;
    static constexpr size_t PAGE_SIZE = 4096; // 4KB pages
    static constexpr size_t NUM_PAGES = MEMORY_SIZE / PAGE_SIZE;
    
    MachineState();
    
    // register access
    uint32_t get_register(Register reg) const;
    void set_register(Register reg, uint32_t value);
    
    // special registers
    uint32_t get_pc() const { return pc_; }
    void set_pc(uint32_t value) { pc_ = value; }
    uint32_t get_hi() const { return hi_; }
    void set_hi(uint32_t value) { hi_ = value; }
    uint32_t get_lo() const { return lo_; }
    void set_lo(uint32_t value) { lo_ = value; }
    
    // memory access
    uint8_t load_byte(uint32_t address) const;
    uint16_t load_half(uint32_t address) const;
    uint32_t load_word(uint32_t address) const;
    void store_byte(uint32_t address, uint8_t value);
    void store_half(uint32_t address, uint16_t value);
    void store_word(uint32_t address, uint32_t value);
    
    // memory initialization
    void load_memory(const std::vector<uint8_t>& data, uint32_t start_address = 0);
    
    // I/O streams (configurable for testing)
    std::istream* input_stream = &std::cin;
    std::ostream* output_stream = &std::cout;
    
private:
    std::array<uint32_t, NUM_REGISTERS> registers_;
    std::unordered_map<uint32_t, std::unique_ptr<std::array<uint8_t, PAGE_SIZE>>> memory_pages_;
    uint32_t pc_;
    uint32_t hi_;
    uint32_t lo_;
    
    // helper methods for page-based memory
    uint32_t get_page_index(uint32_t address) const { return address / PAGE_SIZE; }
    uint32_t get_page_offset(uint32_t address) const { return address % PAGE_SIZE; }
    std::array<uint8_t, PAGE_SIZE>* get_or_create_page(uint32_t page_index);
    const std::array<uint8_t, PAGE_SIZE>* get_page(uint32_t page_index) const;
};

// instruction representation
struct Instruction {
    uint32_t opcode;
    uint32_t rs, rt, rd;
    uint32_t immediate;
    uint32_t function;
    uint32_t shamt;
    uint32_t address;
    InstructionType type;
    InstructionCategory category;
    std::string name;
    
    // decode from 32-bit instruction
    static Instruction decode(uint32_t instruction_word);
    
    // Encode to 32-bit instruction
    uint32_t encode() const;
};

// MIPS CPU class
class CPU {
public:
    CPU();
    
    // execute single instruction
    void execute_instruction(const Instruction& instr);
    
    // run program
    void run();
    void run_single_step();
    
    // machine state access
    MachineState& get_state() { return state_; }
    const MachineState& get_state() const { return state_; }
    
    // control
    void reset();
    bool is_halted() const { return halted_; }
    
private:
    MachineState state_;
    bool halted_;
    
    // instruction execution methods
    void execute_arith_logic(const Instruction& instr);
    void execute_div_mult(const Instruction& instr);
    void execute_shift(const Instruction& instr);
    void execute_shift_reg(const Instruction& instr);
    void execute_jump_reg(const Instruction& instr);
    void execute_move_from(const Instruction& instr);
    void execute_move_to(const Instruction& instr);
    void execute_arith_logic_imm(const Instruction& instr);
    void execute_load_imm(const Instruction& instr);
    void execute_branch(const Instruction& instr);
    void execute_branch_zero(const Instruction& instr);
    void execute_load_store(const Instruction& instr);
    void execute_jump(const Instruction& instr);
    void execute_trap(const Instruction& instr);
    
    // helper functions
    int32_t sign_extend_16(uint16_t value);
    uint32_t zero_extend_16(uint16_t value);
    int32_t sign_extend_8(uint8_t value);
    uint32_t zero_extend_8(uint8_t value);
    void determine_instruction_info(Instruction& instr);
};

// utility functions
Register string_to_register(const std::string& reg_name);
std::string register_to_string(Register reg);

} // namespace mips
