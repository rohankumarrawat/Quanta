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
// In src/codegen.cpp


llvm::Value *LogErrorV(const char *Str) {
    std::cerr << "[Codegen Error] " << Str << std::endl;
    return nullptr;
}

// --- GLOBALS ---
extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;

// --- KEEP THIS ONCE ---
struct VarInfo {
    llvm::Value *Alloca;  // Can be an AllocaInst (stack) or CallInst (malloc/heap)
    llvm::Type *Type;
    std::string TypeName;
    llvm::Type *ElementType; // For Arrays and Lists
};

// Global Variable Registry
static std::map<std::string, VarInfo> NamedValues;
static std::map<std::string, llvm::Value*> StringPool;
// static std::vector<llvm::Value*> AutoFreeList;

// --- AUTO-FREE MEMORY TRACKER ---
static std::map<llvm::Function*, std::vector<llvm::AllocaInst*>> AutoFreeMap;
// --- EXCEPTION SYSTEM GLOBALS ---
#include <stack>
static std::stack<llvm::BasicBlock*> CatchStack;
static llvm::GlobalVariable* QuantaErrorMsg = nullptr;
static llvm::GlobalVariable* RecursionDepth = nullptr;
void trackForAutoFree(llvm::Value *HeapPtr) {
    // 1. Go to the very top of the function (Entry Block)
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    
    // 2. Create a safe pointer variable on the stack
    llvm::AllocaInst *Tracker = TmpB.CreateAlloca(Builder->getPtrTy(), nullptr, "free_tracker");
    
    // 3. Initialize it to NULL (so free(NULL) does nothing if never used)
    TmpB.CreateStore(llvm::ConstantPointerNull::get(Builder->getPtrTy()), Tracker);
    
    // 4. Store the actual heap pointer into it where it was created
    Builder->CreateStore(HeapPtr, Tracker);
    
    // 5. Track the stack variable instead of the raw instruction
   AutoFreeMap[TheFunction].push_back(Tracker);
}

// Global Function Registry
// std::map<std::string, FunctionInfo> FunctionRegistry;
extern std::map<std::string, FunctionInfo> FunctionRegistry;

// --- EXCEPTION SYSTEM HELPERS ---
llvm::Function* getQuantaPanicFunc() {
    llvm::Function *PanicFn = TheModule->getFunction("quanta_panic");
    if (!PanicFn) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getVoidTy(), {Builder->getPtrTy()}, false);
        PanicFn = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "quanta_panic", TheModule.get());
    }
    return PanicFn;
}

// Ensure getPooledString is defined above this so we can call it.
// Forward Declaration:
llvm::Value* getPooledString(const std::string& str, const std::string& label);

void triggerError(const std::string& errorMsg) {
    llvm::Value* ErrStr = getPooledString(errorMsg, "err_str");
    
    if (!QuantaErrorMsg) {
        QuantaErrorMsg = new llvm::GlobalVariable(
            *TheModule, Builder->getPtrTy(), false, 
            llvm::GlobalValue::InternalLinkage, 
            llvm::ConstantPointerNull::get(Builder->getPtrTy()), 
            "_quanta_error_msg"
        );
    }
    Builder->CreateStore(ErrStr, QuantaErrorMsg);

    if (CatchStack.empty()) {
        Builder->CreateCall(getQuantaPanicFunc(), {ErrStr});
        Builder->CreateUnreachable(); // Terminate LLVM basic block
    } else {
        llvm::BasicBlock* CatchBB = CatchStack.top();
        Builder->CreateBr(CatchBB);
    }
}
// ----------------------
llvm::Value* getPooledString(const std::string& str, const std::string& label = "str_pool") {
    if (StringPool.find(str) == StringPool.end()) {
        StringPool[str] = Builder->CreateGlobalString(str, label);
    }
    return StringPool[str];
}
llvm::Function* getQuantaUpperFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_upper");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_upper", TheModule.get());
    return F;
}

llvm::Function* getQuantaLowerFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_lower");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_lower", TheModule.get());
    return F;
}

llvm::Function* getQuantaReverseFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_reverse");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_reverse", TheModule.get());
    return F;
}

llvm::Function* getQuantaIsUpperFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_isupper");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_isupper", TheModule.get());
    return F;
}

llvm::Function* getQuantaIsLowerFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_islower");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_islower", TheModule.get());
    return F;
}

llvm::Function* getQuantaStripFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_strip");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_strip", TheModule.get());
    return F;
}
llvm::Function* getQuantaLstripFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_lstrip");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_lstrip", TheModule.get());
    return F;
}
llvm::Function* getQuantaRstripFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_rstrip");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_rstrip", TheModule.get());
    return F;
}
llvm::Function* getQuantaCapitalizeFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_capitalize");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_capitalize", TheModule.get());
    return F;
}
llvm::Function* getQuantaTitleFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_title");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_title", TheModule.get());
    return F;
}
llvm::Function* getQuantaIsAlphaFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_isalpha");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_isalpha", TheModule.get());
    return F;
}
llvm::Function* getQuantaIsDigitFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_isdigit");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_isdigit", TheModule.get());
    return F;
}
llvm::Function* getQuantaIsSpaceFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_isspace");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_isspace", TheModule.get());
    return F;
}
llvm::Function* getQuantaIsAlnumFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_isalnum");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_isalnum", TheModule.get());
    return F;
}
llvm::Function* getQuantaFindFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_find");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy(), Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_find", TheModule.get());
    return F;
}
llvm::Function* getQuantaCountFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_count");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy(), Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_count", TheModule.get());
    return F;
}
llvm::Function* getQuantaStartswithFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_startswith");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy(), Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_startswith", TheModule.get());
    return F;
}
llvm::Function* getQuantaEndswithFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_endswith");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy(), Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_endswith", TheModule.get());
    return F;
}
llvm::Function* getQuantaReplaceFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_replace");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy(), Builder->getPtrTy(), Builder->getPtrTy()}, false), llvm::Function::ExternalLinkage, "quanta_replace", TheModule.get());
    return F;
}

llvm::Function* getQuantaSliceFunc() {
    llvm::Function *F = TheModule->getFunction("quanta_slice");
    if (!F) F = llvm::Function::Create(llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy(), Builder->getInt32Ty(), Builder->getInt32Ty(), Builder->getInt32Ty()}, false), llvm::Function::ExternalLinkage, "quanta_slice", TheModule.get());
    return F;
}

// --- C STANDARD LIBRARY HELPERS ---

llvm::Function* getMallocFunc() {
    llvm::Function *F = TheModule->getFunction("malloc");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getInt64Ty()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "malloc", TheModule.get());
    }
    return F;
}

llvm::Function* getFreeFunc() {
    llvm::Function *F = TheModule->getFunction("free");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getVoidTy(), {Builder->getPtrTy()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "free", TheModule.get());
    }
    return F;
}

llvm::Function* getStrlenFunc() {
    llvm::Function *F = TheModule->getFunction("strlen");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getInt64Ty(), {Builder->getPtrTy()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "strlen", TheModule.get());
    }
    return F;
}

llvm::Function* getStrcpyFunc() {
    llvm::Function *F = TheModule->getFunction("strcpy");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy(), Builder->getPtrTy()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "strcpy", TheModule.get());
    }
    return F;
}

llvm::Function* getStrcatFunc() {
    llvm::Function *F = TheModule->getFunction("strcat");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getPtrTy(), {Builder->getPtrTy(), Builder->getPtrTy()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "strcat", TheModule.get());
    }
    return F;
}

llvm::Function* getStrcmpFunc() {
    llvm::Function *F = TheModule->getFunction("strcmp");
    if (!F) {
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getInt32Ty(), {Builder->getPtrTy(), Builder->getPtrTy()}, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "strcmp", TheModule.get());
    }
    return F;
}


// --- 1. SETUP ---
void initializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("QuantaModule", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
    NamedValues.clear(); 
    StringPool.clear(); 
}



// --- 2. PRIMITIVE VALUES ---
llvm::Value *NumberAST::codegen() {
   
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(64, Val, true));
    
}
// llvm::Value *ReturnAST::codegen() {
//     llvm::Value *RetVal = nullptr;
    

//     if (Expr) {
//         RetVal = Expr->codegen();
//         if (!RetVal) return nullptr; 
        

//     }

//     // 2. Generate the 'ret' instruction
//     if (RetVal) {
//         return Builder->CreateRet(RetVal);
//     } else {
//         return Builder->CreateRetVoid();
//     }
// }

llvm::Function *FunctionAST::codegen() {
    // 1. Prepare Argument Types for LLVM
    std::vector<llvm::Type*> ArgsTypes;
    
    for (const auto &Arg : Args) {
        if (Arg.Type == "float") {
            ArgsTypes.push_back(Builder->getFloatTy());
        } else if (Arg.Type == "double") {
            ArgsTypes.push_back(Builder->getDoubleTy());
        } else if (Arg.Type == "string") {
            ArgsTypes.push_back(Builder->getPtrTy());
        } else if (Arg.Type == "int8") {
            ArgsTypes.push_back(Builder->getInt8Ty());
        } else {
            // Default to 32-bit int for "int" or unknown types
            ArgsTypes.push_back(Builder->getInt32Ty());
        }
    }

    // 2. Define the Return Type
    llvm::Type* RetTy;
    if (ReturnType == "string") {
        RetTy = Builder->getPtrTy();
    } else if (ReturnType == "void") {
        RetTy = Builder->getVoidTy();
    } else if (ReturnType == "float") {
        RetTy = Builder->getFloatTy();
    } else if (ReturnType == "double") {
        RetTy = Builder->getDoubleTy();
    } else if (ReturnType.size() > 2 && ReturnType.compare(ReturnType.size() - 2, 2, "[]") == 0) {
        // Array return type (int[], string[], bool[], float[]) → return a ptr to the list struct
        RetTy = Builder->getPtrTy();
    } else if (ReturnType.find("int") == 0) {
        int bits = 32;
        if (ReturnType.size() > 3) {
            bits = std::atoi(ReturnType.substr(3).c_str()); 
        }
        RetTy = llvm::IntegerType::get(*TheContext, bits);
    } else {
        RetTy = llvm::Type::getInt32Ty(*TheContext);
    }

    // 3. Create the Function Type (Now includes ArgsTypes!)
    llvm::FunctionType *FT = llvm::FunctionType::get(RetTy, ArgsTypes, false);
    
    // 4. Create the Function
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());
    
    // 5. Create Entry Block
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    // 6. --- PROCESS ARGUMENTS (NEW) ---
    NamedValues.clear(); // Clear local variables from previous function
    unsigned Idx = 0;
    
   // Inside FunctionAST::codegen loop...
    for (auto &Arg : F->args()) {
        std::string argName = Args[Idx].Name;
        std::string argTypeStr = Args[Idx].Type; // "int", "float", etc.
        Arg.setName(argName);

        // Create Stack Memory
        llvm::AllocaInst *Alloca = Builder->CreateAlloca(Arg.getType(), nullptr, argName);
        Builder->CreateStore(&Arg, Alloca);

        // --- FIX: Store Alloca + LLVM Type + String Name ---
        NamedValues[argName] = {Alloca, Arg.getType(), argTypeStr};

        Idx++;
    }

    // --- Smart Guard: Recursion Depth (Push) ---
    if (!RecursionDepth) {
        RecursionDepth = new llvm::GlobalVariable(
            *TheModule, Builder->getInt32Ty(), false, 
            llvm::GlobalValue::InternalLinkage, 
            llvm::ConstantInt::get(Builder->getInt32Ty(), 0), 
            "_quanta_recursion_depth"
        );
    }
    llvm::Value *CurDepth = Builder->CreateLoad(Builder->getInt32Ty(), RecursionDepth, "cur_depth");
    llvm::Value *NextDepth = Builder->CreateAdd(CurDepth, llvm::ConstantInt::get(Builder->getInt32Ty(), 1), "next_depth");
    Builder->CreateStore(NextDepth, RecursionDepth);

    llvm::BasicBlock *StackErrBB = llvm::BasicBlock::Create(*TheContext, "stack_err", F);
    llvm::BasicBlock *StackOkBB = llvm::BasicBlock::Create(*TheContext, "stack_ok");

    llvm::Value *IsTooDeep = Builder->CreateICmpSGT(NextDepth, llvm::ConstantInt::get(Builder->getInt32Ty(), 1000));
    Builder->CreateCondBr(IsTooDeep, StackErrBB, StackOkBB);

    Builder->SetInsertPoint(StackErrBB);
    triggerError("Stack Error: Maximum recursion depth exceeded");

    F->insert(F->end(), StackOkBB);
    Builder->SetInsertPoint(StackOkBB);
