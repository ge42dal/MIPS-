#include "assembler.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>

namespace mips {

// directive implementation
uint32_t Directive::size() const {
    switch (type) {
        case DirectiveType::BYTE:
            return arguments.size();
        case DirectiveType::HALF:
            return arguments.size() * 2;
        case DirectiveType::WORD:
            return arguments.size() * 4;
        case DirectiveType::ASCII:
            return arguments.empty() ? 0 : arguments[0].length();
        case DirectiveType::ASCIIZ:
            return arguments.empty() ? 1 : arguments[0].length() + 1;
        case DirectiveType::SPACE:
            return arguments.empty() ? 0 : std::stoul(arguments[0]);
        default:
            return 0;
    }
}

// Assembler implementation
Assembler::Assembler() : main_address_(0) {
    init_instruction_tables();
}

void Assembler::init_instruction_tables() {
    // R-type (opcode = 0!!!)
    instruction_functions_["sll"] = 0b000000;
    instruction_functions_["srl"] = 0b000010;
    instruction_functions_["sra"] = 0b000011;
    instruction_functions_["sllv"] = 0b000100;
    instruction_functions_["srlv"] = 0b000110;
    instruction_functions_["srav"] = 0b000111;
    instruction_functions_["jr"] = 0b001000;
    instruction_functions_["jalr"] = 0b001001;
    instruction_functions_["mfhi"] = 0b010000;
    instruction_functions_["mthi"] = 0b010001;
    instruction_functions_["mflo"] = 0b010010;
    instruction_functions_["mtlo"] = 0b010011;
    instruction_functions_["mult"] = 0b011000;
    instruction_functions_["multu"] = 0b011001;
    instruction_functions_["div"] = 0b011010;
    instruction_functions_["divu"] = 0b011011;
    instruction_functions_["add"] = 0b100000;
    instruction_functions_["addu"] = 0b100001;
    instruction_functions_["sub"] = 0b100010;
    instruction_functions_["subu"] = 0b100011;
    instruction_functions_["and"] = 0b100100;
    instruction_functions_["or"] = 0b100101;
    instruction_functions_["xor"] = 0b100110;
    instruction_functions_["nor"] = 0b100111;
    instruction_functions_["slt"] = 0b101010;
    instruction_functions_["sltu"] = 0b101011;

    // I-type & J-type 
    instruction_opcodes_["beq"] = 0b000100;
    instruction_opcodes_["bne"] = 0b000101;
    instruction_opcodes_["blez"] = 0b000110;
    instruction_opcodes_["bgtz"] = 0b000111;
    instruction_opcodes_["addi"] = 0b001000;
    instruction_opcodes_["addiu"] = 0b001001;
    instruction_opcodes_["slti"] = 0b001010;
    instruction_opcodes_["sltiu"] = 0b001011;
    instruction_opcodes_["andi"] = 0b001100;
    instruction_opcodes_["ori"] = 0b001101;
    instruction_opcodes_["xori"] = 0b001110;
    instruction_opcodes_["llo"] = 0b011000;
    instruction_opcodes_["lhi"] = 0b011001;
    instruction_opcodes_["lb"] = 0b100000;
    instruction_opcodes_["lh"] = 0b100001;
    instruction_opcodes_["lw"] = 0b100011;
    instruction_opcodes_["lbu"] = 0b100100;
    instruction_opcodes_["lhu"] = 0b100101;
    instruction_opcodes_["sb"] = 0b101000;
    instruction_opcodes_["sh"] = 0b101001;
    instruction_opcodes_["sw"] = 0b101011;
    instruction_opcodes_["j"] = 0b000010;
    instruction_opcodes_["jal"] = 0b000011;
    instruction_opcodes_["trap"] = 0b011010;

    // Instruction categories
    instruction_categories_["add"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["addu"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["sub"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["subu"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["and"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["or"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["xor"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["nor"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["slt"] = InstructionCategory::ARITH_LOGIC;
    instruction_categories_["sltu"] = InstructionCategory::ARITH_LOGIC;
    
    instruction_categories_["mult"] = InstructionCategory::DIV_MULT;
    instruction_categories_["multu"] = InstructionCategory::DIV_MULT;
    instruction_categories_["div"] = InstructionCategory::DIV_MULT;
    instruction_categories_["divu"] = InstructionCategory::DIV_MULT;
    
    instruction_categories_["sll"] = InstructionCategory::SHIFT;
    instruction_categories_["srl"] = InstructionCategory::SHIFT;
    instruction_categories_["sra"] = InstructionCategory::SHIFT;
    
    instruction_categories_["sllv"] = InstructionCategory::SHIFT_REG;
    instruction_categories_["srlv"] = InstructionCategory::SHIFT_REG;
    instruction_categories_["srav"] = InstructionCategory::SHIFT_REG;
    
    instruction_categories_["jr"] = InstructionCategory::JUMP_REG;
    instruction_categories_["jalr"] = InstructionCategory::JUMP_REG;
    
    instruction_categories_["mfhi"] = InstructionCategory::MOVE_FROM;
    instruction_categories_["mflo"] = InstructionCategory::MOVE_FROM;
    
    instruction_categories_["mthi"] = InstructionCategory::MOVE_TO;
    instruction_categories_["mtlo"] = InstructionCategory::MOVE_TO;
    
    instruction_categories_["addi"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["addiu"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["slti"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["sltiu"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["andi"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["ori"] = InstructionCategory::ARITH_LOGIC_IMM;
    instruction_categories_["xori"] = InstructionCategory::ARITH_LOGIC_IMM;
    
    instruction_categories_["llo"] = InstructionCategory::LOAD_IMM;
    instruction_categories_["lhi"] = InstructionCategory::LOAD_IMM;
    
    instruction_categories_["beq"] = InstructionCategory::BRANCH;
    instruction_categories_["bne"] = InstructionCategory::BRANCH;
    
    instruction_categories_["blez"] = InstructionCategory::BRANCH_ZERO;
    instruction_categories_["bgtz"] = InstructionCategory::BRANCH_ZERO;
    
    instruction_categories_["lb"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["lh"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["lw"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["lbu"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["lhu"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["sb"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["sh"] = InstructionCategory::LOAD_STORE;
    instruction_categories_["sw"] = InstructionCategory::LOAD_STORE;
    
    instruction_categories_["j"] = InstructionCategory::JUMP;
    instruction_categories_["jal"] = InstructionCategory::JUMP;
    
    instruction_categories_["trap"] = InstructionCategory::TRAP;
}

std::vector<AssemblyLine> Assembler::parse_assembly(const std::string& assembly_text) {
    std::istringstream stream(assembly_text); //conver to stream
    return parse_assembly(stream); // create asm_lines from string
}

std::vector<AssemblyLine> Assembler::parse_assembly(std::istream& input) {
    std::vector<AssemblyLine> lines;
    std::string line;
    uint32_t line_number = 0;
    uint32_t current_address = 0;
    
    // first pass: parse lines and collect labels
    while (std::getline(input, line)) {
        line_number++;
        AssemblyLine asm_line = parse_line(line, line_number); // trims everything that is not \t, \n, \r + tokenize and store in correct asm line attribute
        
        // case label
        if (!asm_line.label.empty()) {
            labels_[asm_line.label] = current_address; // map label to adress
            if (asm_line.label == "main") {
                main_address_ = current_address;
            }
        }
        // case instruction
        if (!asm_line.instruction.empty()) {
            asm_line.address = current_address;
            if (asm_line.is_directive) { //check if dir
                // calc dir size
                if (asm_line.instruction == ".byte") {
                    asm_line.size = asm_line.operands.size();
                } else if (asm_line.instruction == ".half") {
                    asm_line.size = asm_line.operands.size() * 2;
                } else if (asm_line.instruction == ".word") {
                    asm_line.size = asm_line.operands.size() * 4;
                } else if (asm_line.instruction == ".ascii") {
                    asm_line.size = asm_line.operands.empty() ? 0 : asm_line.operands[0].length() - 2; // Remove quotes
                } else if (asm_line.instruction == ".asciiz") {
                    asm_line.size = asm_line.operands.empty() ? 1 : asm_line.operands[0].length() - 1; // Remove quotes, add null
                } else if (asm_line.instruction == ".space") {
                    asm_line.size = asm_line.operands.empty() ? 0 : std::stoul(asm_line.operands[0]);
                }
            } else {
                asm_line.size = 4; // all instructions are 4 bytes
            }
            current_address += asm_line.size; // move address forward
            lines.push_back(asm_line);
        }
    }
    
    return lines;
}

AssemblyLine Assembler::parse_line(const std::string& line, uint32_t /* line_number */) {
    AssemblyLine asm_line;
    asm_line.address = 0;
    asm_line.size = 0;
    asm_line.is_directive = false;
    
    std::string trimmed = trim(line); 
    
    // skip empty lines and comments
    if (trimmed.empty() || trimmed[0] == '#') {
        return asm_line;
    }
    
    std::vector<std::string> tokens = tokenize(trimmed);
    if (tokens.empty()) {
        return asm_line;
    }
    
    size_t token_index = 0;
    
    // label
    if (tokens[token_index].back() == ':') {
        asm_line.label = tokens[token_index].substr(0, tokens[token_index].length() - 1);
        token_index++;
    }
    
    // check for inst/dir
    if (token_index < tokens.size()) {
        asm_line.instruction = tokens[token_index];
        asm_line.is_directive = (asm_line.instruction[0] == '.');
        token_index++;
        
        // collect operands
        while (token_index < tokens.size()) {
            asm_line.operands.push_back(tokens[token_index]);
            token_index++;
        }
    }
    
    return asm_line;
}

std::vector<std::string> Assembler::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    
    while (stream >> token) {
        // handle commas
        if (token.back() == ',') {
            token.pop_back();
        }
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string Assembler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool Assembler::is_register(const std::string& token) {
    return token[0] == '$';
}

bool Assembler::is_immediate(const std::string& token) {
    return std::isdigit(token[0]) || token[0] == '-' || token[0] == '+';
}

bool Assembler::is_label(const std::string& token) {
    return !is_register(token) && !is_immediate(token) && token.find('(') == std::string::npos;
}

std::vector<uint8_t> Assembler::assemble(const std::vector<AssemblyLine>& lines) {
    std::vector<uint8_t> binary_data;
    errors_.clear();
    
    for (const auto& line : lines) {
        if (line.is_directive) {
            std::vector<uint8_t> directive_data = assemble_directive(line);
            binary_data.insert(binary_data.end(), directive_data.begin(), directive_data.end());
        } else {
            Instruction instr = assemble_instruction(line); // transfer data to instr fields
            uint32_t encoded = instr.encode(); // convert to 32b
            
            // store as little-endian, shiift right, then mask 8b
            binary_data.push_back(encoded & 0xFF); // 0-7
            binary_data.push_back((encoded >> 8) & 0xFF); // 8-15
            binary_data.push_back((encoded >> 16) & 0xFF); // 16-23    
            binary_data.push_back((encoded >> 24) & 0xFF); // 24-31
        }
    }
    
    return binary_data;
}

std::vector<uint8_t> Assembler::assemble_text(const std::string& assembly_text) {
    auto lines = parse_assembly(assembly_text);
    return assemble(lines);
}

std::vector<uint8_t> Assembler::assemble_stream(std::istream& input) {
    auto lines = parse_assembly(input);
    return assemble(lines);
}

void Assembler::add_error(const std::string& error, uint32_t line_number) {
    std::string full_error = "Line " + std::to_string(line_number) + ": " + error;
    errors_.push_back(full_error);
}

// Binary format implementation
void BinaryFormat::write_binary(const std::vector<uint8_t>& data, std::ostream& output, uint32_t main_address) {
    // write main address first (4 bytes, little-endian)
    output.put(main_address & 0xFF);
    output.put((main_address >> 8) & 0xFF);
    output.put((main_address >> 16) & 0xFF);
    output.put((main_address >> 24) & 0xFF);
    
    // write data size (4 bytes, little-endian)
    uint32_t size = data.size();
    output.put(size & 0xFF);
    output.put((size >> 8) & 0xFF);
    output.put((size >> 16) & 0xFF);
    output.put((size >> 24) & 0xFF);
    
    // write data
    output.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void BinaryFormat::write_binary_file(const std::vector<uint8_t>& data, const std::string& filename, uint32_t main_address) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    write_binary(data, file, main_address);
}

std::vector<uint8_t> BinaryFormat::read_binary(std::istream& input, uint32_t& main_address) {
    // read main address (4 bytes, little-endian)
    uint8_t bytes[4];
    input.read(reinterpret_cast<char*>(bytes), 4);
    main_address = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    
    // read data size (4 bytes, little-endian)
    input.read(reinterpret_cast<char*>(bytes), 4);
    uint32_t size = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    
    // read data
    std::vector<uint8_t> data(size);
    input.read(reinterpret_cast<char*>(data.data()), size);
    
    return data;
}

std::vector<uint8_t> BinaryFormat::read_binary_file(const std::string& filename, uint32_t& main_address) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + filename);
    }
    return read_binary(file, main_address);
}

// instruction assembly implementation
Instruction Assembler::assemble_instruction(const AssemblyLine& line) {
    Instruction instr;
    instr.name = line.instruction; // add name
    
    // find instruction category
    auto cat_it = instruction_categories_.find(line.instruction);
    if (cat_it == instruction_categories_.end()) { // not found
        add_error("Unknown instruction: " + line.instruction);
        return instr;
    }
    instr.category = cat_it->second; // add category
    
    // case r type
    if (instruction_functions_.find(line.instruction) != instruction_functions_.end()) {
        instr.opcode = 0;
        instr.function = instruction_functions_[line.instruction];
        instr.type = InstructionType::R_TYPE;
        
        switch (instr.category) {
            case InstructionCategory::ARITH_LOGIC:
                if (line.operands.size() != 3) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rd = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rs = static_cast<uint8_t>(string_to_register(line.operands[1]));
                instr.rt = static_cast<uint8_t>(string_to_register(line.operands[2]));
                instr.shamt = 0;
                break;
                
            case InstructionCategory::SHIFT:
                if (line.operands.size() != 3) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rd = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rt = static_cast<uint8_t>(string_to_register(line.operands[1]));
                instr.shamt = std::stoul(line.operands[2]);
                instr.rs = 0;
                break;
                
            case InstructionCategory::SHIFT_REG:
                if (line.operands.size() != 3) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rd = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rt = static_cast<uint8_t>(string_to_register(line.operands[1]));
                instr.rs = static_cast<uint8_t>(string_to_register(line.operands[2]));
                instr.shamt = 0;
                break;
                
            case InstructionCategory::DIV_MULT:
                if (line.operands.size() != 2) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rs = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rt = static_cast<uint8_t>(string_to_register(line.operands[1]));
                instr.rd = 0;
                instr.shamt = 0;
                break;
                
            case InstructionCategory::JUMP_REG:
                if (line.operands.size() != 1) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rs = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rt = 0;
                instr.rd = 0;
                instr.shamt = 0;
                break;
                
            case InstructionCategory::MOVE_FROM:
                if (line.operands.size() != 1) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rd = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rs = 0;
                instr.rt = 0;
                instr.shamt = 0;
                break;
                
            case InstructionCategory::MOVE_TO:
                if (line.operands.size() != 1) {
                    add_error("Invalid operand count for " + line.instruction);
                    break;
                }
                instr.rs = static_cast<uint8_t>(string_to_register(line.operands[0]));
                instr.rt = 0;
                instr.rd = 0;
                instr.shamt = 0;
                break;
            default:
                add_error("Unhandled R-type instruction category for " + line.instruction);
                break;
        }
    }
    // case j or i type
    else if (instruction_opcodes_.find(line.instruction) != instruction_opcodes_.end()) {
        instr.opcode = instruction_opcodes_[line.instruction]; // 1. opcode to binary
        
        if (instr.category == InstructionCategory::JUMP) {
            instr.type = InstructionType::J_TYPE;
            if (line.operands.size() != 1) {
                add_error("Invalid operand count for " + line.instruction);
            } else {
                instr.address = resolve_address(line.operands[0], line.address) >> 2;
            }
        } else {
            instr.type = InstructionType::I_TYPE;
            
            switch (instr.category) { // convert to register based on category + resolve immediate
                case InstructionCategory::ARITH_LOGIC_IMM:
                    if (line.operands.size() != 3) { // arithmetic type = 3
                        add_error("Invalid operand count for " + line.instruction); 
                        break;
                    }
                    // 2. map to registers to enum (binary)
                    instr.rt = static_cast<uint8_t>(string_to_register(line.operands[0]));
                    instr.rs = static_cast<uint8_t>(string_to_register(line.operands[1]));
                    instr.immediate = resolve_immediate(line.operands[2], line.address);
                    break;
                    
                case InstructionCategory::LOAD_IMM:
                    if (line.operands.size() != 2) { // pseudo inst. = 2
                        add_error("Invalid operand count for " + line.instruction);
                        break;
                    }
                    instr.rt = static_cast<uint8_t>(string_to_register(line.operands[0]));
                    instr.rs = 0;
                    instr.immediate = resolve_immediate(line.operands[1], line.address);
                    break;
                    
                case InstructionCategory::BRANCH:
                    if (line.operands.size() != 3) { // branch = 3
                        add_error("Invalid operand count for " + line.instruction);
                        break;
                    }
                    instr.rs = static_cast<uint8_t>(string_to_register(line.operands[0]));
                    instr.rt = static_cast<uint8_t>(string_to_register(line.operands[1]));
                    instr.immediate = (resolve_address(line.operands[2], line.address) - line.address - 4) >> 2;
                    break;
                    
                case InstructionCategory::BRANCH_ZERO: // branch zero = 2
                    if (line.operands.size() != 2) {
                        add_error("Invalid operand count for " + line.instruction);
                        break;
                    }
                    instr.rs = static_cast<uint8_t>(string_to_register(line.operands[0]));
                    instr.rt = 0;
                    instr.immediate = (resolve_address(line.operands[1], line.address) - line.address - 4) >> 2;
                    break;
                    
                case InstructionCategory::LOAD_STORE: { // load store = 2
                    if (line.operands.size() != 2) {
                        add_error("Invalid operand count for " + line.instruction);
                        break;
                    }
                    instr.rt = static_cast<uint8_t>(string_to_register(line.operands[0]));
                    
                    // parse offset(register) format
                    std::string mem_operand = line.operands[1];
                    size_t paren_pos = mem_operand.find('(');
                    if (paren_pos != std::string::npos) {
                        std::string offset_str = mem_operand.substr(0, paren_pos);
                        std::string reg_str = mem_operand.substr(paren_pos + 1);
                        reg_str.pop_back(); // remove closing parenthesis
                        
                        instr.immediate = resolve_immediate(offset_str, line.address);
                        instr.rs = static_cast<uint8_t>(string_to_register(reg_str));
                    } else {
                        add_error("Invalid memory operand format: " + mem_operand);
                    }
                    break;
                }
                    
                case InstructionCategory::TRAP:
                    if (line.operands.size() != 1) { // trap = 1
                        add_error("Invalid operand count for " + line.instruction);
                        break;
                    }
                    instr.rs = 0;
                    instr.rt = 0;
                    instr.immediate = resolve_immediate(line.operands[0], line.address);
                    break;
                default:
                    add_error("Unhandled I-type instruction category for " + line.instruction);
                    break;
            }
        }
    }
    
    return instr;
}

std::vector<uint8_t> Assembler::assemble_directive(const AssemblyLine& line) {
    std::vector<uint8_t> data;
    
    if (line.instruction == ".byte") {
        for (const auto& operand : line.operands) {
            uint8_t value = static_cast<uint8_t>(resolve_immediate(operand, line.address));
            data.push_back(value);
        }
    } else if (line.instruction == ".half") {
        for (const auto& operand : line.operands) {
            uint16_t value = static_cast<uint16_t>(resolve_immediate(operand, line.address));
            data.push_back(value & 0xFF);
            data.push_back((value >> 8) & 0xFF);
        }
    } else if (line.instruction == ".word") {
        for (const auto& operand : line.operands) {
            uint32_t value = resolve_immediate(operand, line.address);
            data.push_back(value & 0xFF);
            data.push_back((value >> 8) & 0xFF);
            data.push_back((value >> 16) & 0xFF);
            data.push_back((value >> 24) & 0xFF);
        }
    } else if (line.instruction == ".ascii") {
        if (!line.operands.empty()) {
            std::string str = line.operands[0];
            // Remove quotes
            if (str.length() >= 2 && str[0] == '"' && str.back() == '"') {
                str = str.substr(1, str.length() - 2);
            }
            for (char c : str) {
                data.push_back(static_cast<uint8_t>(c));
            }
        }
    } else if (line.instruction == ".asciiz") {
        if (!line.operands.empty()) {
            std::string str = line.operands[0];
            // Remove quotes
            if (str.length() >= 2 && str[0] == '"' && str.back() == '"') {
                str = str.substr(1, str.length() - 2);
            }
            for (char c : str) {
                data.push_back(static_cast<uint8_t>(c));
            }
            data.push_back(0); // Null terminator
        } else {
            data.push_back(0); // Just null terminator
        }
    } else if (line.instruction == ".space") {
        uint32_t size = line.operands.empty() ? 0 : resolve_immediate(line.operands[0], line.address);
        data.resize(size, 0);
    }
    
    return data;
}

uint32_t Assembler::resolve_immediate(const std::string& imm_str, uint32_t /* current_address */) {
    if (imm_str.empty()) return 0;
    // label OR number OR invalid
    
    // Check if it's a label
    auto label_it = labels_.find(imm_str);
    if (label_it != labels_.end()) {
        return label_it->second;
    }
    
    // Parse as number
    try {
        if (imm_str.substr(0, 2) == "0x" || imm_str.substr(0, 2) == "0X") {
            return std::stoul(imm_str, nullptr, 16);
        } else {
            return std::stoul(imm_str);
        }
    } catch (const std::exception&) {
        add_error("Invalid immediate value: " + imm_str);
        return 0;
    }
}

uint32_t Assembler::resolve_address(const std::string& label, uint32_t /* current_address */) {
    auto label_it = labels_.find(label);
    if (label_it != labels_.end()) {
        return label_it->second;
    }
    
    add_error("Undefined label: " + label);
    return 0;
}

} // namespace mips
