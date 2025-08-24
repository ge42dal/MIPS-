#include "mips_core.h"
#include "assembler.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <assembly_file>" << std::endl;
        return 1;
    }
    
    try {
        // read and assemble the text file
        std::ifstream input(argv[1]);
        if (!input) {
            std::cerr << "Error: Cannot open input file: " << argv[1] << std::endl;
            return 1;
        }
        
        mips::Assembler assembler;
        auto binary_data = assembler.assemble_stream(input);
        
        if (assembler.has_errors()) {
            for (const auto& error : assembler.get_errors()) {
                std::cerr << "Assembly Error: " << error << std::endl;
            }
            return 1;
        }
        
        uint32_t main_address = assembler.get_main_address();
        if (main_address == 0 && binary_data.size() > 0) {
            std::cerr << "Error: No 'main' label found in assembly file" << std::endl;
            return 1;
        }
        
        // create CPU and load program
        mips::CPU cpu;
        cpu.get_state().load_memory(binary_data, 0);
        cpu.get_state().set_pc(main_address);
        
        // init stack pointer to end of memory
        cpu.get_state().set_register(mips::Register::SP, 0xFFFFFFFC);
        
        // run the program
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