AutoFreeMap.clear();
    // 7. Generate Body
    for (auto &node : Body) {
        node->codegen();
    }

    // 8. Handle Missing Return (Control Flow Falloff)
    if (!Builder->GetInsertBlock()->getTerminator()) {
      // 1. Get the current function
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    
    // 2. Loop through the tracked variables for THIS function only
    for (auto *Tracker : AutoFreeMap[TheFunction]) {
        llvm::Value *PtrToFree = Builder->CreateLoad(Builder->getPtrTy(), Tracker);
        Builder->CreateCall(getFreeFunc(), {PtrToFree});
    }

        // --- Smart Guard: Recursion Depth (Pop for Implicit Return) ---
        if (RecursionDepth) {
            llvm::Value *CurDepthRet = Builder->CreateLoad(Builder->getInt32Ty(), RecursionDepth);
            llvm::Value *PrevDepth = Builder->CreateSub(CurDepthRet, llvm::ConstantInt::get(Builder->getInt32Ty(), 1));
            Builder->CreateStore(PrevDepth, RecursionDepth);
        }

        if (RetTy->isVoidTy()) {
            Builder->CreateRetVoid();
        } else {
            Builder->CreateRet(llvm::Constant::getNullValue(RetTy));
        }
    }

    // 9. Verify
    if (llvm::verifyFunction(*F, &llvm::errs())) {
        // Optional: fprintf(stderr, "[Warning] Function verification failed: %s\n", Name.c_str());
    }
    
    return F;
}


llvm::Value *ReturnAST::codegen() {
    // 1. Generate the value being returned
    llvm::Value *RetVal = Expr->codegen();
    
    if (!RetVal) return nullptr;

    // 2. Identify types
    llvm::Function *ParentFunc = Builder->GetInsertBlock()->getParent();
    llvm::Type *ExpectedTy = ParentFunc->getReturnType();
    llvm::Type *ActualTy = RetVal->getType();

    llvm::Value *FinalRetVal = nullptr;

    // --- Smart Guard: Recursion Depth (Pop for Explicit Return) ---
    if (RecursionDepth) {
        llvm::Value *CurDepthRet = Builder->CreateLoad(Builder->getInt32Ty(), RecursionDepth);
        llvm::Value *PrevDepth = Builder->CreateSub(CurDepthRet, llvm::ConstantInt::get(Builder->getInt32Ty(), 1));
        Builder->CreateStore(PrevDepth, RecursionDepth);
    }

    // --- CONVERSION LOGIC ---

    // Case 1: Exact Match
    if (ActualTy == ExpectedTy) {
        FinalRetVal = RetVal;
    } 
    // Case 2: Function expects VOID
    else if (ExpectedTy->isVoidTy()) {
        FinalRetVal = nullptr; 
    } 
    // Case 3: Integer to Integer (Truncation or Extension)
    else if (ActualTy->isIntegerTy() && ExpectedTy->isIntegerTy()) {
        FinalRetVal = Builder->CreateIntCast(RetVal, ExpectedTy, true, "ret_cast");
    } 
    // Case 4: Float to Float
    else if (ActualTy->isFloatingPointTy() && ExpectedTy->isFloatingPointTy()) {
        FinalRetVal = Builder->CreateFPCast(RetVal, ExpectedTy, "ret_fp_resize");
    } 
    // Case 5: Float to Integer
    else if (ActualTy->isFloatingPointTy() && ExpectedTy->isIntegerTy()) {
        FinalRetVal = Builder->CreateFPToSI(RetVal, ExpectedTy, "ret_fp2int");
    } 
    // Case 6: Integer to Float
    else if (ActualTy->isIntegerTy() && ExpectedTy->isFloatingPointTy()) {
        FinalRetVal = Builder->CreateSIToFP(RetVal, ExpectedTy, "ret_int2fp");
    } 
    else {
        // --- ERROR HANDLING (Cannot Convert) ---
        std::string funcName = ParentFunc->getName().str();
        std::cerr << "\n\033[1;31m[Quanta Error]\033[0m Type Mismatch in function '" 
                  << funcName << "' "
                  << "at line " << Line << "." << std::endl; 

        if (ActualTy->isPointerTy() && ExpectedTy->isIntegerTy()) {
            std::cerr << "  Reason: Cannot implicitly convert a Pointer/String to an Integer." << std::endl;
        } else if (ActualTy->isIntegerTy() && ExpectedTy->isPointerTy()) {
            std::cerr << "  Reason: Cannot implicitly convert an Integer to a Pointer." << std::endl;
        } else {
            std::cerr << "  Reason: No valid casting rule found." << std::endl;
        }

        HasError = true; // Flag global error
        return nullptr;  // Return null to signal failure without exiting
    }

    // ==============================================================
    // --- THE MAGIC: AUTO FREE ALL HEAP MEMORY BEFORE RETURNING ---
    // ==============================================================
    // 1. Get the current function we are inside
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // 2. Loop ONLY through the memory created inside this specific function
    for (auto *Tracker : AutoFreeMap[TheFunction]) {
        
        // Load the actual heap pointer from our stack tracker
        llvm::Value *PtrToFree = Builder->CreateLoad(Builder->getPtrTy(), Tracker);
        llvm::Value *SafeToFree = PtrToFree;

        // 3. Smart check: We don't want to delete the string we are returning!
        // We use LLVM's 'Select' instruction. If the pointer matches what we are returning,
        // we swap it to NULL. (Because calling free(NULL) does nothing, safely!)
        if (FinalRetVal) {
            llvm::Value *IsRet = Builder->CreateICmpEQ(PtrToFree, FinalRetVal, "check_ret");
            SafeToFree = Builder->CreateSelect(IsRet, llvm::ConstantPointerNull::get(Builder->getPtrTy()), PtrToFree);
        }

        // 4. Free the memory
        Builder->CreateCall(getFreeFunc(), {SafeToFree});
    }

    // --- FINALLY, CREATE THE RETURN INSTRUCTION ---
    if (ExpectedTy->isVoidTy()) {
        return Builder->CreateRetVoid();
    } else {
        return Builder->CreateRet(FinalRetVal);
    }
}


llvm::Value *FloatAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

llvm::Value *BoolAST::codegen() {
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(1, Val, false));
}

llvm::Value *CharAST::codegen() {
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(8, Val, false));
}

llvm::Value *StringAST::codegen() {
    // 1. Check if we have already generated this exact string
    if (StringPool.find(val) != StringPool.end()) {
        // We found it! Return the existing memory address
        return StringPool[val];
    }

    // 2. If it's a new string, ask LLVM to create it in Global Memory
    llvm::Value *newString = Builder->CreateGlobalString(val, "strtmp");
    
    // 3. Save it in our pool so we can reuse it next time!
    StringPool[val] = newString;
    
    return newString;
}
// --- 3. VARIABLES ---
llvm::Value *VariableAST::codegen() {
    if (NamedValues.find(Name) == NamedValues.end()) {
        std::cerr << "[Quanta Error] Unknown variable: " << Name << std::endl;
        exit(1);
    }
    VarInfo& info = NamedValues[Name];
    if (info.Type->isArrayTy() || info.Type->isStructTy()) {
        return info.Alloca; // Arrays and Lists shouldn't be loaded into registers by value
    }

    llvm::Value *LoadedVal = Builder->CreateLoad(info.Type, info.Alloca, Name.c_str());

    // Smart Guard: Null Pointer Check for Strings/Objects
    if (info.Type->isPointerTy()) {
        llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
        llvm::BasicBlock *NullBB = llvm::BasicBlock::Create(*TheContext, "null_err", TheFunction);
        llvm::BasicBlock *OkBB = llvm::BasicBlock::Create(*TheContext, "null_ok");
        
        llvm::Value *IsNull = Builder->CreateICmpEQ(LoadedVal, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(info.Type)));
        Builder->CreateCondBr(IsNull, NullBB, OkBB);
        
        Builder->SetInsertPoint(NullBB);
        triggerError("Reference Error: Attempted to access a null or undefined variable");
        
        TheFunction->insert(TheFunction->end(), OkBB);
        Builder->SetInsertPoint(OkBB);
    }

    return LoadedVal;
}


llvm::Value *VarDeclAST::codegen() {
    // 1. Generate Initial Value
    llvm::Value *InitRes = InitVal->codegen();
    if (!InitRes) return nullptr;

    // 2. Determine LLVM Type
    llvm::Type *TargetType = nullptr;
    if (Type == "bool") {
        // Store booleans as 8-bit (1 byte) so they play nice with memory
        TargetType = llvm::IntegerType::get(*TheContext, 8); 
    }
    else if (Type.find("int") != std::string::npos) {
        TargetType = llvm::IntegerType::get(*TheContext, Bytes * 8);
    } 
    else if (Type == "char") {
        TargetType = llvm::IntegerType::get(*TheContext, 8);
    }
    else if (Type.find("float") != std::string::npos) {
        if (Bytes == 4) TargetType = llvm::Type::getFloatTy(*TheContext);
        else TargetType = llvm::Type::getDoubleTy(*TheContext);
    }
    else if (Type == "string") {
        // TargetType = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*TheContext));
        TargetType = llvm::PointerType::getUnqual(*TheContext);
    }
    else {
        TargetType = llvm::IntegerType::get(*TheContext, 32);
    }

    // 3. INTEGER BOUNDS CHECKING (FIXED)
    if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(InitRes)) {
        if (TargetType->isIntegerTy()) {
            
            // --- THE FIX IS HERE ---
            int64_t val;
            // If the input is a 1-bit boolean (i1), use Zero Extension (ZExt)
            // This ensures 'true' is read as 1, not -1.
            if (CI->getBitWidth() == 1) {
                val = CI->getZExtValue();
            } else {
                // For regular numbers, use Sign Extension (SExt) to handle negative numbers
                val = CI->getSExtValue();
            }
            // -----------------------

            // CASE A: Boolean Check
            if (Type == "bool") {
                if (val != 0 && val != 1) {
                    std::cerr << "\n[Quanta Error] Invalid Boolean! Must be true(1) or false(0). Got: " << val << std::endl;
                    return nullptr;
                }
            }
            // CASE B: Integer Overflow Check
            else if (Type.find("int") != std::string::npos) {
                unsigned bits = TargetType->getIntegerBitWidth();
                if (bits < 64) {
                    int64_t maxVal = (1LL << (bits - 1)) - 1;
                    int64_t minVal = -(1LL << (bits - 1));
                    if (val > maxVal || val < minVal) {
                        std::cerr << "\n[Quanta Error] Overflow Detected for '" << Name << "'\n";
                        std::cerr << "  Value: " << val << " | Range: " << minVal << " to " << maxVal << "\n";
                        return nullptr;
                    }
                }
            }
        }
    }

    // 4. MEMORY MANAGEMENT
    llvm::AllocaInst *Alloca = nullptr;
    if (NamedValues.find(Name) != NamedValues.end()) {
        VarInfo& oldInfo = NamedValues[Name];
        llvm::AllocaInst *oldAlloca = llvm::dyn_cast<llvm::AllocaInst>(oldInfo.Alloca);
        if (oldAlloca && oldAlloca->getAllocatedType()->getPrimitiveSizeInBits() >= TargetType->getPrimitiveSizeInBits()) {
            Alloca = oldAlloca;
        } 
    }
    if (!Alloca) {
        llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
        Alloca = TmpB.CreateAlloca(TargetType, nullptr, Name);
    }

    // 5. SAFE CASTING
    llvm::Value *FinalVal = InitRes;
    if (InitRes->getType() != TargetType) {
        if (InitRes->getType()->isIntegerTy() && TargetType->isIntegerTy()) {
            // Bool (i1) -> Int8, or Int64 -> Int8
            // isSigned should be false for booleans to avoid sign extension
            bool isSigned = (InitRes->getType()->getIntegerBitWidth() > 1); 
            FinalVal = Builder->CreateIntCast(InitRes, TargetType, isSigned, "cast_int");
        }
        else if (InitRes->getType()->isFloatingPointTy() && TargetType->isFloatingPointTy()) {
            FinalVal = Builder->CreateFPCast(InitRes, TargetType, "cast_float");
        }
        else if (InitRes->getType()->isIntegerTy() && TargetType->isFloatingPointTy()) {
            FinalVal = Builder->CreateSIToFP(InitRes, TargetType, "cast_int_to_float");
        }
        else if (InitRes->getType()->isFloatingPointTy() && TargetType->isIntegerTy()) {
            FinalVal = Builder->CreateFPToSI(InitRes, TargetType, "cast_float_to_int");
        }
    }

    Builder->CreateStore(FinalVal, Alloca);
    NamedValues[Name] = {Alloca, TargetType, Type, nullptr}; 
    return FinalVal;
 
}

