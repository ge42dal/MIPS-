#include "catch2.hpp"
#include "../src/mips_core.h"
#include <stdexcept>

TEST_CASE("Utilities - Register name conversion") {
    // test valid register names
    REQUIRE_EQ(mips::string_to_register("$zero"), mips::Register::ZERO);
    REQUIRE_EQ(mips::string_to_register("$t0"), mips::Register::T0);
    REQUIRE_EQ(mips::string_to_register("$ra"), mips::Register::RA);
    REQUIRE_EQ(mips::string_to_register("$sp"), mips::Register::SP);
    
    // test invalid register names throw exceptions
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
    
    // test initial PC
    REQUIRE_EQ(state.get_pc(), 0);
    
    // test PC setting
    state.set_pc(0x1000);
    REQUIRE_EQ(state.get_pc(), 0x1000);
    
    // test PC increment
    state.set_pc(state.get_pc() + 4);
    REQUIRE_EQ(state.get_pc(), 0x1004);
}

TEST_CASE("MachineState - HI/LO registers") {
    mips::MachineState state;
    
    // test initial values
    REQUIRE_EQ(state.get_hi(), 0);
    REQUIRE_EQ(state.get_lo(), 0);
    
    // test setting values
    state.set_hi(0xDEADBEEF);
    state.set_lo(0x12345678);
    
    REQUIRE_EQ(state.get_hi(), 0xDEADBEEF);
    REQUIRE_EQ(state.get_lo(), 0x12345678);
}

TEST_CASE("MachineState - Little-endian memory layout") {
    mips::MachineState state;
    
    // store a 32-bit value
    uint32_t test_value = 0x12345678;
    state.store_word(0x1000, test_value);
    
    // verify little-endian byte order
    REQUIRE_EQ(state.load_byte(0x1000), 0x78); // LSB first
    REQUIRE_EQ(state.load_byte(0x1001), 0x56);
    REQUIRE_EQ(state.load_byte(0x1002), 0x34);
    REQUIRE_EQ(state.load_byte(0x1003), 0x12); // MSB last
    
    // verify 16-bit loads
    REQUIRE_EQ(state.load_half(0x1000), 0x5678);
    REQUIRE_EQ(state.load_half(0x1002), 0x1234);
}

TEST_CASE("CPU - Basic functionality") {
    mips::CPU cpu;
    
    // test CPU initialization
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::ZERO), 0);
    
    // test register operations
    cpu.get_state().set_register(mips::Register::T0, 0x12345678);
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::T0), 0x12345678);
    
    // test that ZERO register cannot be modified
    cpu.get_state().set_register(mips::Register::ZERO, 42);
    REQUIRE_EQ(cpu.get_state().get_register(mips::Register::ZERO), 0);
    
    // test PC operations
    cpu.get_state().set_pc(100);
    REQUIRE_EQ(cpu.get_state().get_pc(), 100);
}
