#include "src/mips_core.h"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // create MIPS machine state - should be instant now
    mips::MachineState state;
    
    auto after_init = std::chrono::high_resolution_clock::now();
    
    // test memory operations
    std::cout << "Testing memory operations...\n";
    
    // write to various memory locations (should allocate pages on demand)
    state.store_word(0x1000, 0xDEADBEEF);
    state.store_word(0x100000, 0x12345678);
    state.store_word(0x80000000, 0xABCDEF00);
    
    // read back the values
    uint32_t val1 = state.load_word(0x1000);
    uint32_t val2 = state.load_word(0x100000);
    uint32_t val3 = state.load_word(0x80000000);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // verify values
    bool success = (val1 == 0xDEADBEEF) && (val2 == 0x12345678) && (val3 == 0xABCDEF00);
    
    auto init_time = std::chrono::duration_cast<std::chrono::microseconds>(after_init - start);
    auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Initialization time: " << init_time.count() << " microseconds\n";
    std::cout << "Total test time: " << total_time.count() << " microseconds\n";
    std::cout << "Memory test " << (success ? "PASSED" : "FAILED") << "\n";
    
    // test reading from uninitialized memory (should return 0!!!!)
    uint32_t uninit = state.load_word(0x50000000);
    std::cout << "Uninitialized memory read: 0x" << std::hex << uninit << std::dec;
    std::cout << (uninit == 0 ? " (CORRECT)" : " (ERROR)") << "\n";
    
    return success ? 0 : 1;
}