llvm::Value *AssignmentAST::codegen() {
    // 1. Generate the value (RHS)
    llvm::Value *Val = RHS->codegen();
    if (!Val) return nullptr;

    // 2. Check if variable exists
    llvm::Value *Alloca = nullptr;
    llvm::Type *TargetType = nullptr;

    if (NamedValues.find(Name) == NamedValues.end()) {
    
        TargetType = Val->getType();
        
        // BETTER TYPE DETECTION
        std::string TypeName = "unknown";
        
        if (TargetType->isDoubleTy()) {
            TypeName = "float";
        } 
        else if (TargetType->isPointerTy()) {
            TypeName = "string"; 
        } 
        else if (TargetType->isIntegerTy()) {
            int width = TargetType->getIntegerBitWidth();
            if (width == 1) TypeName = "bool";       // Detect Bool (i1)
            else if (width == 8) TypeName = "char";  // Detect Char (i8)
            else TypeName = "int";                   // Default Int (i32/i64)
        }

        // Create memory
        llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
        Alloca = TmpB.CreateAlloca(TargetType, nullptr, Name);
        
        // Save to Symbol Table with correct TypeName
        NamedValues[Name] = {Alloca, TargetType, TypeName};
    } else {
        // --- EXISTING VARIABLE ---
        VarInfo& info = NamedValues[Name];
        Alloca = info.Alloca;
        TargetType = info.Type;
    }

    // 3. Auto-Cast Logic (Safety)
    llvm::Value *FinalVal = Val;
    
    if (Val->getType() != TargetType) {
      
        if (Val->getType()->isIntegerTy() && TargetType->isIntegerTy()) {
            // "ZExt" (Zero Extend) helps safely convert bool(i1) to int(i32)
            // "Trunc" helps convert int to bool/char (though dangerous if value is big)
            FinalVal = Builder->CreateIntCast(Val, TargetType, false, "int_cast");
        }
        // FLOAT -> INT
        else if (Val->getType()->isDoubleTy() && TargetType->isIntegerTy()) {
            FinalVal = Builder->CreateFPToSI(Val, TargetType, "fptosi");
        }
        // INT -> FLOAT
        else if (Val->getType()->isIntegerTy() && TargetType->isDoubleTy()) {
            FinalVal = Builder->CreateSIToFP(Val, TargetType, "sitofp");
        }
    }

    // 4. Store result
    Builder->CreateStore(FinalVal, Alloca);
    return FinalVal;
}
// --- 5. MATH & ASSIGNMENT ---
llvm::Value *BinaryExprAST::codegen() {
  
    // B. MATH
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R) return nullptr;

    bool LIsString = L->getType()->isPointerTy();
    bool RIsString = R->getType()->isPointerTy();

    if (LIsString && RIsString) {
        if (Op == '+') {
            llvm::Value *LenL = Builder->CreateCall(getStrlenFunc(), {L}, "lenL");
            llvm::Value *LenR = Builder->CreateCall(getStrlenFunc(), {R}, "lenR");

            llvm::Value *TotalLen = Builder->CreateAdd(LenL, LenR, "sum_len");
            TotalLen = Builder->CreateAdd(TotalLen, llvm::ConstantInt::get(Builder->getInt64Ty(), 1), "total_len");

            // 1. MALLOC: Ask for memory on the Heap
            llvm::Value *NewHeapStr = Builder->CreateCall(getMallocFunc(), {TotalLen}, "heap_str");

            // 2. KEEP THE RECEIPT: Tell the compiler to remember this pointer!
            trackForAutoFree(NewHeapStr);

            // 3. Copy the text into the new heap memory
            Builder->CreateCall(getStrcpyFunc(), {NewHeapStr, L});
            Builder->CreateCall(getStrcatFunc(), {NewHeapStr, R});

            return NewHeapStr;
        }
        else if (Op == TOK_EQ || Op == TOK_NEQ || Op == '<' || Op == '>' || Op == TOK_GEQ || Op == TOK_LEQ) {
            // Compare strings using strcmp(L, R)
            // strcmp returns: 0 if equal, <0 if L is alphabetically first, >0 if R is first
            llvm::Value *CmpRes = Builder->CreateCall(getStrcmpFunc(), {L, R}, "strcmp_res");
            llvm::Value *Zero = llvm::ConstantInt::get(Builder->getInt32Ty(), 0);
            
            llvm::Value *IsMatch = nullptr;

            switch (Op) {
                case TOK_EQ:  IsMatch = Builder->CreateICmpEQ(CmpRes, Zero, "str_eq"); break;
                case TOK_NEQ: IsMatch = Builder->CreateICmpNE(CmpRes, Zero, "str_neq"); break;
                case '<':     IsMatch = Builder->CreateICmpSLT(CmpRes, Zero, "str_lt"); break;
                case '>':     IsMatch = Builder->CreateICmpSGT(CmpRes, Zero, "str_gt"); break;
                case TOK_LEQ: IsMatch = Builder->CreateICmpSLE(CmpRes, Zero, "str_leq"); break;
                case TOK_GEQ: IsMatch = Builder->CreateICmpSGE(CmpRes, Zero, "str_geq"); break;
            }

            // Convert boolean 1-bit result to a 32-bit integer (1 or 0) for your language
            return Builder->CreateZExt(IsMatch, Builder->getInt32Ty(), "bool_int");
        }
        
        return LogErrorV("Invalid operator for strings. Use +, ==, !=, <, >, <=, or >=.");
    }


    bool LIsFloat = L->getType()->isFloatingPointTy();
    bool RIsFloat = R->getType()->isFloatingPointTy();

    // Smart Guard: Type Error
    // If one is a pointer (String) and the other is a number, generate an unconditional panic/catch jump at runtime
    if ((L->getType()->isPointerTy() && !R->getType()->isPointerTy()) ||
        (!L->getType()->isPointerTy() && R->getType()->isPointerTy())) {
        triggerError("Type Error: Incompatible types for operation");
        return llvm::Constant::getNullValue(L->getType());
    }

    if (LIsFloat && !RIsFloat) {
        R = Builder->CreateSIToFP(R, L->getType(), "cast_int_to_float");
        RIsFloat = true;
    } 
    else if (!LIsFloat && RIsFloat) {
        L = Builder->CreateSIToFP(L, R->getType(), "cast_int_to_float");
        LIsFloat = true;
    }
    
    // SAFE BITWIDTH CHECK
    if (!LIsFloat && !RIsFloat) {
        if (L->getType()->getIntegerBitWidth() < R->getType()->getIntegerBitWidth())
             L = Builder->CreateIntCast(L, R->getType(), true, "grow");
        else if (L->getType()->getIntegerBitWidth() > R->getType()->getIntegerBitWidth())
             R = Builder->CreateIntCast(R, L->getType(), true, "grow");
    }

    bool isFloat = LIsFloat; 

    switch (Op) {
        case '+': return isFloat ? Builder->CreateFAdd(L, R, "add") : Builder->CreateAdd(L, R, "add");
        case '-': return isFloat ? Builder->CreateFSub(L, R, "sub") : Builder->CreateSub(L, R, "sub");
        case '*': return isFloat ? Builder->CreateFMul(L, R, "mul") : Builder->CreateMul(L, R, "mul");
        case '/': {
            // Smart Guard: Division by Zero
            llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *DivZeroBB = llvm::BasicBlock::Create(*TheContext, "div_zero", TheFunction);
            llvm::BasicBlock *DivOkBB = llvm::BasicBlock::Create(*TheContext, "div_ok");
            
            llvm::Value *IsZero = isFloat ? Builder->CreateFCmpOEQ(R, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0))) 
                                          : Builder->CreateICmpEQ(R, llvm::ConstantInt::get(R->getType(), 0));
            Builder->CreateCondBr(IsZero, DivZeroBB, DivOkBB);
            
            Builder->SetInsertPoint(DivZeroBB);
            triggerError("Arithmetic Error: Division by Zero");
            
            TheFunction->insert(TheFunction->end(), DivOkBB);
            Builder->SetInsertPoint(DivOkBB);
            
            return isFloat ? Builder->CreateFDiv(L, R, "div") : Builder->CreateSDiv(L, R, "div");
        }
        case '<':
            L = isFloat ? Builder->CreateFCmpOLT(L, R, "cmp") : Builder->CreateICmpSLT(L, R, "cmp");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
        case '>':
            L = isFloat ? Builder->CreateFCmpOGT(L, R, "cmp") : Builder->CreateICmpSGT(L, R, "cmp");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
        // ... existing cases (+, -, *, /) ...

        case '%': {
            if (L->getType()->isPointerTy() || R->getType()->isPointerTy()) {
                triggerError("Type Error: Incompatible types for operation");
                return llvm::Constant::getNullValue(L->getType());
            }

            // Smart Guard: Division by Zero
            llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *DivZeroBB = llvm::BasicBlock::Create(*TheContext, "mod_zero", TheFunction);
            llvm::BasicBlock *DivOkBB = llvm::BasicBlock::Create(*TheContext, "mod_ok");
            
            llvm::Value *IsZero = isFloat ? Builder->CreateFCmpOEQ(R, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0))) 
                                          : Builder->CreateICmpEQ(R, llvm::ConstantInt::get(R->getType(), 0));
            Builder->CreateCondBr(IsZero, DivZeroBB, DivOkBB);
            
            Builder->SetInsertPoint(DivZeroBB);
            triggerError("Arithmetic Error: Division by Zero");
            
            TheFunction->insert(TheFunction->end(), DivOkBB);
            Builder->SetInsertPoint(DivOkBB);

            if (isFloat) return Builder->CreateFRem(L, R, "remtmp");
            return Builder->CreateSRem(L, R, "remtmp");
        }

   

        // --- NEW: Greater Than or Equal (>=) ---
        case TOK_GEQ:
            if (isFloat) 
                L = Builder->CreateFCmpOGE(L, R, "geq_tmp"); // Ordered Greater Equal
            else 
                L = Builder->CreateICmpSGE(L, R, "geq_tmp"); // Signed Greater Equal
            
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");

        // --- NEW: Less Than or Equal (<=) ---
        case TOK_LEQ:
            if (isFloat) 
                L = Builder->CreateFCmpOLE(L, R, "leq_tmp"); // Ordered Less Equal
            else 
                L = Builder->CreateICmpSLE(L, R, "leq_tmp"); // Signed Less Equal
            
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");

        // --- NEW: Not Equal (!=) ---
        case TOK_NEQ:
            if (isFloat) 
                L = Builder->CreateFCmpONE(L, R, "neq_tmp"); // Ordered Not Equal
            else 
                L = Builder->CreateICmpNE(L, R, "neq_tmp");  // Not Equal
            
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");

        // ... existing case for TOK_EQ ...
        case TOK_EQ:
            if (isFloat) {
                // FCmpOEQ = Floating Point Compare Ordered EQual
                L = Builder->CreateFCmpOEQ(L, R, "eq_tmp");
            } else {
                // ICmpEQ = Integer Compare EQual
                L = Builder->CreateICmpEQ(L, R, "eq_tmp");
            }
            // Convert 1-bit result (i1) to Double (for printing "1.0000" or "0.0000")

            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
            // return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "bool_tmp");
            
        case TOK_AND:
            // Convert to boolean (i1) handles
            L = Builder->CreateICmpNE(L, llvm::ConstantInt::get(L->getType(), 0), "and_lhs_bool");
            R = Builder->CreateICmpNE(R, llvm::ConstantInt::get(R->getType(), 0), "and_rhs_bool");
            L = Builder->CreateLogicalAnd(L, R, "and_res");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
            
        case TOK_OR:
            // Convert to boolean (i1) handles
            L = Builder->CreateICmpNE(L, llvm::ConstantInt::get(L->getType(), 0), "or_lhs_bool");
            R = Builder->CreateICmpNE(R, llvm::ConstantInt::get(R->getType(), 0), "or_rhs_bool");
            L = Builder->CreateLogicalOr(L, R, "or_res");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
            
        default: return nullptr;
    }
}


