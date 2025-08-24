#include "mips_core.h"
#include "assembler.h"
#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::cout << "MIPS Interpreter Test Program" << std::endl;
    
    // Test basic functionality
    try {
        // Create a simple test program
        std::string test_program = R"(
main:
    addi $t0, $zero, 42
    addi $t1, $zero, 10
    add $t2, $t0, $t1
    addi $a0, $t2, 0
    trap 0
    trap 5
)";
        
        std::cout << "Test program:" << std::endl;
        std::cout << test_program << std::endl;
        
        // Assemble the program
        mips::Assembler assembler;
        auto binary_data = assembler.assemble_text(test_program);
        
        if (assembler.has_errors()) {
            std::cout << "Assembly errors:" << std::endl;
            for (const auto& error : assembler.get_errors()) {
                std::cout << "  " << error << std::endl;
            }
            return 1;
        }
        
        std::cout << "Assembly successful! Binary size: " << binary_data.size() << " bytes" << std::endl;
        std::cout << "Main address: 0x" << std::hex << assembler.get_main_address() << std::dec << std::endl;
        
        // Create CPU and run program
        mips::CPU cpu;
        cpu.get_state().load_memory(binary_data, 0); // store bytes in pages
        cpu.get_state().set_pc(assembler.get_main_address()); // set pc to main address
        cpu.get_state().set_register(mips::Register::SP, 0xFFFFFFFC); // set sp to end of memory
        
        std::cout << "\nExecuting program..." << std::endl;
        std::cout << "Output: ";
        
        // debug: Print first few instructions
        std::cout << "\nFirst instruction at PC=0: 0x" << std::hex 
                  << cpu.get_state().load_word(0) << std::dec << std::endl;
        
        // run with step limit to avoid infinite loop
        int step_count = 0;
        const int MAX_STEPS = 1000;
        
        while (!cpu.is_halted() && step_count < MAX_STEPS) {
            cpu.run_single_step();
            step_count++;
        }
        
        // debug
        if (step_count >= MAX_STEPS) {
            std::cout << "\nProgram stopped after " << MAX_STEPS << " steps (possible infinite loop)" << std::endl;
        }
        
        std::cout << std::endl;
        
        std::cout << "Program completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}