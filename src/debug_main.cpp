#include "debugger.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <assembly_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " program.asm" << std::endl;
        return 1;
    }
    
    std::string assembly_file = argv[1];
    
    try {
        std::cout << "Creating debugger..." << std::endl;
        mips::Debugger debugger;
        
        std::cout << "Loading assembly file: " << assembly_file << std::endl;
        
        if (!debugger.load_program(assembly_file)) {
            std::cerr << "Failed to load program from " << assembly_file << std::endl;
            return 1;
        }
        
        std::cout << "Starting debugging session..." << std::endl;
        // start debugging sesh
        debugger.run();
        
        std::cout << "Debugging session ended." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