llvm::Value *UpdateExprAST::codegen() {
    // 1. Find variable address
    if (NamedValues.find(Name) == NamedValues.end())
        return LogErrorV("Unknown variable in update expression");
    
    llvm::Value *V = NamedValues[Name].Alloca;

    // 2. Load current value
    llvm::Value *CurVal = Builder->CreateLoad(NamedValues[Name].Type, V, Name.c_str());

    // 3. Add or Sub 1
    llvm::Value *One = llvm::ConstantInt::get(CurVal->getType(), 1);
    llvm::Value *NextVal;
    
    if (IsIncrement) NextVal = Builder->CreateAdd(CurVal, One, "inc_tmp");
    else             NextVal = Builder->CreateSub(CurVal, One, "dec_tmp");

    // 4. Store new value
    Builder->CreateStore(NextVal, V);

    // 5. Return (Prefix = New, Postfix = Old)
    return IsPrefix ? NextVal : CurVal;
}


// In src/codegen.cpp
llvm::Value *PrintAST::codegen() {
    // 1. Ensure printf exists
    llvm::Function *PrintfFunc = TheModule->getFunction("printf");
    if (!PrintfFunc) {
        std::vector<llvm::Type*> PArgs;
        PArgs.push_back(Builder->getPtrTy()); // i8* for format string
        llvm::FunctionType *PrintfType = llvm::FunctionType::get(
            Builder->getInt32Ty(), PArgs, true); // true = varargs
        PrintfFunc = llvm::Function::Create(PrintfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    }

    // 2. Iterate over all arguments
    for (size_t i = 0; i < Args.size(); ++i) {
        
        // Generate code for the current argument
        llvm::Value *Val = Args[i]->codegen();
        if (!Val) return nullptr;

        llvm::Type *ValType = Val->getType();
        if (ValType->isVoidTy()) {
            return LogErrorV("Cannot print a void return value.");
        }

        llvm::Value *FormatStr = nullptr;
        std::vector<llvm::Value*> PrintArgs;

        // --- Type Detection & Formatting (NO NEWLINES) ---
        if (ValType->isIntegerTy()) {
            unsigned bitWidth = ValType->getIntegerBitWidth();

            if (bitWidth == 8) {
                // char (i8) -> print as character %c (promote to i32 for varargs)
                Val = Builder->CreateZExt(Val, Builder->getInt32Ty(), "char_ext");
                FormatStr = getPooledString("%c", "fmt_char");
            }
            else if (bitWidth == 64) {
                FormatStr = getPooledString("%lld", "fmt_long");
            } 
            else {
                if (bitWidth < 32) {
                    bool isSigned = (bitWidth > 1);
                    Val = Builder->CreateIntCast(Val, Builder->getInt32Ty(), isSigned, "int_promote");
                }
                FormatStr = getPooledString("%d", "fmt_int");
            }
        }
        else if (ValType->isFloatingPointTy()) {
            // Float -> Double -> %f (printf expects doubles)
            if (ValType->isFloatTy()) {
                Val = Builder->CreateFPExt(Val, Builder->getDoubleTy(), "float_ext");
            }
         FormatStr = getPooledString("%f", "fmt_float");
        }
        else if (ValType->isPointerTy()) {
            // String (i8*) -> %s
           FormatStr = getPooledString("%s", "fmt_str");
        } 
        else {
            return LogErrorV("Unknown type passed to print function.");
        }

        // --- Call Printf for this value ---
        PrintArgs.push_back(FormatStr);
        PrintArgs.push_back(Val);
        Builder->CreateCall(PrintfFunc, PrintArgs, "print_call");

        // --- Print SPACE if not the last argument ---
        if (i < Args.size() - 1) {
           llvm::Value *SpaceFmt = getPooledString(" ", "fmt_space");
            Builder->CreateCall(PrintfFunc, {SpaceFmt});
        }
    }

    // 3. Print Final Newline ("\n")
    llvm::Value *NewLineFmt = getPooledString("\n", "fmt_newline");
    Builder->CreateCall(PrintfFunc, {NewLineFmt});

    // 4. Force Buffer Flush (fflush)
    // This guarantees output appears even if the program crashes or ends instantly
    llvm::Function *FFlushFunc = TheModule->getFunction("fflush");
    if (!FFlushFunc) {
        std::vector<llvm::Type*> FFArgs = { Builder->getPtrTy() };
        llvm::FunctionType *FFType = llvm::FunctionType::get(Builder->getInt32Ty(), FFArgs, false);
        FFlushFunc = llvm::Function::Create(FFType, llvm::Function::ExternalLinkage, "fflush", TheModule.get());
    }
    // Call fflush(0) = flush stdout
    Builder->CreateCall(FFlushFunc, { llvm::ConstantPointerNull::get(Builder->getPtrTy()) });

    // Return 0 (Success)
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0));
}

llvm::Value *ByteSizeAST::codegen() {
    // 1. Look up variable
    if (NamedValues.find(Name) == NamedValues.end()) {
        std::cerr << "[Quanta Error] Unknown variable in bytesize: " << Name << std::endl;
        return nullptr;
    }

    // 2. Get the Type
    llvm::Type *T = NamedValues[Name].Type;
    
    // 3. Calculate Size
    // getPrimitiveSizeInBits returns bits (e.g., 32 for int). Divide by 8 to get Bytes.
    uint64_t sizeInBytes = 0;

    if (T->isPointerTy()) {
        sizeInBytes = 8; // Pointers (strings) are 8 bytes on 64-bit systems
    } else {
        sizeInBytes = T->getPrimitiveSizeInBits() / 8;
    }

    // 4. Return as a Number (Constant Integer)
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(64, sizeInBytes));
}

// --- Add to src/codegen.cpp ---

llvm::Value *TypeofAST::codegen() {
    // 1. Look up variable in Symbol Table
    if (NamedValues.find(Name) == NamedValues.end()) {
        std::cerr << "[Quanta Error] Unknown variable in type(): " << Name << std::endl;
        return nullptr;
    }

    // 2. Retrieve the stored Type Name (e.g., "int", "float", "string")
    std::string typeName = NamedValues[Name].TypeName;

    // 3. Create a Global String Pointer (standard string in LLVM)
    // return Builder->CreateGlobalStringPtr(typeName);
    // 1. Create the global string array
auto *GlobalStr = Builder->CreateGlobalString(typeName, "global_str");

// 2. Get a pointer to the first character (index 0)
// This converts [N x i8] (array) -> i8* (pointer)
return Builder->CreateConstGEP2_32(
    GlobalStr->getValueType(), 
    GlobalStr,               
    0, 0,                    
    "str_ptr"
);
}

// --- Generate Code for IF / ELSE ---
llvm::Value *IfExprAST::codegen() {
    // 1. Generate Condition
    llvm::Value *CondV = Cond->codegen();
    if (!CondV) return nullptr;

    // Convert to bool (i1)
    if (CondV->getType()->isDoubleTy()) {
        CondV = Builder->CreateFCmpONE(CondV, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "ifcond");
    } else if (CondV->getType()->isIntegerTy()) {
        CondV = Builder->CreateICmpNE(CondV, llvm::ConstantInt::get(CondV->getType(), 0), "ifcond");
    }

    // 2. Setup Blocks
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    
    // Create blocks. Note: Only 'ThenBB' is attached to function initially.
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

    // 3. Create Branch
    Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // ============================
    //      EMIT 'THEN' BLOCK
    // ============================
    Builder->SetInsertPoint(ThenBB);
    
    llvm::Value *ThenV = Then->codegen();
    if (!ThenV) return nullptr;

    // [FIX] Always jump to Merge if the block is not already terminated (like by a return)
    // We check the CURRENT block because Then->codegen() might have created new internal blocks.
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(MergeBB);
    }

    // ============================
    //      EMIT 'ELSE' BLOCK
    // ============================
    TheFunction->insert(TheFunction->end(), ElseBB); // Attach Else block now
    Builder->SetInsertPoint(ElseBB);
    
    if (Else) {
        llvm::Value *ElseV = Else->codegen();
        if (!ElseV) return nullptr;
    }
    
    // [FIX] Always jump to Merge from Else too
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(MergeBB);
    }

    // ============================
    //      EMIT 'MERGE' BLOCK
    // ============================
    TheFunction->insert(TheFunction->end(), MergeBB); // Attach Merge block now
    Builder->SetInsertPoint(MergeBB);

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}


// --- Generate Code for a Block { ... } ---
llvm::Value *BlockAST::codegen() {
    llvm::Value *LastVal = nullptr;
    
    // Loop through every statement in the block
    for (const auto &Stmt : Statements) {
        LastVal = Stmt->codegen();
        if (!LastVal) return nullptr; // Stop if there was an error
    }
    
// Return the value of the last statement (or 0.0 if empty)
    return LastVal ? LastVal : llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}

