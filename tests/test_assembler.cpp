#include "catch2.hpp"
#include "../src/assembler.h"
#include "../src/mips_core.h"
#include <sstream>

TEST_CASE("Assembler - Valid instruction parsing") {
    mips::Assembler assembler;
    
    std::string valid_program = R"(
main:
    addi $t0, $zero, 42
    add $t1, $t0, $zero
    trap 5
)";
    
    auto lines = assembler.parse_assembly(valid_program);
    REQUIRE_FALSE(assembler.has_errors());
    REQUIRE_EQ(lines.size(), 3);
    
    // check first instruction (main label should be on separate line)
    REQUIRE_EQ(lines[0].instruction, "addi");
    REQUIRE_EQ(lines[0].operands.size(), 3);
    REQUIRE_EQ(lines[0].operands[0], "$t0");
    REQUIRE_EQ(lines[0].operands[1], "$zero");
    REQUIRE_EQ(lines[0].operands[2], "42");
}

TEST_CASE("Assembler - Invalid instruction handling") {
    mips::Assembler assembler;
    
    std::string invalid_program = R"(
main:
    invalid_instruction $t0, $t1
    addi $t0
)";
    
    auto lines = assembler.parse_assembly(invalid_program);
    auto binary = assembler.assemble(lines);
    REQUIRE(assembler.has_errors());
    
    auto errors = assembler.get_errors();
    REQUIRE(errors.size() > 0);
}

TEST_CASE("Assembler - Binary generation") {
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 10
    trap 5
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    REQUIRE_EQ(binary.size(), 8); // 2 instructions * 4 bytes each
    
    // check main address is set
    REQUIRE_EQ(assembler.get_main_address(), 0);
}

TEST_CASE("Assembler - Register name validation") {
    mips::Assembler assembler;
    
    std::string invalid_reg_program = R"(
main:
    addi $invalid, $zero, 10
)";
    
    // this should throw an exception during assembly
    REQUIRE_THROWS(assembler.assemble_text(invalid_reg_program));
}

TEST_CASE("Assembler - Immediate value parsing") {
    mips::Assembler assembler;
    
    std::string program = R"(
main:
    addi $t0, $zero, 42
    addi $t1, $zero, -10
    addi $t2, $zero, 0
)";
    
    auto binary = assembler.assemble_text(program);
    REQUIRE_FALSE(assembler.has_errors());
    REQUIRE_EQ(binary.size(), 12); // 3 instructions * 4 bytes each
}
