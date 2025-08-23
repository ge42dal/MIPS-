#include "mips_core.h"
#include <iostream>

namespace mips {

void CPU::execute_arith_logic(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    Register rd_reg = static_cast<Register>(instr.rd);
    
    uint32_t rs_val = state_.get_register(rs_reg);
    uint32_t rt_val = state_.get_register(rt_reg);
    uint32_t result = 0;
    
    switch (instr.function) {
        case 0b100000: // add
            result = static_cast<uint32_t>(static_cast<int32_t>(rs_val) + static_cast<int32_t>(rt_val));
            break;
        case 0b100001: // addu
            result = rs_val + rt_val;
            break;
        case 0b100010: // sub
            result = static_cast<uint32_t>(static_cast<int32_t>(rs_val) - static_cast<int32_t>(rt_val));
            break;
        case 0b100011: // subu
            result = rs_val - rt_val;
            break;
        case 0b100100: // and
            result = rs_val & rt_val;
            break;
        case 0b100101: // or
            result = rs_val | rt_val;
            break;
        case 0b100110: // xor
            result = rs_val ^ rt_val;
            break;
        case 0b100111: // nor
            result = ~(rs_val | rt_val);
            break;
        case 0b101010: // slt
            result = (static_cast<int32_t>(rs_val) < static_cast<int32_t>(rt_val)) ? 1 : 0;
            break;
        case 0b101011: // sltu
            result = (rs_val < rt_val) ? 1 : 0;
            break;
    }
    
    state_.set_register(rd_reg, result);
}

void CPU::execute_div_mult(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    
    uint32_t rs_val = state_.get_register(rs_reg);
    uint32_t rt_val = state_.get_register(rt_reg);
    
    switch (instr.function) {
        case 0b011000: { // mult
            int64_t result = static_cast<int64_t>(static_cast<int32_t>(rs_val)) * 
                           static_cast<int64_t>(static_cast<int32_t>(rt_val));
            state_.set_lo(static_cast<uint32_t>(result & 0xFFFFFFFF));
            state_.set_hi(static_cast<uint32_t>((result >> 32) & 0xFFFFFFFF));
            break;
        }
        case 0b011001: { // multu
            uint64_t result = static_cast<uint64_t>(rs_val) * static_cast<uint64_t>(rt_val);
            state_.set_lo(static_cast<uint32_t>(result & 0xFFFFFFFF));
            state_.set_hi(static_cast<uint32_t>((result >> 32) & 0xFFFFFFFF));
            break;
        }
        case 0b011010: // div
            if (rt_val != 0) {
                state_.set_lo(static_cast<uint32_t>(static_cast<int32_t>(rs_val) / static_cast<int32_t>(rt_val)));
                state_.set_hi(static_cast<uint32_t>(static_cast<int32_t>(rs_val) % static_cast<int32_t>(rt_val)));
            }
            break;
        case 0b011011: // divu
            if (rt_val != 0) {
                state_.set_lo(rs_val / rt_val);
                state_.set_hi(rs_val % rt_val);
            }
            break;
    }
}

void CPU::execute_shift(const Instruction& instr) {
    Register rt_reg = static_cast<Register>(instr.rt);
    Register rd_reg = static_cast<Register>(instr.rd);
    
    uint32_t rt_val = state_.get_register(rt_reg);
    uint32_t shamt = instr.shamt;
    uint32_t result = 0;
    
    switch (instr.function) {
        case 0b000000: // sll
            result = rt_val << shamt;
            break;
        case 0b000010: // srl
            result = rt_val >> shamt;
            break;
        case 0b000011: // sra
            result = static_cast<uint32_t>(static_cast<int32_t>(rt_val) >> shamt);
            break;
    }
    
    state_.set_register(rd_reg, result);
}

void CPU::execute_shift_reg(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    Register rd_reg = static_cast<Register>(instr.rd);
    
    uint32_t rs_val = state_.get_register(rs_reg) & 0x1F; // Only use lower 5 bits
    uint32_t rt_val = state_.get_register(rt_reg);
    uint32_t result = 0;
    
    switch (instr.function) {
        case 0b000100: // sllv
            result = rt_val << rs_val;
            break;
        case 0b000110: // srlv
            result = rt_val >> rs_val;
            break;
        case 0b000111: // srav
            result = static_cast<uint32_t>(static_cast<int32_t>(rt_val) >> rs_val);
            break;
    }
    
    state_.set_register(rd_reg, result);
}

void CPU::execute_jump_reg(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    uint32_t rs_val = state_.get_register(rs_reg);
    
    switch (instr.function) {
        case 0b001000: // jr
            state_.set_pc(rs_val);
            break;
        case 0b001001: // jalr
            state_.set_register(Register::RA, state_.get_pc() + 4);
            state_.set_pc(rs_val);
            break;
    }
}

void CPU::execute_move_from(const Instruction& instr) {
    Register rd_reg = static_cast<Register>(instr.rd);
    
    switch (instr.function) {
        case 0b010000: // mfhi
            state_.set_register(rd_reg, state_.get_hi());
            break;
        case 0b010010: // mflo
            state_.set_register(rd_reg, state_.get_lo());
            break;
    }
}

void CPU::execute_move_to(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    uint32_t rs_val = state_.get_register(rs_reg);
    
    switch (instr.function) {
        case 0b010001: // mthi
            state_.set_hi(rs_val);
            break;
        case 0b010011: // mtlo
            state_.set_lo(rs_val);
            break;
    }
}

void CPU::execute_arith_logic_imm(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    
    uint32_t rs_val = state_.get_register(rs_reg);
    uint32_t result = 0;
    
    switch (instr.opcode) {
        case 0b001000: // addi
            result = rs_val + sign_extend_16(instr.immediate);
            break;
        case 0b001001: // addiu
            result = rs_val + sign_extend_16(instr.immediate);
            break;
        case 0b001010: // slti
            result = (static_cast<int32_t>(rs_val) < sign_extend_16(instr.immediate)) ? 1 : 0;
            break;
        case 0b001011: // sltiu
            result = (rs_val < static_cast<uint32_t>(sign_extend_16(instr.immediate))) ? 1 : 0;
            break;
        case 0b001100: // andi
            result = rs_val & zero_extend_16(instr.immediate);
            break;
        case 0b001101: // ori
            result = rs_val | zero_extend_16(instr.immediate);
            break;
        case 0b001110: // xori
            result = rs_val ^ zero_extend_16(instr.immediate);
            break;
    }
    
    state_.set_register(rt_reg, result);
}

void CPU::execute_load_imm(const Instruction& instr) {
    Register rt_reg = static_cast<Register>(instr.rt);
    uint32_t rt_val = state_.get_register(rt_reg);
    uint32_t result = 0;
    
    switch (instr.opcode) {
        case 0b011000: // llo (load lower immediate)
            result = (rt_val & 0xFFFF0000u) | instr.immediate;
            break;
        case 0b011001: // lhi (load higher immediate)
            result = (rt_val & 0x0000FFFFu) | (instr.immediate << 16);
            break;
    }
    
    state_.set_register(rt_reg, result);
}

void CPU::execute_branch(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    
    uint32_t rs_val = state_.get_register(rs_reg);
    uint32_t rt_val = state_.get_register(rt_reg);
    
    bool branch_taken = false;
    
    switch (instr.opcode) {
        case 0b000100: // beq
            branch_taken = (rs_val == rt_val);
            break;
        case 0b000101: // bne
            branch_taken = (rs_val != rt_val);
            break;
    }
    
    if (branch_taken) {
        int32_t offset = sign_extend_16(instr.immediate) << 2;
        state_.set_pc(state_.get_pc() + 4 + offset);
    } else {
        state_.set_pc(state_.get_pc() + 4);
    }
}

void CPU::execute_branch_zero(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    int32_t rs_val = static_cast<int32_t>(state_.get_register(rs_reg));
    
    bool branch_taken = false;
    
    switch (instr.opcode) {
        case 0b000110: // blez
            branch_taken = (rs_val <= 0);
            break;
        case 0b000111: // bgtz
            branch_taken = (rs_val > 0);
            break;
    }
    
    if (branch_taken) {
        int32_t offset = sign_extend_16(instr.immediate) << 2;
        state_.set_pc(state_.get_pc() + 4 + offset);
    } else {
        state_.set_pc(state_.get_pc() + 4);
    }
}

void CPU::execute_load_store(const Instruction& instr) {
    Register rs_reg = static_cast<Register>(instr.rs);
    Register rt_reg = static_cast<Register>(instr.rt);
    
    uint32_t rs_val = state_.get_register(rs_reg);
    uint32_t address = rs_val + sign_extend_16(instr.immediate);
    
    switch (instr.opcode) {
        case 0b100000: // lb
            state_.set_register(rt_reg, sign_extend_8(state_.load_byte(address)));
            break;
        case 0b100001: // lh
            state_.set_register(rt_reg, sign_extend_16(state_.load_half(address)));
            break;
        case 0b100011: // lw
            state_.set_register(rt_reg, state_.load_word(address));
            break;
        case 0b100100: // lbu
            state_.set_register(rt_reg, zero_extend_8(state_.load_byte(address)));
            break;
        case 0b100101: // lhu
            state_.set_register(rt_reg, zero_extend_16(state_.load_half(address)));
            break;
        case 0b101000: // sb
            state_.store_byte(address, static_cast<uint8_t>(state_.get_register(rt_reg) & 0xFF));
            break;
        case 0b101001: // sh
            state_.store_half(address, static_cast<uint16_t>(state_.get_register(rt_reg) & 0xFFFF));
            break;
        case 0b101011: // sw
            state_.store_word(address, state_.get_register(rt_reg));
            break;
    }
}

void CPU::execute_jump(const Instruction& instr) {
    switch (instr.opcode) {
        case 0b000010: // j
            state_.set_pc(instr.address << 2);
            break;
        case 0b000011: // jal
            state_.set_register(Register::RA, state_.get_pc() + 4);
            state_.set_pc(instr.address << 2);
            break;
    }
}

void CPU::execute_trap(const Instruction& instr) {
    uint32_t syscall_num = instr.immediate;
    
    switch (syscall_num) {
        case 0: { // print_int
            uint32_t value = state_.get_register(Register::A0);
            *state_.output_stream << static_cast<int32_t>(value);
            break;
        }
        case 1: { // print_character
            uint32_t value = state_.get_register(Register::A0);
            *state_.output_stream << static_cast<char>(value & 0xFF);
            break;
        }
        case 2: { // print_string
            uint32_t address = state_.get_register(Register::A0);
            while (true) {
                uint8_t c = state_.load_byte(address);
                if (c == 0) break;
                *state_.output_stream << static_cast<char>(c);
                address++;
            }
            break;
        }
        case 3: { // read_int
            int32_t value;
            *state_.input_stream >> value;
            state_.set_register(Register::V0, static_cast<uint32_t>(value));
            break;
        }
        case 4: { // read_character
            char c;
            *state_.input_stream >> c;
            state_.set_register(Register::V0, static_cast<uint32_t>(c));
            break;
        }
        case 5: // exit
            halted_ = true;
            break;
    }
}

} // namespace mips
