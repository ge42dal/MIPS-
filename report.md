# MIPS interpreter report

The MIPS interpreter employs a modular, object-oriented design to simulate the behavior of a MIPS processor, which is centered around the library core (`mips_core`). The core library provides basic functionality for the CPU, machine state (memory and registers) for instruction execution and assembly. We separate concerns through various classes:

**Assembler** handles the conversion of MIPS assembly code into binary instructions.

**CPU** for instruction execution and accessing the Machine State

**MachineState** for system state management

**Debugger** for interactive debugging.

*Design Decisions*

**Memory Management:** Althought the exercise states to choose a static amount of memory, I opted for a more dynamic page-based memory management (4KB pages) system with lazy allocation supporting up to 4GB of memory (as per the theoreticaL MIPS address space). The Machine state has a `memory_pages_` unordered map, where each page is a unique pointer to an array of bytes `(std::array<uint8_t, PAGE_SIZE>)`. We access the current page via `get_page_index` and `get_page_offset`, and create new pages on demand via `get_or_create_page`. 

**Instruction Representation:** The `Instruction` struct represents a MIPS instruction and contains fields for the opcode, register numbers, immediate values, function code, shift amount and variant fields (for R-, J-, I-type instructions) which allow the intstructions to be encoded and decoded in a polymorphic manner. The key feature allowing this polymorphis are the `InstructionType` and `InstructionCategory` enums. The encoding and decoding occurs thorugh the `encode` and `decode` methods by shifting the fields to the correct position and masking the unused bits. 

**Error Handling:** To avoid memory leaks, we use smart pointers (unique_ptr) to manage the pages (`std::unordered_map<uint32_t, std::unique_ptr<std::array<uint8_t, PAGE_SIZE>>> memory_pages_;`). These are automatically deallocated when going out od scope and if exception occurs, the destructor is called and the memory is freed (no manual delete or new). We also use RAII when creating a new page, where ownership is transferred to the unordered map `memory_pages_` and the unique pointer is automatically deallocated when the page is removed from the map. We also distinguish critical failures from recoverable errors by using exceptions and status flags such as `throw std::out_of_range("Memory address out of bounds")` in memory operations or `Assembler::has_errors() and get_errors()` in the assembler.

For modern C++ we use STL containers such as `std::vector` to store binary data and `std::unordered_map` to store the pages of memory and symbol tables. We also use Enum classes for registers and opcodes to make the code more readable and prevent confusion. We also use `constexpr`for fields of the machine state such as for MEMORY_SIZE, NUM_REGISTER and PAGE_SIZE to ensure they are evaluated at compile time for performance optimization. 


