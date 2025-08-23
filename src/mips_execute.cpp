#include "mips_core.h"
#include "assembler.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }
    
    try {
        // Read binary file
        uint32_t main_address;
        auto binary_data = mips::BinaryFormat::read_binary_file(argv[1], main_address);
        
        // Create CPU and load program
        mips::CPU cpu;
        cpu.get_state().load_memory(binary_data, 0);
        cpu.get_state().set_pc(main_address);
        
        // Initialize stack pointer to end of memory
        cpu.get_state().set_register(mips::Register::SP, 0xFFFFFFFC);
        
        // Run the program
        std::cout << "Starting MIPS program execution at address 0x" 
                  << std::hex << main_address << std::dec << std::endl;
        
        cpu.run();
        
        std::cout << "\nProgram execution completed." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