// --- EXCEPTION HANDLING AST ---
llvm::Value *TryCatchAST::codegen() {
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // 1. Create Basic Blocks
    llvm::BasicBlock *TryBB = llvm::BasicBlock::Create(*TheContext, "try", TheFunction);
    llvm::BasicBlock *CatchBB = llvm::BasicBlock::Create(*TheContext, "catch");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*TheContext, "after_try");

    // 2. Jump into the Try block from current
    Builder->CreateBr(TryBB);
    Builder->SetInsertPoint(TryBB);

    // 3. Push CatchBB to the global CatchStack BEFORE executing try body
    CatchStack.push(CatchBB);

    // 4. Generate the Try Body
    llvm::Value *TryResult = nullptr;
    for (const auto &Stmt : TryBody) {
        TryResult = Stmt->codegen();
        if (!TryResult) return nullptr;
    }

    // 5. If we reach the end of the Try block without throwing, branch to Merge
    Builder->CreateBr(MergeBB);

    // 6. Pop CatchBB since we exited the Try block normally
    if (!CatchStack.empty() && CatchStack.top() == CatchBB) {
        CatchStack.pop();
    }

    // 7. Generate the Catch Block
    TheFunction->insert(TheFunction->end(), CatchBB);
    Builder->SetInsertPoint(CatchBB);

    // If an error was thrown inside, it jumped here via `triggerError()`.
    // We must pop the stack now so nested try/catch works correctly.
    if (!CatchStack.empty() && CatchStack.top() == CatchBB) {
        CatchStack.pop();
    }

    // a) Retrieve the error message
    llvm::Value* ErrMsg = Builder->CreateLoad(Builder->getPtrTy(), QuantaErrorMsg, "err_msg_ld");
    
    // b) Reset the global error msg to null (so it's clean for future throws)
    Builder->CreateStore(llvm::ConstantPointerNull::get(Builder->getPtrTy()), QuantaErrorMsg);

    // c) Bind the catch variable to the error message (as a String)
    VarInfo OldVar;
    bool hasOldVar = false;
    if (NamedValues.find(CatchVar) != NamedValues.end()) {
        OldVar = NamedValues[CatchVar];
        hasOldVar = true;
    }

    VarInfo NewVar;
    NewVar.Type = Builder->getPtrTy();
    NewVar.TypeName = "string";
    
    // Allocate local var for the exception string
    llvm::AllocaInst *Alloca = Builder->CreateAlloca(NewVar.Type, nullptr, CatchVar.c_str());
    Builder->CreateStore(ErrMsg, Alloca);
    NewVar.Alloca = Alloca;
    NamedValues[CatchVar] = NewVar;

    // d) Generate Catch Body
    llvm::Value *CatchResult = nullptr;
    for (const auto &Stmt : CatchBody) {
        CatchResult = Stmt->codegen();
        if (!CatchResult) return nullptr;
    }

    // Restore overshadowed variable
    if (hasOldVar) NamedValues[CatchVar] = OldVar;
    else NamedValues.erase(CatchVar);

    // e) Jump to MergeBB
    Builder->CreateBr(MergeBB);

    // 8. Continue from MergeBB
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}



// --- 8. SAVE TO FILE ---


void generateObjectCode() {
    // Only initialize the host machine's target — we never cross-compile.
    // InitializeAll* would require linking every LLVM backend (AMDGPU, ARM, RISCV, ...)
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    std::string TargetTriple = llvm::sys::getDefaultTargetTriple();
    // std::string TargetTriple = "arm64-apple-macosx15.0.0";
    // TheModule->setTargetTriple(TargetTriple);
    
    
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    
    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    // ── LLVM API version split ────────────────────────────────────────────────
    // LLVM 18 (Linux apt):  createTargetMachine(StringRef) + setTargetTriple(StringRef)
    // LLVM 19-20 (Windows): createTargetMachine(StringRef, deprecated) + setTargetTriple(Triple)
    // LLVM 21 (macOS brew): createTargetMachine(Triple) + setTargetTriple(Triple)
#if LLVM_VERSION_MAJOR >= 21
    llvm::Triple TT(TargetTriple);
    auto TargetMachine = Target->createTargetMachine(TT, CPU, Features, opt, llvm::Reloc::PIC_);
    TheModule->setTargetTriple(TT);
#elif LLVM_VERSION_MAJOR >= 19
    llvm::Triple TT(TargetTriple);
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);
    TheModule->setTargetTriple(TT);
#else
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);
    TheModule->setTargetTriple(TargetTriple);
#endif
    TheModule->setDataLayout(TargetMachine->createDataLayout());
    std::error_code EC;
    llvm::raw_fd_ostream dest("output.o", EC, llvm::sys::fs::OF_None);
    llvm::legacy::PassManager pass;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        return;
    }
    pass.run(*TheModule);
    dest.flush();
    std::cout << "[Success] Native object file 'output.o' created!" << std::endl;
}

llvm::Value *WhileAST::codegen() {
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // 1. Create Basic Blocks for the loop structure
    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(*TheContext, "loop_cond", TheFunction);
    llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(*TheContext, "loop_body");
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*TheContext, "after_loop");

    // 2. Jump from current location to the Condition Block
    Builder->CreateBr(CondBB);

    // --- CONDITION BLOCK ---
    Builder->SetInsertPoint(CondBB);
    
    // Generate code for the condition (e.g., i < 10)
    llvm::Value *CondV = Cond->codegen();
    if (!CondV) return nullptr;

    // Convert condition to a 1-bit boolean if it isn't already
    // (Checks if the value is "Not Equal" to 0)
    if (CondV->getType()->isIntegerTy()) {
         CondV = Builder->CreateICmpNE(
             CondV, 
             llvm::ConstantInt::get(CondV->getType(), 0), 
             "loopcond"
         );
    }

    // Branch: If True -> Go to Body, If False -> Go to After
    Builder->CreateCondBr(CondV, BodyBB, AfterBB);

    // --- BODY BLOCK ---
    TheFunction->insert(TheFunction->end(), BodyBB);
    Builder->SetInsertPoint(BodyBB);

    // Generate code for the loop body
    if (!Body->codegen()) return nullptr;

    // Jump back to the start (Condition) ONLY if we didn't return/break inside the loop
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(CondBB);
    }

    // --- AFTER BLOCK ---
    TheFunction->insert(TheFunction->end(), AfterBB);
    Builder->SetInsertPoint(AfterBB);

    // Return 0.0 (Loops don't return a value)
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}



// src/codegen.cpp
llvm::Value *CallAST::codegen() {
    // 1. Look up the LLVM function
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
    if (!CalleeF) return LogErrorV(("Undefined function: " + Callee).c_str());

    // 2. Look up Registry Info (needed for Argument Names & Defaults)
    bool hasRegistryInfo = (FunctionRegistry.find(Callee) != FunctionRegistry.end());
    FunctionInfo FuncInfo;
    if (hasRegistryInfo) {
        FuncInfo = FunctionRegistry[Callee];
    }

    unsigned ExpectedCount = CalleeF->arg_size();
    
    // --- STEP A: Initialize Slots with NULL ---
    // We create a bucket for every argument the function expects.
    std::vector<llvm::Value *> FinalArgs(ExpectedCount, nullptr);

    // --- STEP B: Map User Arguments to Slots ---
    int PositionalSlot = 0; // Tracks where the next unnamed argument goes

    for (auto &Arg : Args) {
        // Generate the value (e.g., 30)
        llvm::Value *Val = Arg.Val->codegen();
        if (!Val) return nullptr;

        if (!Arg.Name.empty()) {
            // === KEYWORD ARGUMENT (rollno=30) ===
            // Find which slot index belongs to "rollno"
            int TargetIndex = -1;
            if (hasRegistryInfo) {
                for (unsigned i = 0; i < FuncInfo.Args.size(); ++i) {
                    if (FuncInfo.Args[i].Name == Arg.Name) {
                        TargetIndex = i;
                        break;
                    }
                }
            }

            if (TargetIndex == -1) 
                return LogErrorV(("Unknown argument name: " + Arg.Name).c_str());

            if (FinalArgs[TargetIndex] != nullptr)
                return LogErrorV(("Argument '" + Arg.Name + "' provided twice.").c_str());

            FinalArgs[TargetIndex] = Val;
        } 
        else {
            // === POSITIONAL ARGUMENT (30) ===
            // Skip slots that are already filled by keyword args
            while (PositionalSlot < ExpectedCount && FinalArgs[PositionalSlot] != nullptr) {
                PositionalSlot++;
            }

            if (PositionalSlot >= ExpectedCount) 
                return LogErrorV("Too many positional arguments passed.");

            FinalArgs[PositionalSlot] = Val;
            // Move slot counter for the next positional arg
            PositionalSlot++;
        }
    }

    // --- STEP C: Fill Missing Slots with DEFAULTS ---
    for (unsigned i = 0; i < ExpectedCount; ++i) {
        if (FinalArgs[i] == nullptr) {
            // Check if we have a default value in the registry
            if (hasRegistryInfo && i < FuncInfo.Args.size() && FuncInfo.Args[i].DefaultValue) {
                FinalArgs[i] = FuncInfo.Args[i].DefaultValue->codegen();
            } else {
                return LogErrorV(("Missing required argument #" + std::to_string(i+1)).c_str());
            }
        }
    }

    // --- STEP D: Automatic Type Casting ---
    for (unsigned i = 0; i < ExpectedCount; ++i) {
        llvm::Type *ExpectedType = CalleeF->getArg(i)->getType();
        llvm::Value *ArgVal = FinalArgs[i];

        if (ArgVal->getType() != ExpectedType) {
            if (ArgVal->getType()->isIntegerTy() && ExpectedType->isIntegerTy()) {
                ArgVal = Builder->CreateIntCast(ArgVal, ExpectedType, true, "arg_cast");
            }
            else if (ArgVal->getType()->isIntegerTy() && ExpectedType->isFloatingPointTy()) {
                ArgVal = Builder->CreateSIToFP(ArgVal, ExpectedType, "arg_i2f");
            }
            else if (ArgVal->getType()->isFloatingPointTy() && ExpectedType->isIntegerTy()) {
                ArgVal = Builder->CreateFPToSI(ArgVal, ExpectedType, "arg_f2i");
            }
            else if (ArgVal->getType()->isFloatingPointTy() && ExpectedType->isFloatingPointTy()) {
                ArgVal = Builder->CreateFPCast(ArgVal, ExpectedType, "arg_fp_resize");
            }
        }
        FinalArgs[i] = ArgVal;
    }

    // 5. Generate Call
    if (CalleeF->getReturnType()->isVoidTy()) {
        return Builder->CreateCall(CalleeF, FinalArgs);
    } else {
        return Builder->CreateCall(CalleeF, FinalArgs, "calltmp");
    }
}

// Legacy Strings have been migrated to OOP logic

