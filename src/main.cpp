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
std::map<int, int> BinopPrecedence;

// --- GLOBAL DEFINITIONS ---
std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: quanta <file.qnt>" << std::endl;
        return 1;
    }


    BinopPrecedence['<'] = 10;
    BinopPrecedence['>'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;
    BinopPrecedence['/'] = 40;
    
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

    // 4. Parse
    // FIX: parse() now returns a single 'FunctionAST' pointer, NOT a vector.
    auto funcNode = parse(tokens); 

    // Check for Parser Errors
    if (HasError || !funcNode) {
        std::cerr << "[Summary] Compilation failed due to errors above." << std::endl;
        return 1;
    }

    // 5. Compile AST to IR
    // FIX: We just call codegen() on the function node.
    // It automatically creates the 'main' function and processes the body.
    if (!funcNode->codegen()) {
        std::cerr << "[ERROR] Code Generation failed." << std::endl;
        return 1;
    }

    // 6. Generate Object File
    generateObjectCode();

    // 7. Link and Auto-Run
    std::cout << "[INFO] Compiling object code..." << std::endl;
    int linkResult = system("clang output.o -o my_quanta_app");
    
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