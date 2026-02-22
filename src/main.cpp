#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// LLVM Headers
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

#include "../include/quanta.h"
std::string RootDir = "./";

// --- GLOBAL DEFINITIONS ---
std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: quanta <file.qnt>" << std::endl;
        return 1;
    }
    std::string filepath = argv[1];
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        RootDir = filepath.substr(0, lastSlash + 1);
    }
    


    BinopPrecedence['<'] = 10;
    BinopPrecedence['>'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;
    BinopPrecedence['/'] = 40;
    BinopPrecedence[TOK_GEQ] = 10;  // >=
    BinopPrecedence[TOK_LEQ] = 10;  // <=
    BinopPrecedence[TOK_NEQ] = 5;   // !=
    BinopPrecedence['%'] = 40;
    
    // [FIX] Add this line!
    BinopPrecedence[TOK_EQ] = 5;
   

    // 1. Read the Source File
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Initialize
    initializeModule();
    
    // 3. Tokenize
    auto tokens = tokenize(source);

    // std::cout << "\n[TRACE] 1. LEXER OUTPUT:" << std::endl;
    // std::cout << "--------------------------------" << std::endl;
    // for (const auto& t : tokens) {
    //     // Print Token Type and Value
    //     std::cout << "Token Type: " << t.type << " | Value: '" << t.value << "'" << std::endl;
    // }
    // std::cout << "--------------------------------\n" << std::endl;

    // 4. Parse
    // FIX: parse() now returns a single 'FunctionAST' pointer, NOT a vector.
    ProgramAST program = parse(tokens);

   

if (HasError) {
        std::cerr << "\n\033[1;31m[Fatal]\033[0m Parsing failed. Aborting." << std::endl;
        return 1;
    }
    

    // 5. Compile AST to IR
    bool foundMain = false;

    // Loop through every function (main, add, etc.)
    for (const auto& func : program.functions) {
        // Check if we found 'main'
        if (func->getName() == "main") {
            foundMain = true;
        }

        // Generate IR for this function
        if (!func->codegen()) {
            std::cerr << "[ERROR] Code Generation failed for function: " << func->getName() << std::endl;
            return 1;
        }
    }

    // Ensure a 'main' function exists (either explicit or auto-generated)
    if (!foundMain) {
        std::cerr << "Error: No 'main' function found! (And no top-level script code was valid)" << std::endl;
        return 1;
    }
    if (HasError) {
        std::cerr << "\n\033[1;31m[Fatal]\033[0m Compilation failed due to type errors. Object file was NOT created." << std::endl;
        return 1; // STOP HERE! Do not generate object code.
    }

    // 6. Generate Object File
    generateObjectCode();
    // TheModule->print(llvm::errs(), nullptr);
    
    // 7. Link and Auto-Run
    std::cout << "[INFO] Compiling object code..." << std::endl;
    // int linkResult = system("clang -g output.o -o my_quanta_app");
    int linkResult = system("clang -g output.o ../src/quanta_lib.c -o my_quanta_app");
    
    if (linkResult == 0) {
        std::cout << "SUCCESS! Running program..." << std::endl;
        std::cout << "------------------------------------" << std::endl;

        int exitCode = system("./my_quanta_app");
        int actualReturn = exitCode >> 8;
        
        std::cout << "\n------------------------------------" << std::endl;
        std::cout << "Program exited with code: " << actualReturn << std::endl;
    } else {
        std::cerr << "Linking Failed." << std::endl;
        return 1;
    }
   
    return 0;
}