llvm::Value *StringIndexAST::codegen() {
    // 1. Array / List Check
    if (auto *VarAst = dynamic_cast<VariableAST*>(BaseExpr.get())) {
        std::string name = VarAst->getName();
        if (NamedValues.find(name) != NamedValues.end()) {
            VarInfo &info = NamedValues[name];
            
            if (info.Type->isArrayTy() || info.Type->isStructTy()) {

                if (info.Type->isArrayTy()) {
                    // Fixed Array (Stack) - N-dimensional Support
                    std::vector<llvm::Value*> IdxList;
                    IdxList.push_back(llvm::ConstantInt::get(Builder->getInt32Ty(), 0));
                    llvm::Type *CurrentArrayTy = info.Type;
                    for (auto &idxAST : Indices) {
                        llvm::Value *IdxVal = idxAST->codegen();
                        if (!IdxVal) return nullptr;

                        // Smart Guard: Fixed Array Bounds
                        if (auto *AT = llvm::dyn_cast<llvm::ArrayType>(CurrentArrayTy)) {
                            llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
                            llvm::BasicBlock *OobBB = llvm::BasicBlock::Create(*TheContext, "oob", TheFunction);
                            llvm::BasicBlock *OkBB = llvm::BasicBlock::Create(*TheContext, "idx_ok");

                            llvm::Value *Length = llvm::ConstantInt::get(IdxVal->getType(), AT->getNumElements());
                            llvm::Value *IsNeg = Builder->CreateICmpSLT(IdxVal, llvm::ConstantInt::get(IdxVal->getType(), 0));
                            llvm::Value *IsTooBig = Builder->CreateICmpSGE(IdxVal, Length);
                            llvm::Value *IsOob = Builder->CreateOr(IsNeg, IsTooBig, "is_oob");

                            Builder->CreateCondBr(IsOob, OobBB, OkBB);

                            Builder->SetInsertPoint(OobBB);
                            triggerError("Index Error: Array index out of bounds");

                            TheFunction->insert(TheFunction->end(), OkBB);
                            Builder->SetInsertPoint(OkBB);
                            CurrentArrayTy = AT->getElementType();
                        }

                        IdxList.push_back(IdxVal);
                    }
                    llvm::Value *ElementPtr = Builder->CreateInBoundsGEP(info.Type, info.Alloca, IdxList);

                    // Compute the yield type dynamically
                    llvm::Type *YieldType = info.Type;
                    for (size_t i = 0; i < Indices.size(); i++) {
                        if (auto *AT = llvm::dyn_cast<llvm::ArrayType>(YieldType)) {
                            YieldType = AT->getElementType();
                        }
                    }

                    return Builder->CreateLoad(YieldType, ElementPtr, "arr_read");
                } else if (info.Type->isStructTy()) {
                    // Dynamic List (Heap) - Only 1D supported for dynamic reading so far
                    if (Indices.size() != 1) return LogErrorV("Multi-dimensional dynamic lists not yet supported for reading");
                    llvm::Value *IdxVal = Indices[0]->codegen();
                    if (!IdxVal) return nullptr;

                    // Struct layout: { ptr=0, len=1, cap=2 }
                    llvm::Value *LenFieldPtr = Builder->CreateStructGEP(info.Type, info.Alloca, 1); // field 1 = len
                    llvm::Value *ListLength = Builder->CreateLoad(Builder->getInt32Ty(), LenFieldPtr, "list_len");
                    
                    // Smart Guard: Dynamic List Bounds Check
                    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
                    llvm::BasicBlock *OobBB = llvm::BasicBlock::Create(*TheContext, "oob_list", TheFunction);
                    llvm::BasicBlock *OkBB = llvm::BasicBlock::Create(*TheContext, "idx_list_ok");

                    llvm::Value *Idx32 = Builder->CreateIntCast(IdxVal, Builder->getInt32Ty(), true, "idx32");
                    llvm::Value *IsNeg = Builder->CreateICmpSLT(Idx32, llvm::ConstantInt::get(Builder->getInt32Ty(), 0));
                    llvm::Value *IsTooBig = Builder->CreateICmpSGE(Idx32, ListLength);
                    llvm::Value *IsOob = Builder->CreateOr(IsNeg, IsTooBig, "is_oob");

                    Builder->CreateCondBr(IsOob, OobBB, OkBB);

                    Builder->SetInsertPoint(OobBB);
                    triggerError("Index Error: Array index out of bounds");

                    TheFunction->insert(TheFunction->end(), OkBB);
                    Builder->SetInsertPoint(OkBB);

                    llvm::Value *PtrField = Builder->CreateStructGEP(info.Type, info.Alloca, 0); // field 0 = data ptr
                    llvm::Value *BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField);
                    
                    // GEP on the dynamically sized buffer
                    llvm::Value *ListElementPtr = Builder->CreateGEP(info.ElementType, BufferPtr, IdxVal);
                    return Builder->CreateLoad(info.ElementType, ListElementPtr, "list_read");
                }
            }
        }
    }

    llvm::Value *Base = BaseExpr->codegen();
    if (Indices.empty()) return nullptr;
    if (Indices.size() > 1) return LogErrorV("Strings only support 1D indexing.");
    llvm::Value *Index = Indices[0]->codegen();
    if (!Base || !Index) return nullptr;
    if (!Base->getType()->isPointerTy()) return LogErrorV("String index base must be a string (pointer).");
    if (!Index->getType()->isIntegerTy()) return LogErrorV("String index must be an integer.");
    llvm::Value *Idx64 = Index->getType()->getIntegerBitWidth() < 64
        ? Builder->CreateSExt(Index, Builder->getInt64Ty(), "idx64")
        : (Index->getType()->getIntegerBitWidth() > 64 ? Builder->CreateTrunc(Index, Builder->getInt64Ty(), "idx64") : Index);
    /* Negative index: -1 = last char, use index + len */
    llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {Base}, "idx_len");
    llvm::Value *Zero64 = llvm::ConstantInt::get(Builder->getInt64Ty(), 0);
    llvm::Value *IsNegNative = Builder->CreateICmpSLT(Idx64, Zero64, "idx_neg_native");
    llvm::Value *Adjusted64 = Builder->CreateSelect(IsNegNative, Builder->CreateAdd(Idx64, Len64, "idx_adj"), Idx64, "idx_final");

    // Smart Guard: String Bounds Check
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *OobBB = llvm::BasicBlock::Create(*TheContext, "oob_str", TheFunction);
    llvm::BasicBlock *OkBB = llvm::BasicBlock::Create(*TheContext, "idx_str_ok");

    llvm::Value *IsNeg = Builder->CreateICmpSLT(Adjusted64, Zero64);
    llvm::Value *IsTooBig = Builder->CreateICmpSGE(Adjusted64, Len64);
    llvm::Value *IsOob = Builder->CreateOr(IsNeg, IsTooBig, "is_oob");

    Builder->CreateCondBr(IsOob, OobBB, OkBB);

    Builder->SetInsertPoint(OobBB);
    triggerError("Index Error: Array index out of bounds");

    TheFunction->insert(TheFunction->end(), OkBB);
    Builder->SetInsertPoint(OkBB);

    llvm::Value *CharAddr = Builder->CreateGEP(Builder->getInt8Ty(), Base, Adjusted64, "char_addr");
    return Builder->CreateLoad(Builder->getInt8Ty(), CharAddr, "char_val");
}

llvm::Value *StringSliceAST::codegen() {
    llvm::Value *Base = BaseExpr->codegen();
    llvm::Value *Start = StartExpr->codegen();
    llvm::Value *End = EndExpr->codegen();
    if (!Base || !Start || !End) return nullptr;
    if (!Base->getType()->isPointerTy()) return LogErrorV("Slice base must be a string.");
    llvm::Value *Start32 = Start->getType()->isIntegerTy() && Start->getType()->getIntegerBitWidth() != 32
        ? Builder->CreateIntCast(Start, Builder->getInt32Ty(), true, "start32") : Start;
    llvm::Value *End32 = End->getType()->isIntegerTy() && End->getType()->getIntegerBitWidth() != 32
        ? Builder->CreateIntCast(End, Builder->getInt32Ty(), true, "end32") : End;
    llvm::Value *Step32 = nullptr;
    if (StepExpr) {
        Step32 = StepExpr->codegen();
        if (!Step32) return nullptr;
        if (Step32->getType()->isIntegerTy() && Step32->getType()->getIntegerBitWidth() != 32)
            Step32 = Builder->CreateIntCast(Step32, Builder->getInt32Ty(), true, "step32");
    } else {
        Step32 = llvm::ConstantInt::get(Builder->getInt32Ty(), 1);
    }
    llvm::Value *NewStr = Builder->CreateCall(getQuantaSliceFunc(), {Base, Start32, End32, Step32}, "slice_res");
    trackForAutoFree(NewStr);
    return NewStr;
}

llvm::Value *ForIterableAST::codegen() {
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // 1. Evaluate the iterable expression
    llvm::Value *IterableVal = IterableExpr->codegen();
    if (!IterableVal) return nullptr;

    llvm::Type *IterableTy = IterableVal->getType();

    // Determine type information from named variables if possible (for arrays/lists)
    llvm::Type *ElementTy = Builder->getInt8Ty(); // Default for String
    llvm::Value *Len32 = nullptr;
    llvm::Value *BufferPtr = IterableVal;
    std::string QuantaTypeName = "char";

    // Attempt to extract array/list length dynamically
    if (auto *VarAst = dynamic_cast<VariableAST*>(IterableExpr.get())) {
        std::string name = VarAst->getName();
        if (NamedValues.find(name) != NamedValues.end()) {
            VarInfo &info = NamedValues[name];
            
            if (info.Type->isArrayTy()) {
                // Fixed Array
                auto *AT = llvm::cast<llvm::ArrayType>(info.Type);
                ElementTy = AT->getElementType();
                QuantaTypeName = info.TypeName; // E.g., "int"
                Len32 = llvm::ConstantInt::get(Builder->getInt32Ty(), AT->getNumElements());
                
                // GEP to get pointer to first element of the array
                BufferPtr = Builder->CreateInBoundsGEP(
                    info.Type, info.Alloca, 
                    {llvm::ConstantInt::get(Builder->getInt32Ty(), 0), llvm::ConstantInt::get(Builder->getInt32Ty(), 0)}
                );
            } else if (info.Type->isStructTy()) {
                // Dynamic List — struct layout: { ptr=0, len=1, cap=2 }
                ElementTy = info.ElementType;
                QuantaTypeName = info.TypeName;
                
                llvm::Value *LenFieldPtr = Builder->CreateStructGEP(info.Type, info.Alloca, 1); // field 1 = len
                Len32 = Builder->CreateLoad(Builder->getInt32Ty(), LenFieldPtr, "list_len");
                
                llvm::Value *PtrField = Builder->CreateStructGEP(info.Type, info.Alloca, 0); // field 0 = data ptr
                BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField);
            }
        }
    }

    // Default string fallback if length isn't calculated yet
    if (!Len32) {
        if (!IterableTy->isPointerTy()) return LogErrorV("Loop iterable must be an array, list, or string.");
        llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {IterableVal}, "str_len");
        Len32 = Builder->CreateTrunc(Len64, Builder->getInt32Ty(), "len32");
        QuantaTypeName = "char";
    }

    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(*TheContext, "loop_str_cond", TheFunction);
    llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(*TheContext, "loop_str_body");
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*TheContext, "loop_str_after");

    // 1. Setup loop counter (hidden)
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    llvm::AllocaInst *IndexAlloca = TmpB.CreateAlloca(Builder->getInt32Ty(), nullptr, "loop_idx");
    TmpB.CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), 0), IndexAlloca);

    // 2. Setup user variable (which will hold the element)
    llvm::AllocaInst *VarAlloca = TmpB.CreateAlloca(ElementTy, nullptr, VarName);
    Builder->CreateBr(CondBB);

    // --- CONDITION BLOCK ---
    Builder->SetInsertPoint(CondBB);
    llvm::Value *CurIdx = Builder->CreateLoad(Builder->getInt32Ty(), IndexAlloca, "cur_i");
    llvm::Value *CondV = Builder->CreateICmpSLT(CurIdx, Len32, "loop_str_cmp");
    Builder->CreateCondBr(CondV, BodyBB, AfterBB);

    // --- BODY BLOCK ---
    TheFunction->insert(TheFunction->end(), BodyBB);
    Builder->SetInsertPoint(BodyBB);

    // Load the element at CurIdx
    llvm::Value *CurIdx64 = Builder->CreateZExt(CurIdx, Builder->getInt64Ty());
    llvm::Value *ElementAddr = Builder->CreateGEP(ElementTy, BufferPtr, CurIdx64, "elem_addr");
    
    // Arrays/Lists might already return a value, Strings return int8.
    llvm::Value *ElementVal = Builder->CreateLoad(ElementTy, ElementAddr, "elem_val");

    // Store the element into the user's loop variable
    Builder->CreateStore(ElementVal, VarAlloca);

    // Register the user variable so the loop body can see it
    NamedValues[VarName] = VarInfo{VarAlloca, ElementTy, QuantaTypeName, nullptr};

    // Execute the loop body
    if (!Body->codegen()) return nullptr;

    // Increment hidden index
    llvm::Value *NextIdx = Builder->CreateAdd(CurIdx, llvm::ConstantInt::get(Builder->getInt32Ty(), 1), "next_i");
    Builder->CreateStore(NextIdx, IndexAlloca);
    Builder->CreateBr(CondBB);

    // --- AFTER BLOCK ---
    TheFunction->insert(TheFunction->end(), AfterBB);
    Builder->SetInsertPoint(AfterBB);
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}

