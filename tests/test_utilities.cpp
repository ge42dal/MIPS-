#include "catch2.hpp"
#include "../src/mips_core.h"
#include <stdexcept>

TEST_CASE("Utilities - Register name conversion") {
    // Test valid register names
    REQUIRE_EQ(mips::string_to_register("$zero"), mips::Register::ZERO);
    REQUIRE_EQ(mips::string_to_register("$t0"), mips::Register::T0);
    REQUIRE_EQ(mips::string_to_register("$ra"), mips::Register::RA);
    REQUIRE_EQ(mips::string_to_register("$sp"), mips::Register::SP);
    
    // Test invalid register names throw exceptions
    REQUIRE_THROWS(mips::string_to_register("$invalid"));
    REQUIRE_THROWS(mips::string_to_register("t0")); // Missing $
    REQUIRE_THROWS(mips::string_to_register(""));
}

TEST_CASE("Utilities - Register to string conversion") {
    REQUIRE_EQ(mips::register_to_string(mips::Register::ZERO), "$zero");
    REQUIRE_EQ(mips::register_to_string(mips::Register::T0), "$t0");
    REQUIRE_EQ(mips::register_to_string(mips::Register::RA), "$ra");
    REQUIRE_EQ(mips::register_to_string(mips::Register::SP), "$sp");
}

TEST_CASE("MachineState - PC operations") {
    mips::MachineState state;
    
    // Test initial PC
    REQUIRE_EQ(state.get_pc(), 0);
    
    // Test PC setting
    state.set_pc(0x1000);
    REQUIRE_EQ(state.get_pc(), 0x1000);
    
    // Test PC increment
    state.set_pc(state.get_pc() + 4);
    REQUIRE_EQ(state.get_pc(), 0x1004);
}

TEST_CASE("MachineState - HI/LO registers") {
    mips::MachineState state;
    
    // Test initial values
    REQUIRE_EQ(state.get_hi(), 0);
    REQUIRE_EQ(state.get_lo(), 0);
    
    // Test setting values
    state.set_hi(0xDEADBEEF);
    state.set_lo(0x12345678);
    
    REQUIRE_EQ(state.get_hi(), 0xDEADBEEF);
    REQUIRE_EQ(state.get_lo(), 0x12345678);
}

TEST_CASE("MachineState - Little-endian memory layout") {
    mips::MachineState state;
    
    // Store a 32-bit value
    uint32_t test_value = 0x12345678;
    state.store_word(0x1000, test_value);
    
    // Verify little-endian byte order
    REQUIRE_EQ(state.load_byte(0x1000), 0x78); // LSB first
    REQUIRE_EQ(state.load_byte(0x1001), 0x56);
    REQUIRE_EQ(state.load_byte(0x1002), 0x34);
    REQUIRE_EQ(state.load_byte(0x1003), 0x12); // MSB last
    
    // Verify 16-bit loads
    REQUIRE_EQ(state.load_half(0x1000), 0x5678);
    REQUIRE_EQ(state.load_half(0x1002), 0x1234);
}

TEST_CASE("CPU - Sign extension functions") {
    mips::CPU cpu;
    
    // Test positive 16-bit value
    int32_t result1 = cpu.sign_extend_16(0x1234);
    REQUIRE_EQ(result1, 0x1234);
    
    // Test negative 16-bit value (MSB set)
    int32_t result2 = cpu.sign_extend_16(0x8000);
    REQUIRE_EQ(result2, -32768);
    
    // Test -1 (all bits set in 16-bit)
    int32_t result3 = cpu.sign_extend_16(0xFFFF);
    REQUIRE_EQ(result3, -1);
    
    // Test positive 8-bit value
    int32_t result4 = cpu.sign_extend_8(0x7F);
    REQUIRE_EQ(result4, 127);
    
    // Test negative 8-bit value
    int32_t result5 = cpu.sign_extend_8(0x80);
    REQUIRE_EQ(result5, -128);
}

TEST_CASE("CPU - Zero extension functions") {
    mips::CPU cpu;
    
    // Test 16-bit zero extension
    uint32_t result1 = cpu.zero_extend_16(0xFFFF);
    REQUIRE_EQ(result1, 0x0000FFFF);
    
    uint32_t result2 = cpu.zero_extend_16(0x1234);
    REQUIRE_EQ(result2, 0x00001234);
}
