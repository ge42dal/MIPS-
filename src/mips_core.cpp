#include "mips_core.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace mips {

// MachineState implementation
MachineState::MachineState() 
    : pc_(0), hi_(0), lo_(0) {
    registers_.fill(0);
    // Memory pages are allocated on-demand, no upfront allocation
}

uint32_t MachineState::get_register(Register reg) const {
    uint8_t index = static_cast<uint8_t>(reg);
    if (index >= NUM_REGISTERS) {
        throw std::out_of_range("Invalid register index");
    }
    // $zero always returns 0
    if (reg == Register::ZERO) {
        return 0;
    }
    return registers_[index];
}

void MachineState::set_register(Register reg, uint32_t value) {
    uint8_t index = static_cast<uint8_t>(reg);
    if (index >= NUM_REGISTERS) {
        throw std::out_of_range("Invalid register index");
    }
    // $zero is read-only
    if (reg != Register::ZERO) {
        registers_[index] = value;
    }
}

uint8_t MachineState::load_byte(uint32_t address) const {
    if (static_cast<uint64_t>(address) >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    uint32_t page_index = get_page_index(address);
    uint32_t page_offset = get_page_offset(address);
    
    const auto* page = get_page(page_index);
    if (!page) {
        return 0; // Uninitialized memory reads as 0
    }
    return (*page)[page_offset];
}

uint16_t MachineState::load_half(uint32_t address) const {
    if (static_cast<uint64_t>(address) + 1 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    // Little-endian
    return static_cast<uint16_t>(load_byte(address)) | 
           (static_cast<uint16_t>(load_byte(address + 1)) << 8);
}

uint32_t MachineState::load_word(uint32_t address) const {
    if (static_cast<uint64_t>(address) + 3 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    // Little-endian
    return static_cast<uint32_t>(load_byte(address)) |
           (static_cast<uint32_t>(load_byte(address + 1)) << 8) |
           (static_cast<uint32_t>(load_byte(address + 2)) << 16) |
           (static_cast<uint32_t>(load_byte(address + 3)) << 24);
}

void MachineState::store_byte(uint32_t address, uint8_t value) {
    if (static_cast<uint64_t>(address) >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    uint32_t page_index = get_page_index(address);
    uint32_t page_offset = get_page_offset(address); // location in page_index
    
    auto* page = get_or_create_page(page_index); // page pointer to location in unorderd map
    (*page)[page_offset] = value; // store value at page_offset in page_index
}

void MachineState::store_half(uint32_t address, uint16_t value) {
    if (static_cast<uint64_t>(address) + 1 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    // Little-endian
    store_byte(address, static_cast<uint8_t>(value & 0xFF));
    store_byte(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}

void MachineState::store_word(uint32_t address, uint32_t value) {
    if (static_cast<uint64_t>(address) + 3 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory address out of bounds");
    }
    // Little-endian
    store_byte(address, static_cast<uint8_t>(value & 0xFF));
    store_byte(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
    store_byte(address + 2, static_cast<uint8_t>((value >> 16) & 0xFF));
    store_byte(address + 3, static_cast<uint8_t>((value >> 24) & 0xFF));
}

void MachineState::load_memory(const std::vector<uint8_t>& data, uint32_t start_address) {
    if (start_address + data.size() > MEMORY_SIZE) {
        throw std::out_of_range("Data too large for memory");
    }
    for (size_t i = 0; i < data.size(); ++i) {
        store_byte(start_address + i, data[i]);
    }
}

// page management helper methods
std::array<uint8_t, MachineState::PAGE_SIZE>* MachineState::get_or_create_page(uint32_t page_index) {
    //returns page pointer
    auto it = memory_pages_.find(page_index); // from map 
    if (it != memory_pages_.end()) { // case mem left in page
        return it->second.get();
    }
    
    // create new page, initialize 4KB (4096) to zero
    auto new_page = std::make_unique<std::array<uint8_t, PAGE_SIZE>>();
    new_page->fill(0);
    auto* page_ptr = new_page.get();
    memory_pages_[page_index] = std::move(new_page);
    return page_ptr;
}

const std::array<uint8_t, MachineState::PAGE_SIZE>* MachineState::get_page(uint32_t page_index) const {
    auto it = memory_pages_.find(page_index);
    if (it != memory_pages_.end()) {
        return it->second.get();
    }
    return nullptr; // Page doesn't exist
}

// Instruction implementation
Instruction Instruction::decode(uint32_t instruction_word) {
    Instruction instr;
    
    instr.opcode = (instruction_word >> 26) & 0x3F;
    instr.rs = (instruction_word >> 21) & 0x1F;
    instr.rt = (instruction_word >> 16) & 0x1F;
    instr.rd = (instruction_word >> 11) & 0x1F;
    instr.shamt = (instruction_word >> 6) & 0x1F;
    instr.function = instruction_word & 0x3F;
    instr.immediate = instruction_word & 0xFFFF;
    instr.address = instruction_word & 0x3FFFFFF;
    
    // Determine instruction type based on opcode
    if (instr.opcode == 0) {
        instr.type = InstructionType::R_TYPE;
    } else if (instr.opcode == 2 || instr.opcode == 3) {
        instr.type = InstructionType::J_TYPE;
    } else {
        instr.type = InstructionType::I_TYPE;
    }
    
    // Set default category (will be overridden by assembler)
    instr.category = InstructionCategory::ARITH_LOGIC;
    
    return instr;
}

uint32_t Instruction::encode() const { // each instr = 32 bits, shift instr fields to left 
    switch (type) {
        case InstructionType::R_TYPE:
            return (opcode << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | function; // 31-26 + 25-21 + 20-16 + 15-11 + 10-6 + 5-0 
        case InstructionType::I_TYPE:
            return (opcode << 26) | (rs << 21) | (rt << 16) | (immediate & 0xFFFF); // 31-26 + 25-21 + 20-16 + 15-0; bitmask 0xFFFF to ensure only 16 bits are used
        case InstructionType::J_TYPE:
            return (opcode << 26) | (address & 0x3FFFFFF); // 31-26 + 25-0; bitmask 0x3FFFFFF to ensure only 26 bits are used
        default:
            return 0;
    }
}

// CPU implementation
CPU::CPU() : halted_(false) {}

void CPU::execute_instruction(const Instruction& instr) {
    if (halted_) return;
    
    switch (instr.category) {
        case InstructionCategory::ARITH_LOGIC:
            execute_arith_logic(instr);
            break;
        case InstructionCategory::DIV_MULT:
            execute_div_mult(instr);
            break;
        case InstructionCategory::SHIFT:
            execute_shift(instr);
            break;
        case InstructionCategory::SHIFT_REG:
            execute_shift_reg(instr);
            break;
        case InstructionCategory::JUMP_REG:
            execute_jump_reg(instr);
            break;
        case InstructionCategory::MOVE_FROM:
            execute_move_from(instr);
            break;
        case InstructionCategory::MOVE_TO:
            execute_move_to(instr);
            break;
        case InstructionCategory::ARITH_LOGIC_IMM:
            execute_arith_logic_imm(instr);
            break;
        case InstructionCategory::LOAD_IMM:
            execute_load_imm(instr);
            break;
        case InstructionCategory::BRANCH:
            execute_branch(instr);
            break;
        case InstructionCategory::BRANCH_ZERO:
            execute_branch_zero(instr);
            break;
        case InstructionCategory::LOAD_STORE:
            execute_load_store(instr);
            break;
        case InstructionCategory::JUMP:
            execute_jump(instr);
            break;
        case InstructionCategory::TRAP:
            execute_trap(instr);
            break;
    }
    
    // Increment PC for most instructions (jumps/branches handle PC themselves)
    if (instr.category != InstructionCategory::JUMP && 
        instr.category != InstructionCategory::JUMP_REG &&
        instr.category != InstructionCategory::BRANCH &&
        instr.category != InstructionCategory::BRANCH_ZERO) {
        state_.set_pc(state_.get_pc() + 4);
    }
}

void CPU::run() {
    while (!halted_) {
        run_single_step();
    }
}

void CPU::run_single_step() {
    if (halted_) return;
    
    uint32_t pc = state_.get_pc();
    uint32_t instruction_word = state_.load_word(pc);
    
    // Check for null instruction
    if (instruction_word == 0) {
        state_.set_pc(pc + 4); // Skip null instruction (NOP)
        return;
    }
    
    Instruction instr = Instruction::decode(instruction_word);
    
    // Set instruction category and name based on opcode/function
    determine_instruction_info(instr);
    
    execute_instruction(instr);
}

void CPU::reset() {
    state_ = MachineState();
    halted_ = false;
}

// helper funcs
int32_t CPU::sign_extend_16(uint16_t value) {
    return static_cast<int32_t>(static_cast<int16_t>(value));
}

uint32_t CPU::zero_extend_16(uint16_t value) {
    return static_cast<uint32_t>(value);
}

int32_t CPU::sign_extend_8(uint8_t value) {
    return static_cast<int32_t>(static_cast<int8_t>(value));
}

uint32_t CPU::zero_extend_8(uint8_t value) {
    return static_cast<uint32_t>(value);
}

// utility funcs
Register string_to_register(const std::string& reg_name) {
    static std::unordered_map<std::string, Register> reg_map = {
        {"$zero", Register::ZERO}, {"$at", Register::AT},
        {"$v0", Register::V0}, {"$v1", Register::V1},
        {"$a0", Register::A0}, {"$a1", Register::A1}, {"$a2", Register::A2}, {"$a3", Register::A3},
        {"$t0", Register::T0}, {"$t1", Register::T1}, {"$t2", Register::T2}, {"$t3", Register::T3},
        {"$t4", Register::T4}, {"$t5", Register::T5}, {"$t6", Register::T6}, {"$t7", Register::T7},
        {"$s0", Register::S0}, {"$s1", Register::S1}, {"$s2", Register::S2}, {"$s3", Register::S3},
        {"$s4", Register::S4}, {"$s5", Register::S5}, {"$s6", Register::S6}, {"$s7", Register::S7},
        {"$t8", Register::T8}, {"$t9", Register::T9},
        {"$k0", Register::K0}, {"$k1", Register::K1},
        {"$gp", Register::GP}, {"$sp", Register::SP}, {"$s8", Register::S8}, {"$ra", Register::RA}
    };
    
    auto it = reg_map.find(reg_name); // find register name in map
    if (it != reg_map.end()) {
        return it->second;
    }
    throw std::invalid_argument("Invalid register name: " + reg_name);
}

std::string register_to_string(Register reg) {
    static std::array<std::string, 32> reg_names = {
        "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$s8", "$ra"
    };
    
    uint8_t index = static_cast<uint8_t>(reg);
    if (index < 32) {
        return reg_names[index];
    }
    return "$unknown";
}

void CPU::determine_instruction_info(Instruction& instr) {
    if (instr.opcode == 0) {
        // R-type instruction
        switch (instr.function) {
            case 0b000000: instr.name = "sll"; instr.category = InstructionCategory::SHIFT; break;
            case 0b000010: instr.name = "srl"; instr.category = InstructionCategory::SHIFT; break;
            case 0b000011: instr.name = "sra"; instr.category = InstructionCategory::SHIFT; break;
            case 0b000100: instr.name = "sllv"; instr.category = InstructionCategory::SHIFT_REG; break;
            case 0b000110: instr.name = "srlv"; instr.category = InstructionCategory::SHIFT_REG; break;
            case 0b000111: instr.name = "srav"; instr.category = InstructionCategory::SHIFT_REG; break;
            case 0b001000: instr.name = "jr"; instr.category = InstructionCategory::JUMP_REG; break;
            case 0b001001: instr.name = "jalr"; instr.category = InstructionCategory::JUMP_REG; break;
            case 0b010000: instr.name = "mfhi"; instr.category = InstructionCategory::MOVE_FROM; break;
            case 0b010001: instr.name = "mthi"; instr.category = InstructionCategory::MOVE_TO; break;
            case 0b010010: instr.name = "mflo"; instr.category = InstructionCategory::MOVE_FROM; break;
            case 0b010011: instr.name = "mtlo"; instr.category = InstructionCategory::MOVE_TO; break;
            case 0b011000: instr.name = "mult"; instr.category = InstructionCategory::DIV_MULT; break;
            case 0b011001: instr.name = "multu"; instr.category = InstructionCategory::DIV_MULT; break;
            case 0b011010: instr.name = "div"; instr.category = InstructionCategory::DIV_MULT; break;
            case 0b011011: instr.name = "divu"; instr.category = InstructionCategory::DIV_MULT; break;
            case 0b100000: instr.name = "add"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100001: instr.name = "addu"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100010: instr.name = "sub"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100011: instr.name = "subu"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100100: instr.name = "and"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100101: instr.name = "or"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100110: instr.name = "xor"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b100111: instr.name = "nor"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b101010: instr.name = "slt"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            case 0b101011: instr.name = "sltu"; instr.category = InstructionCategory::ARITH_LOGIC; break;
            default: instr.name = "unknown"; instr.category = InstructionCategory::ARITH_LOGIC; break;
        }
    } else {
        // I-type and J-type instructions
        switch (instr.opcode) {
            case 0b000010: instr.name = "j"; instr.category = InstructionCategory::JUMP; break;
            case 0b000011: instr.name = "jal"; instr.category = InstructionCategory::JUMP; break;
            case 0b000100: instr.name = "beq"; instr.category = InstructionCategory::BRANCH; break;
            case 0b000101: instr.name = "bne"; instr.category = InstructionCategory::BRANCH; break;
            case 0b000110: instr.name = "blez"; instr.category = InstructionCategory::BRANCH_ZERO; break;
            case 0b000111: instr.name = "bgtz"; instr.category = InstructionCategory::BRANCH_ZERO; break;
            case 0b001000: instr.name = "addi"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001001: instr.name = "addiu"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001010: instr.name = "slti"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001011: instr.name = "sltiu"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001100: instr.name = "andi"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001101: instr.name = "ori"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b001110: instr.name = "xori"; instr.category = InstructionCategory::ARITH_LOGIC_IMM; break;
            case 0b011000: instr.name = "llo"; instr.category = InstructionCategory::LOAD_IMM; break;
            case 0b011001: instr.name = "lhi"; instr.category = InstructionCategory::LOAD_IMM; break;
            case 0b011010: instr.name = "trap"; instr.category = InstructionCategory::TRAP; break;
            case 0b100000: instr.name = "lb"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b100001: instr.name = "lh"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b100011: instr.name = "lw"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b100100: instr.name = "lbu"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b100101: instr.name = "lhu"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b101000: instr.name = "sb"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b101001: instr.name = "sh"; instr.category = InstructionCategory::LOAD_STORE; break;
            case 0b101011: instr.name = "sw"; instr.category = InstructionCategory::LOAD_STORE; break;
            default: instr.name = "unknown"; instr.category = InstructionCategory::ARITH_LOGIC; break;
        }
    }
}

} // namespace mips
