#include "../include/quanta.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/IR/LegacyPassManager.h"
#include <map>
#include <iostream>
#include <vector>

// --- GLOBALS ---
// CORRECT: This tells the compiler "Look for these in main.cpp"
extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::map<std::string, llvm::AllocaInst*> NamedValues;

// --- 1. SETUP ---
void initializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("QuantaModule", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

// --- 2. NUMBER GENERATION ---
llvm::Value *NumberAST::codegen() {
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, Val, true));
}
// --- STRING GENERATION ---
llvm::Value *StringAST::codegen() {
    // CreateGlobalString generates the constant and returns a pointer to it
    // In newer LLVM, this is a 'ptr' type which is perfect for printf
    return Builder->CreateGlobalString(val, "strtmp");
}

// --- 3. VARIABLE GENERATION ---
llvm::Value *VariableAST::codegen() {
    llvm::Value *V = NamedValues[Name];
    if (!V) {
        std::cerr << "Unknown variable name: " << Name << std::endl;
        return nullptr;
    }
    return Builder->CreateLoad(llvm::Type::getInt32Ty(*TheContext), V, Name.c_str());
}

// --- 4. MATH & ASSIGNMENT ---
llvm::Value *BinaryExprAST::codegen() {
    // CASE A: Assignment (x = 10)
    if (Op == '=') {
        VariableAST *LHSE = dynamic_cast<VariableAST*>(LHS.get());
        if (!LHSE) {
            std::cerr << "Error: You can only assign to a variable!" << std::endl;
            return nullptr;
        }

        llvm::Value *Val = RHS->codegen();
        if (!Val) return nullptr;

        llvm::Value *Variable = NamedValues[LHSE->Name];
        if (!Variable) {
            llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
            llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
            Variable = TmpB.CreateAlloca(llvm::Type::getInt32Ty(*TheContext), nullptr, LHSE->Name);
            NamedValues[LHSE->Name] = static_cast<llvm::AllocaInst*>(Variable);
        }

        Builder->CreateStore(Val, Variable);
        return Val;
    }

    // CASE B: Math (+, -, *, /)
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R) return nullptr;

    switch (Op) {
        case '+': return Builder->CreateAdd(L, R, "addtmp");
        case '-': return Builder->CreateSub(L, R, "subtmp");
        case '*': return Builder->CreateMul(L, R, "multmp");
        case '/': return Builder->CreateSDiv(L, R, "divtmp"); 
        default: return nullptr;
    }
}

// --- 5. PRINT GENERATION (UPDATED FOR NEW LLVM) ---
// --- PRINT GENERATION ---
llvm::Value *PrintAST::codegen() {
    llvm::Value *Val = Expr->codegen();
    if (!Val) return nullptr;

    // A. Find/Declare printf
    llvm::Function *PrintfFunc = TheModule->getFunction("printf");
    if (!PrintfFunc) {
        std::vector<llvm::Type*> Args;
        Args.push_back(llvm::PointerType::getUnqual(*TheContext)); 
        llvm::FunctionType *PrintfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*TheContext), Args, true);
        PrintfFunc = llvm::Function::Create(PrintfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    }

    // B. Logic for Strings vs Numbers
    llvm::Value *FormatStr;
    std::vector<llvm::Value*> ArgsV;

    if (Val->getType()->isPointerTy()) {
        // BUG FIX: If it's a string literal (Constant), we must ensure it's a pointer to i8.
        // LLVM Global Strings are often [N x i8]. We need to point to index [0, 0].
        FormatStr = Builder->CreateGlobalString("%s\n", "fmt_s");
    } else {
        FormatStr = Builder->CreateGlobalString("%d\n", "fmt_d");
    }

    ArgsV.push_back(FormatStr);
    ArgsV.push_back(Val);

    return Builder->CreateCall(PrintfFunc, ArgsV, "printcall");
}

// --- 6. SAVE TO FILE ---
void generateObjectCode() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();

    std::string TargetTriple = llvm::sys::getDefaultTargetTriple();
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto TargetMachine = Target->createTargetMachine(llvm::Triple(TargetTriple), CPU, Features, opt, llvm::Reloc::PIC_);

    TheModule->setTargetTriple(llvm::Triple(TargetTriple));
    TheModule->setDataLayout(TargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest("output.o", EC, llvm::sys::fs::OF_None);

    llvm::legacy::PassManager pass;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "TheTargetMachine can't emit a file of this type" << std::endl;
        return;
    }

    pass.run(*TheModule);
    dest.flush();
    std::cout << "[Success] Native object file 'output.o' created!" << std::endl;
     
 
}