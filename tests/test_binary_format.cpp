#include "catch2.hpp"
#include "../src/assembler.h"
#include "../src/mips_core.h"
#include <sstream>

TEST_CASE("BinaryFormat - Write and read binary data") {
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 42
    add $t1, $t0, $zero
    trap 5
)";
    
    auto binary_data = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    uint32_t main_address = assembler.get_main_address();
    
    // write binary to stream
    std::ostringstream output_stream;
    mips::BinaryFormat::write_binary(binary_data, output_stream, main_address);
    
    // read binary back from stream
    std::istringstream input_stream(output_stream.str());
    uint32_t read_main_address;
    auto read_binary = mips::BinaryFormat::read_binary(input_stream, read_main_address);
    
    // verify data integrity
    REQUIRE_EQ(read_main_address, main_address);
    REQUIRE_EQ(read_binary.size(), binary_data.size());
    
    for (size_t i = 0; i < binary_data.size(); ++i) {
        REQUIRE_EQ(read_binary[i], binary_data[i]);
    }
}

TEST_CASE("BinaryFormat - Empty program handling") {
    std::vector<uint8_t> empty_data;
    uint32_t main_address = 0;
    
    std::ostringstream output_stream;
    mips::BinaryFormat::write_binary(empty_data, output_stream, main_address);
    
    std::istringstream input_stream(output_stream.str());
    uint32_t read_main_address;
    auto read_binary = mips::BinaryFormat::read_binary(input_stream, read_main_address);
    
    REQUIRE_EQ(read_main_address, main_address);
    REQUIRE_EQ(read_binary.size(), 0);
}

TEST_CASE("Instruction - Decode and encode roundtrip") {
    // test R-type instruction
    uint32_t original_word = 0x01094020; // add $t0, $t0, $t1
    mips::Instruction instr = mips::Instruction::decode(original_word);
    
    REQUIRE_EQ(instr.opcode, 0);
    REQUIRE_EQ(instr.function, 0x20); // add function code
    
    uint32_t encoded_word = instr.encode();
    REQUIRE_EQ(encoded_word, original_word);
}

TEST_CASE("Instruction - Type detection") {
    // Test R-type detection
    uint32_t r_type_word = 0x01094020; // add instruction
    mips::Instruction r_instr = mips::Instruction::decode(r_type_word);
    REQUIRE_EQ(r_instr.type, mips::InstructionType::R_TYPE);
    
    // Test I-type detection
    uint32_t i_type_word = 0x2108002A; // addi $t0, $t0, 42
    mips::Instruction i_instr = mips::Instruction::decode(i_type_word);
    REQUIRE_EQ(i_instr.type, mips::InstructionType::I_TYPE);
    
    // Test J-type detection
    uint32_t j_type_word = 0x08000010; // j 0x40
    mips::Instruction j_instr = mips::Instruction::decode(j_type_word);
    REQUIRE_EQ(j_instr.type, mips::InstructionType::J_TYPE);
}
