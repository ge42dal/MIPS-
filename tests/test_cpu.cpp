#include "catch2.hpp"
#include "../src/mips_core.h"
#include "../src/assembler.h"
#include <sstream>

TEST_CASE("CPU - Register operations") {
    mips::MachineState state;
    
    // Test register initialization
    REQUIRE_EQ(state.get_register(mips::Register::ZERO), 0);
    REQUIRE_EQ(state.get_register(mips::Register::T0), 0);
    
    // Test register setting (except $zero)
    state.set_register(mips::Register::T0, 42);
    REQUIRE_EQ(state.get_register(mips::Register::T0), 42);
    
    // Test $zero register immutability
    state.set_register(mips::Register::ZERO, 100);
    REQUIRE_EQ(state.get_register(mips::Register::ZERO), 0);
}

TEST_CASE("CPU - Memory operations") {
    mips::MachineState state;
    
    // Test byte operations
    state.store_byte(0x1000, 0xFF);
    REQUIRE_EQ(state.load_byte(0x1000), 0xFF);
    
    // Test word operations
    state.store_word(0x2000, 0xDEADBEEF);
    REQUIRE_EQ(state.load_word(0x2000), 0xDEADBEEF);
    
    // Test uninitialized memory reads as 0
    REQUIRE_EQ(state.load_byte(0x50000000), 0);
    REQUIRE_EQ(state.load_word(0x50000000), 0);
}

TEST_CASE("CPU - Memory bounds checking") {
    mips::MachineState state;
    
    // Test out of bounds access throws exception
    REQUIRE_THROWS(state.store_byte(0xFFFFFFFF, 0xFF));
    REQUIRE_THROWS(state.load_word(0xFFFFFFFE)); // Would read past end
}

TEST_CASE("CPU - Arithmetic instruction execution") {
    mips::CPU cpu;
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 10
    addi $t1, $zero, 5
    add $t2, $t0, $t1
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    
    // Load program into CPU
    for (size_t i = 0; i < binary.size(); ++i) {
        cpu.get_state().store_byte(i, binary[i]);
    }
    cpu.get_state().set_pc(0);
    
    // Execute first instruction: addi $t0, $zero, 10
    cpu.run_single_step();
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T0), 10);
    
    // Execute second instruction: addi $t1, $zero, 5
    cpu.run_single_step();
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T1), 5);
    
    // Execute third instruction: add $t2, $t0, $t1
    cpu.run_single_step();
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T2), 15);
}

TEST_CASE("CPU - Memory load/store instructions") {
    mips::CPU cpu;
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 0x1000
    addi $t1, $zero, 42
    sw $t1, 0($t0)
    lw $t2, 0($t0)
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    
    // Load program into CPU
    for (size_t i = 0; i < binary.size(); ++i) {
        cpu.get_state().store_byte(i, binary[i]);
    }
    cpu.get_state().set_pc(0);
    
    // Execute instructions
    cpu.run_single_step(); // addi $t0, $zero, 0x1000
    cpu.run_single_step(); // addi $t1, $zero, 42
    cpu.run_single_step(); // sw $t1, 0($t0)
    cpu.run_single_step(); // lw $t2, 0($t0)
    
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T2), 42);
}

TEST_CASE("CPU - Jump instruction execution") {
    mips::CPU cpu;
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    j target
    addi $t0, $zero, 999
target:
    addi $t1, $zero, 42
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    
    // Load program into CPU
    for (size_t i = 0; i < binary.size(); ++i) {
        cpu.get_state().store_byte(i, binary[i]);
    }
    cpu.get_state().set_pc(0);
    
    // Execute jump instruction
    cpu.run_single_step(); // j target
    
    // Should skip the addi $t0 instruction
    cpu.run_single_step(); // addi $t1, $zero, 42
    
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T0), 0); // Should not be set
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T1), 42); // Should be set
}

TEST_CASE("CPU - Branch instruction execution") {
    mips::CPU cpu;
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 5
    addi $t1, $zero, 5
    beq $t0, $t1, equal
    addi $t2, $zero, 999
equal:
    addi $t3, $zero, 42
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    
    // Load program into CPU
    for (size_t i = 0; i < binary.size(); ++i) {
        cpu.get_state().store_byte(i, binary[i]);
    }
    cpu.get_state().set_pc(0);
    
    // Execute instructions
    cpu.run_single_step(); // addi $t0, $zero, 5
    cpu.run_single_step(); // addi $t1, $zero, 5
    cpu.run_single_step(); // beq $t0, $t1, equal (should branch)
    cpu.run_single_step(); // addi $t3, $zero, 42
    
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T2), 0); // Should not be set
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T3), 42); // Should be set
}

TEST_CASE("CPU - System call execution") {
    mips::CPU cpu;
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $a0, $zero, 42
    trap 0
    trap 5
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    
    // Load program into CPU
    for (size_t i = 0; i < binary.size(); ++i) {
        cpu.get_state().store_byte(i, binary[i]);
    }
    cpu.get_state().set_pc(0);
    
    // Execute instructions
    cpu.run_single_step(); // addi $a0, $zero, 42
    cpu.run_single_step(); // trap 0 (print_int)
    cpu.run_single_step(); // trap 5 (exit)
    
    REQUIRE(cpu.is_halted());
}