llvm::Value *FixedStringDeclAST::codegen() {
    // 1. Tell the terminal we are building a safe stack string!
    std::cout << "[Codegen] Allocating embedded stack buffer for: " << VarName << " (Size: " << Capacity << ")" << std::endl;

    // 2. Create the fixed-size array on the stack
    llvm::ArrayType *ArrayTy = llvm::ArrayType::get(Builder->getInt8Ty(), Capacity);
    llvm::AllocaInst *StackBuffer = Builder->CreateAlloca(ArrayTy, nullptr, VarName + "_buffer");
    llvm::Value *BufferPtr = StackBuffer;

    // 3. Generate the giant string we want to copy
    llvm::Value *InitVal = InitValue->codegen();
    if (!InitVal) return nullptr;

    // ==========================================================
    // [NEW] HARDWARE-LEVEL MEMORY COPY (Bypasses C strncpy)
    // ==========================================================
    // We force LLVM to copy exactly 'Capacity' bytes from the giant string into our StackBuffer
    llvm::Value *CapVal = llvm::ConstantInt::get(Builder->getInt64Ty(), Capacity);
    
    // CreateMemCpy(Dest, DestAlign, Src, SrcAlign, Size)
    // Align of 1 means characters (bytes).
    Builder->CreateMemCpy(
        BufferPtr, llvm::MaybeAlign(1), 
        InitVal, llvm::MaybeAlign(1), 
        CapVal
    );
    // ==========================================================

    // 4. Force null-termination at the very end of our buffer
    llvm::Value *LastIndex = llvm::ConstantInt::get(Builder->getInt64Ty(), Capacity - 1);
    llvm::Value *LastCharAddr = Builder->CreateGEP(Builder->getInt8Ty(), BufferPtr, LastIndex, "safety_null");
    Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt8Ty(), 0), LastCharAddr);

    // 5. Save ONLY the safe buffer pointer in our Variable Registry
    llvm::AllocaInst *VarAlloca = Builder->CreateAlloca(Builder->getPtrTy(), nullptr, VarName);
    
    // [CRITICAL] Store BufferPtr into the variable, NOT InitVal!
    Builder->CreateStore(BufferPtr, VarAlloca); 
    
    NamedValues[VarName] = VarInfo{VarAlloca, Builder->getPtrTy(), "string[" + std::to_string(Capacity) + "]", nullptr};

    return BufferPtr;
}

// --- NEW ARRAY / LIST COMPILERS ---

llvm::Function* getReallocFunc() {
    llvm::Function *F = TheModule->getFunction("realloc");
    if (!F) {
        std::vector<llvm::Type*> Args = { Builder->getPtrTy(), Builder->getInt64Ty() };
        llvm::FunctionType *FT = llvm::FunctionType::get(Builder->getPtrTy(), Args, false);
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "realloc", TheModule.get());
    }
    return F;
}

llvm::Value *ArrayExprAST::codegen() {
    return LogErrorV("Raw array literals cannot be evaluated directly.");
}

llvm::Value *FixedArrayDeclAST::codegen() {
    // Determine Element Type
    llvm::Type *ElementType = Builder->getInt32Ty(); 
    if (TypeName == "float") ElementType = Builder->getDoubleTy();
    else if (TypeName == "string") ElementType = Builder->getPtrTy();
    else if (TypeName == "bool" || TypeName == "char") ElementType = Builder->getInt8Ty();

    // Allocate Array on the Stack
    llvm::Type *ArrayTy = ElementType;
    for (auto it = Dimensions.rbegin(); it != Dimensions.rend(); ++it) {
        ArrayTy = llvm::ArrayType::get(ArrayTy, *it);
    }
    llvm::AllocaInst *ArrayAlloca = Builder->CreateAlloca(ArrayTy, nullptr, VarName);

    // Initialize Elements if provided
    if (InitValue) {
        if (auto *ArrayLit = dynamic_cast<ArrayExprAST*>(InitValue.get())) {
            
            // 1D Array Initialization
            if (Dimensions.size() == 1) {
                int Size = Dimensions[0];
                for (size_t i = 0; i < ArrayLit->Elements.size() && i < Size; i++) {
                    llvm::Value *Val = ArrayLit->Elements[i]->codegen();
                    if (!Val) return nullptr;
                    
                    if (Val->getType() != ElementType) {
                        if (Val->getType()->isIntegerTy() && ElementType->isFloatingPointTy()) Val = Builder->CreateSIToFP(Val, ElementType);
                        else if (Val->getType()->isFloatingPointTy() && ElementType->isIntegerTy()) Val = Builder->CreateFPToSI(Val, ElementType);
                        else if (Val->getType()->isIntegerTy() && ElementType->isIntegerTy()) Val = Builder->CreateIntCast(Val, ElementType, true);
                    }
                    
                    llvm::Value *IdxList[] = { llvm::ConstantInt::get(Builder->getInt32Ty(), 0), llvm::ConstantInt::get(Builder->getInt32Ty(), i) };
                    llvm::Value *ElementPtr = Builder->CreateInBoundsGEP(ArrayTy, ArrayAlloca, IdxList, "element_ptr");
                    Builder->CreateStore(Val, ElementPtr);
                }
            }
            // 2D Matrix Initialization
            else if (Dimensions.size() == 2) {
                int Rows = Dimensions[0];
                int Cols = Dimensions[1];
                for (size_t i = 0; i < ArrayLit->Elements.size() && i < Rows; i++) {
                    if (auto *SubArrayLit = dynamic_cast<ArrayExprAST*>(ArrayLit->Elements[i].get())) {
                        for (size_t j = 0; j < SubArrayLit->Elements.size() && j < Cols; j++) {
                            llvm::Value *Val = SubArrayLit->Elements[j]->codegen();
                            if (!Val) return nullptr;
                            
                            if (Val->getType() != ElementType) {
                                if (Val->getType()->isIntegerTy() && ElementType->isFloatingPointTy()) Val = Builder->CreateSIToFP(Val, ElementType);
                                else if (Val->getType()->isFloatingPointTy() && ElementType->isIntegerTy()) Val = Builder->CreateFPToSI(Val, ElementType);
                                else if (Val->getType()->isIntegerTy() && ElementType->isIntegerTy()) Val = Builder->CreateIntCast(Val, ElementType, true);
                            }
                            
                            llvm::Value *IdxList[] = { 
                                llvm::ConstantInt::get(Builder->getInt32Ty(), 0), 
                                llvm::ConstantInt::get(Builder->getInt32Ty(), i),
                                llvm::ConstantInt::get(Builder->getInt32Ty(), j)
                            };
                            llvm::Value *ElementPtr = Builder->CreateInBoundsGEP(ArrayTy, ArrayAlloca, IdxList, "matrix_ptr");
                            Builder->CreateStore(Val, ElementPtr);
                        }
                    }
                }
            } else {
                return LogErrorV("Static array initializers only support up to 2 dimensions currently.");
            }
        }
    }

    std::string FullTypeName = TypeName;
    for (int d : Dimensions) {
        FullTypeName += "[" + std::to_string(d) + "]";
    }
    NamedValues[VarName] = VarInfo{ArrayAlloca, ArrayTy, FullTypeName, ElementType};
    return ArrayAlloca;
}

llvm::Value *DynamicListDeclAST::codegen() {
    // Determine Element Type
    llvm::Type *ElementType = Builder->getInt32Ty(); 
    if (TypeName == "float") ElementType = Builder->getDoubleTy();
    else if (TypeName == "string") ElementType = Builder->getPtrTy();
    else if (TypeName == "bool" || TypeName == "char") ElementType = Builder->getInt8Ty();

    // Struct: { ElementType*, i32 len, i32 capacity }
    llvm::StructType *ListStructTy = llvm::StructType::get(*TheContext, {
        Builder->getPtrTy(),
        Builder->getInt32Ty(),
        Builder->getInt32Ty()
    });

    llvm::Value *ListAlloca = nullptr;

    if (InitValue && !dynamic_cast<ArrayExprAST*>(InitValue.get())) {
        // The initializer is a function call (or variable alias) returning a List struct pointer!
        ListAlloca = InitValue->codegen();
        if (!ListAlloca) return nullptr;
    } else {
        // Malloc the struct container on the heap instead of the stack so it can be safely returned
        llvm::Function *MallocFn = getMallocFunc();
        const llvm::DataLayout &DL = TheModule->getDataLayout();
        uint64_t StructBytesLen = DL.getTypeAllocSize(ListStructTy);
        llvm::Value *StructSize = llvm::ConstantInt::get(Builder->getInt64Ty(), StructBytesLen);
        ListAlloca = Builder->CreateCall(MallocFn, {StructSize}, "list_struct_" + VarName);

        // Calculate initial capacity & heap bytes
        int initialCap = 8;
        int elSize = ElementType->getPrimitiveSizeInBits() / 8;
        if (elSize == 0 && ElementType->isPointerTy()) elSize = 8;

        llvm::Value *BytesToAlloc = llvm::ConstantInt::get(Builder->getInt64Ty(), initialCap * elSize);
        llvm::Value *BufferPtrRaw = Builder->CreateCall(getMallocFunc(), {BytesToAlloc}, "list_alloc");
        // Removed trackForAutoFree so the array can safely escape the function

        // Setup fields
        llvm::Value *PtrField = Builder->CreateStructGEP(ListStructTy, ListAlloca, 0);
        Builder->CreateStore(BufferPtrRaw, PtrField);

        llvm::Value *LenField = Builder->CreateStructGEP(ListStructTy, ListAlloca, 1);
        Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), 0), LenField);

        llvm::Value *CapField = Builder->CreateStructGEP(ListStructTy, ListAlloca, 2);
        Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), initialCap), CapField);

        // Initialize from literal [x, y, z]
        if (InitValue) {
            if (auto *ArrayLit = dynamic_cast<ArrayExprAST*>(InitValue.get())) {
                for (size_t i = 0; i < ArrayLit->Elements.size(); i++) {
                    llvm::Value *Val = ArrayLit->Elements[i]->codegen();
                    if (!Val) return nullptr;
                    
                    if (Val->getType() != ElementType) {
                        if (Val->getType()->isIntegerTy() && ElementType->isFloatingPointTy()) Val = Builder->CreateSIToFP(Val, ElementType);
                        else if (Val->getType()->isFloatingPointTy() && ElementType->isIntegerTy()) Val = Builder->CreateFPToSI(Val, ElementType);
                        else if (Val->getType()->isIntegerTy() && ElementType->isIntegerTy()) Val = Builder->CreateIntCast(Val, ElementType, true);
                    }

                    llvm::Value *IdxGEP = Builder->CreateGEP(ElementType, BufferPtrRaw, llvm::ConstantInt::get(Builder->getInt32Ty(), i), "init_idx");
                    Builder->CreateStore(Val, IdxGEP);
                }
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), ArrayLit->Elements.size()), LenField);
            }
        }
    }

    NamedValues[VarName] = VarInfo{ListAlloca, ListStructTy, TypeName + "[]", ElementType};
    return ListAlloca;
}

