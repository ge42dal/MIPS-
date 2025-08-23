#include "assembler.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    mips::Assembler assembler;
    
    try {
        if (argc == 1) {
            // Read from stdin, write to stdout
            auto binary_data = assembler.assemble_stream(std::cin);
            if (assembler.has_errors()) {
                for (const auto& error : assembler.get_errors()) {
                    std::cerr << "Error: " << error << std::endl;
                }
                return 1;
            }
            mips::BinaryFormat::write_binary(binary_data, std::cout, assembler.get_main_address());
        }
        else if (argc == 2) {
            // Read from input file, write to stdout
            std::ifstream input(argv[1]);
            if (!input) {
                std::cerr << "Error: Cannot open input file: " << argv[1] << std::endl;
                return 1;
            }
            
            auto binary_data = assembler.assemble_stream(input);
            if (assembler.has_errors()) {
                for (const auto& error : assembler.get_errors()) {
                    std::cerr << "Error: " << error << std::endl;
                }
                return 1;
            }
            mips::BinaryFormat::write_binary(binary_data, std::cout, assembler.get_main_address());
        }
        else if (argc == 3) {
            // Read from input file, write to output file
            std::ifstream input(argv[1]);
            if (!input) {
                std::cerr << "Error: Cannot open input file: " << argv[1] << std::endl;
                return 1;
            }
            
            auto binary_data = assembler.assemble_stream(input);
            if (assembler.has_errors()) {
                for (const auto& error : assembler.get_errors()) {
                    std::cerr << "Error: " << error << std::endl;
                }
                return 1;
            }
            
            mips::BinaryFormat::write_binary_file(binary_data, argv[2], assembler.get_main_address());
            std::cout << "Assembly completed successfully. Output written to " << argv[2] << std::endl;
        }
        else {
            std::cerr << "Usage:" << std::endl;
            std::cerr << "  " << argv[0] << "                    # Read from stdin, write to stdout" << std::endl;
            std::cerr << "  " << argv[0] << " input.asm         # Read from file, write to stdout" << std::endl;
            std::cerr << "  " << argv[0] << " input.asm output.bin # Read from file, write to file" << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
