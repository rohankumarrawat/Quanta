#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// LLVM Headers
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"

#include "../include/quanta.h"

// --- GLOBAL DEFINITIONS ---
// We define them here (NO 'extern') so the memory is actually allocated.
// codegen.cpp will access these using 'extern'.
std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: quanta <file.qnt>" << std::endl;
        return 1;
    }

    // 1. Read the Source File
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Initialize and Parse
    initializeModule();
    
    auto tokens = tokenize(source);
//     std::cout << "DEBUG: Tokens found: ";
// for(auto &t : tokens) std::cout << "[" << t.value << "] ";
// std::cout << std::endl;

    // Assuming parse returns a vector of statements (based on your loop below)
    auto statements = parse(tokens); 
    

    // 3. Create the implicit 'main' function
    // This allows users to write loose code like "x = 10" without wrapping it in a function.
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), false);
    llvm::Function *MainFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());
    
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", MainFunc);
    Builder->SetInsertPoint(BB);

    // 4. Compile AST to IR
    for (const auto& stmt : statements) {
        if (stmt) stmt->codegen();
    }

    // Always return 0 by default if the user script doesn't
    Builder->CreateRet(llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0)));
    llvm::verifyFunction(*MainFunc);

    // 5. Generate Object File
    generateObjectCode();

    // 6. Link and Auto-Run
    
    // int linkResult = system("clang output.o -o my_quanta_app -Wno-override-module");
    // Update this line in main.cpp
int linkResult = system("clang output.o -o my_quanta_app");
    
    if (linkResult == 0) {
        std::cout << "SUCCESS! Running program..." << std::endl;
        std::cout << "------------------------------------" << std::endl;

        // Run the executable immediately
        int exitCode = system("./my_quanta_app");
        
        // Convert raw system exit code to actual return value
        int actualReturn = exitCode >> 8;

        std::cout << "\n------------------------------------" << std::endl;
        std::cout << "Program exited with code: " << actualReturn << std::endl;
    } else {
        std::cerr << "Linking Failed." << std::endl;
        return 1;
    }

    return 0;
}