llvm::Value *IndexAssignAST::codegen() {
    auto *VarAst = dynamic_cast<VariableAST*>(Obj.get());
    if (!VarAst) return LogErrorV("Cannot assign to non-variable index");
    
    std::string name = VarAst->getName();
    if (NamedValues.find(name) == NamedValues.end()) return LogErrorV("Unknown variable in index assignment");

    VarInfo &info = NamedValues[name];
    llvm::Value *Val = Value->codegen();
    if (!Val) return nullptr;

    if (info.Type->isArrayTy()) {
        std::vector<llvm::Value*> IdxList;
        IdxList.push_back(llvm::ConstantInt::get(Builder->getInt32Ty(), 0));
        for (auto &idxAST : Indices) {
            llvm::Value *IdxVal = idxAST->codegen();
            if (!IdxVal) return nullptr;
            IdxList.push_back(IdxVal);
        }
        llvm::Value *ElementPtr = Builder->CreateInBoundsGEP(info.Type, info.Alloca, IdxList, "arr_write");
        Builder->CreateStore(Val, ElementPtr);
        return Val;
    } else if (info.Type->isStructTy()) {
        if (Indices.size() != 1) return LogErrorV("Multi-dimensional dynamic lists not yet supported for writing");
        llvm::Value *Idx = Indices[0]->codegen();
        if (!Idx) return nullptr;

        llvm::Value *PtrField = Builder->CreateStructGEP(info.Type, info.Alloca, 0);
        llvm::Value *BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField);
        llvm::Value *ElementPtr = Builder->CreateGEP(info.ElementType, BufferPtr, Idx, "list_write");
        Builder->CreateStore(Val, ElementPtr);
        return Val;
    } else if (info.Type->isPointerTy()) {
        // String / Pointer mutation
        if (Indices.size() != 1) return LogErrorV("Strings only support 1D indexing.");
        llvm::Value *Idx = Indices[0]->codegen();
        if (!Idx) return nullptr;

        llvm::Value *BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), info.Alloca);
        llvm::Value *Idx64 = Idx->getType()->getIntegerBitWidth() < 64
                             ? Builder->CreateSExt(Idx, Builder->getInt64Ty())
                             : (Idx->getType()->getIntegerBitWidth() > 64 ? Builder->CreateTrunc(Idx, Builder->getInt64Ty()) : Idx);

        llvm::Value *StrLen = Builder->CreateCall(getStrlenFunc(), {BufferPtr});
        llvm::Value *Zero64 = llvm::ConstantInt::get(Builder->getInt64Ty(), 0);
        llvm::Value *IsNeg = Builder->CreateICmpSLT(Idx64, Zero64);
        llvm::Value *AdjustedIdx = Builder->CreateSelect(IsNeg, Builder->CreateAdd(Idx64, StrLen), Idx64);

        llvm::Value *ElementPtr = Builder->CreateGEP(Builder->getInt8Ty(), BufferPtr, AdjustedIdx, "str_write");
        
        // Ensure the value being stored is an 8-bit character
        if (Val->getType() != Builder->getInt8Ty()) {
            if (Val->getType()->isIntegerTy()) {
                Val = Builder->CreateTrunc(Val, Builder->getInt8Ty(), "char_cast");
            } else {
                return LogErrorV("Can only assign characters (or integers) to a string index");
            }
        }
        
        Builder->CreateStore(Val, ElementPtr);
        return Val;
    } else {
        return LogErrorV("Cannot index into this specific type.");
    }
}

llvm::Value *MethodCallAST::codegen() {
    llvm::Value *ObjVal = Obj->codegen();
    if (!ObjVal) return nullptr;

    auto *VarAst = dynamic_cast<VariableAST*>(Obj.get());
    std::string ObjName = VarAst ? VarAst->getName() : "";

    // 1. DYNAMIC LIST METHODS (Heap Lists)
    if (!ObjName.empty() && NamedValues.find(ObjName) != NamedValues.end()) {
        VarInfo &info = NamedValues[ObjName];
        if (info.Type->isStructTy()) {
            if (MethodName == "push") {
                if (Args.size() != 1) return LogErrorV("push() requires exactly 1 argument.");
                llvm::Value *Val = Args[0]->codegen();
                if (!Val) return nullptr;

                llvm::Value *PtrField = Builder->CreateStructGEP(info.Type, info.Alloca, 0);
                llvm::Value *LenField = Builder->CreateStructGEP(info.Type, info.Alloca, 1);
                llvm::Value *CapField = Builder->CreateStructGEP(info.Type, info.Alloca, 2);

                llvm::Value *BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField, "buf_ptr");
                llvm::Value *Len = Builder->CreateLoad(Builder->getInt32Ty(), LenField, "len");
                llvm::Value *Cap = Builder->CreateLoad(Builder->getInt32Ty(), CapField, "cap");

                llvm::Value *IsFull = Builder->CreateICmpEQ(Len, Cap, "is_full");

                llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
                llvm::BasicBlock *ReallocBB = llvm::BasicBlock::Create(*TheContext, "realloc_bb", TheFunction);
                llvm::BasicBlock *PushBB = llvm::BasicBlock::Create(*TheContext, "push_bb");

                Builder->CreateCondBr(IsFull, ReallocBB, PushBB);

                // --- REALLOC BLOCK ---
                Builder->SetInsertPoint(ReallocBB);
                llvm::Value *NewCap = Builder->CreateMul(Cap, llvm::ConstantInt::get(Builder->getInt32Ty(), 2), "new_cap");
                Builder->CreateStore(NewCap, CapField);

                llvm::Value *NewCap64 = Builder->CreateZExt(NewCap, Builder->getInt64Ty());
                uint64_t elSize = info.ElementType->getPrimitiveSizeInBits() / 8;
                if (elSize == 0 && info.ElementType->isPointerTy()) elSize = 8;
                
                llvm::Value *Bytes64 = Builder->CreateMul(NewCap64, llvm::ConstantInt::get(Builder->getInt64Ty(), elSize), "bytes");

                llvm::Value *NewBufferPtr = Builder->CreateCall(getReallocFunc(), {BufferPtr, Bytes64}, "new_buffer");
                Builder->CreateStore(NewBufferPtr, PtrField);
                
                // Re-track memory for AutoFree feature (if realloc moved it)
                trackForAutoFree(NewBufferPtr); 
                
                Builder->CreateBr(PushBB);

                // --- PUSH BLOCK ---
                TheFunction->insert(TheFunction->end(), PushBB);
                Builder->SetInsertPoint(PushBB);

                llvm::Value *FinalBufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField);
                llvm::Value *IdxGEP = Builder->CreateGEP(info.ElementType, FinalBufferPtr, Len, "push_idx");
                Builder->CreateStore(Val, IdxGEP);

                llvm::Value *NewLen = Builder->CreateAdd(Len, llvm::ConstantInt::get(Builder->getInt32Ty(), 1), "new_len");
                Builder->CreateStore(NewLen, LenField);

                return llvm::Constant::getNullValue(Builder->getDoubleTy());
            }
            else if (MethodName == "pop") {
                llvm::Value *LenField = Builder->CreateStructGEP(info.Type, info.Alloca, 1);
                llvm::Value *Len = Builder->CreateLoad(Builder->getInt32Ty(), LenField, "len");
                
                llvm::Value *NewLen = Builder->CreateSub(Len, llvm::ConstantInt::get(Builder->getInt32Ty(), 1), "new_len");
                Builder->CreateStore(NewLen, LenField);
                
                llvm::Value *PtrField = Builder->CreateStructGEP(info.Type, info.Alloca, 0);
                llvm::Value *BufferPtr = Builder->CreateLoad(Builder->getPtrTy(), PtrField, "buf_ptr");
                llvm::Value *IdxGEP = Builder->CreateGEP(info.ElementType, BufferPtr, NewLen, "pop_idx");
                return Builder->CreateLoad(info.ElementType, IdxGEP, "pop_val");
            }
            else if (MethodName == "len") {
                llvm::Value *LenField = Builder->CreateStructGEP(info.Type, info.Alloca, 1);
                return Builder->CreateLoad(Builder->getInt32Ty(), LenField, "len");
            }
            else if (MethodName == "clear") {
                llvm::Value *LenField = Builder->CreateStructGEP(info.Type, info.Alloca, 1);
                Builder->CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), 0), LenField);
                return llvm::Constant::getNullValue(Builder->getDoubleTy());
            }
        }
    }

    // 2. STRING NATIVE METHODS
    if (ObjVal->getType()->isPointerTy()) {
        if (MethodName == "len") {
            llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {ObjVal}, "len_val");
            return Builder->CreateIntCast(Len64, Builder->getInt32Ty(), false, "len_32");
        }
        else if (MethodName == "isupper") return Builder->CreateCall(getQuantaIsUpperFunc(), {ObjVal}, "isup_res");
        else if (MethodName == "islower") return Builder->CreateCall(getQuantaIsLowerFunc(), {ObjVal}, "islow_res");
        else if (MethodName == "isalpha") return Builder->CreateCall(getQuantaIsAlphaFunc(), {ObjVal}, "isalpha_res");
        else if (MethodName == "isdigit") return Builder->CreateCall(getQuantaIsDigitFunc(), {ObjVal}, "isdigit_res");
        else if (MethodName == "isspace") return Builder->CreateCall(getQuantaIsSpaceFunc(), {ObjVal}, "isspace_res");
        else if (MethodName == "isalnum") return Builder->CreateCall(getQuantaIsAlnumFunc(), {ObjVal}, "isalnum_res");

        // String Modifiers (Retain trackForAutoFree buffer)
        else if (MethodName == "upper" || MethodName == "lower" || MethodName == "reverse" ||
                 MethodName == "strip" || MethodName == "lstrip" || MethodName == "rstrip" ||
                 MethodName == "capitalize" || MethodName == "title") {
            
            llvm::Value *NewStr = nullptr;
            if (MethodName == "upper")        NewStr = Builder->CreateCall(getQuantaUpperFunc(), {ObjVal}, "up_res");
            else if (MethodName == "lower")   NewStr = Builder->CreateCall(getQuantaLowerFunc(), {ObjVal}, "low_res");
            else if (MethodName == "reverse") NewStr = Builder->CreateCall(getQuantaReverseFunc(), {ObjVal}, "rev_res");
            else if (MethodName == "strip")   NewStr = Builder->CreateCall(getQuantaStripFunc(), {ObjVal}, "strip_res");
            else if (MethodName == "lstrip")  NewStr = Builder->CreateCall(getQuantaLstripFunc(), {ObjVal}, "lstrip_res");
            else if (MethodName == "rstrip")  NewStr = Builder->CreateCall(getQuantaRstripFunc(), {ObjVal}, "rstrip_res");
            else if (MethodName == "capitalize") NewStr = Builder->CreateCall(getQuantaCapitalizeFunc(), {ObjVal}, "cap_res");
            else if (MethodName == "title")   NewStr = Builder->CreateCall(getQuantaTitleFunc(), {ObjVal}, "title_res");
            if (NewStr) trackForAutoFree(NewStr);
            return NewStr;
        }

        // 2 Argument Modifiers
        else if (MethodName == "find" || MethodName == "count" || MethodName == "startswith" || MethodName == "endswith") {
            if (Args.size() != 1) return LogErrorV((MethodName + " requires exactly 1 argument.").c_str());
            llvm::Value *SubVal = Args[0]->codegen();
            if (!SubVal || !SubVal->getType()->isPointerTy()) return LogErrorV("Expected a string argument");
            
            if (MethodName == "find")       return Builder->CreateCall(getQuantaFindFunc(), {ObjVal, SubVal}, "find_res");
            else if (MethodName == "count")      return Builder->CreateCall(getQuantaCountFunc(), {ObjVal, SubVal}, "count_res");
            else if (MethodName == "startswith") return Builder->CreateCall(getQuantaStartswithFunc(), {ObjVal, SubVal}, "startswith_res");
            else if (MethodName == "endswith")   return Builder->CreateCall(getQuantaEndswithFunc(), {ObjVal, SubVal}, "endswith_res");
        }

        // 3 Argument Modifier
        else if (MethodName == "replace") {
            if (Args.size() != 2) return LogErrorV("replace() requires exactly 2 arguments");
            llvm::Value *OldVal = Args[0]->codegen();
            llvm::Value *NewVal = Args[1]->codegen();
            if (!OldVal || !NewVal || !OldVal->getType()->isPointerTy() || !NewVal->getType()->isPointerTy()) 
                return LogErrorV("replace() requires string arguments");
            
            llvm::Value *NewStr = Builder->CreateCall(getQuantaReplaceFunc(), {ObjVal, OldVal, NewVal}, "replace_res");
            trackForAutoFree(NewStr);
            return NewStr;
        }
    }
    
    return LogErrorV(("Unsupported method '" + MethodName + "' on object").c_str());
}