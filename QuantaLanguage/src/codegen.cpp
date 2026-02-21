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
    llvm::AllocaInst *Alloca;
    llvm::Type *Type;
    std::string TypeName;
};

// Global Variable Registry
static std::map<std::string, VarInfo> NamedValues;
static std::map<std::string, llvm::Value*> StringPool;
// static std::vector<llvm::Value*> AutoFreeList;

// --- AUTO-FREE MEMORY TRACKER ---
static std::map<llvm::Function*, std::vector<llvm::AllocaInst*>> AutoFreeMap;
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
    } else if (ReturnType.find("int") == 0) {
        int bits = 32;
        if (ReturnType.size() > 3) {
            // FIX: Removed '* 8' so int8 is 8 bits, not 64 bits
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
    return Builder->CreateLoad(info.Type, info.Alloca, Name.c_str());
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
        if (oldInfo.Alloca->getAllocatedType()->getPrimitiveSizeInBits() >= TargetType->getPrimitiveSizeInBits()) {
            Alloca = oldInfo.Alloca;
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
    NamedValues[Name] = {Alloca, TargetType, Type}; 
    return FinalVal;
 
}

llvm::Value *AssignmentAST::codegen() {
    // 1. Generate the value (RHS)
    llvm::Value *Val = RHS->codegen();
    if (!Val) return nullptr;

    llvm::AllocaInst *Alloca = nullptr;
    llvm::Type *TargetType = nullptr;

    // 2. Check if variable exists
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
        case '/': 
            if (auto *CR = llvm::dyn_cast<llvm::ConstantFP>(R)) {
                if (CR->getValueAPF().isZero()) { std::cerr << "Error: Div by Zero\n"; return nullptr; }
            }
            if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(R)) {
                if (CI->isZero()) { std::cerr << "Error: Div by Zero\n"; return nullptr; }
            }
            return isFloat ? Builder->CreateFDiv(L, R, "div") : Builder->CreateSDiv(L, R, "div");
        case '<':
            L = isFloat ? Builder->CreateFCmpOLT(L, R, "cmp") : Builder->CreateICmpSLT(L, R, "cmp");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
        case '>':
            L = isFloat ? Builder->CreateFCmpOGT(L, R, "cmp") : Builder->CreateICmpSGT(L, R, "cmp");
            return Builder->CreateZExt(L, llvm::Type::getInt32Ty(*TheContext), "bool_int");
        // ... existing cases (+, -, *, /) ...

        case '%': {
        // 1. If both are Integers, use Integer Remainder (SRem)
        if (L->getType()->isIntegerTy() && R->getType()->isIntegerTy()) {
            return Builder->CreateSRem(L, R, "remtmp");
        }
        
        // 2. If both are Floating Point, use Float Remainder (FRem)
        if (L->getType()->isFloatingPointTy() && R->getType()->isFloatingPointTy()) {
            return Builder->CreateFRem(L, R, "remtmp");
        }

        // 3. Fallback: If types are mixed (e.g. 5 % 2.5), ensure both are floats
        // (Assuming you have a helper to cast mixed types, or just error out)
        return LogErrorV("Modulo requires both operands to be Integers or both Doubles.");
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
        default: return nullptr;
    }
}


llvm::Value *UpdateExprAST::codegen() {
    // 1. Find variable address
    if (NamedValues.find(Name) == NamedValues.end())
        return LogErrorV("Unknown variable in update expression");
    
    llvm::AllocaInst *V = NamedValues[Name].Alloca; // Ensure you use .Alloca

    // 2. Load current value
    llvm::Value *CurVal = Builder->CreateLoad(V->getAllocatedType(), V, Name.c_str());

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


// --- 8. SAVE TO FILE ---


void generateObjectCode() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    std::string TargetTriple = llvm::sys::getDefaultTargetTriple();
    // std::string TargetTriple = "arm64-apple-macosx15.0.0";
    // TheModule->setTargetTriple(TargetTriple);
    
    
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
        return;
    }
    pass.run(*TheModule);
    dest.flush();
    std::cout << "[Success] Native object file 'output.o' created!" << std::endl;
}

llvm::Value *LoopAST::codegen() {
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

    // Jump back to the start (Condition)
    Builder->CreateBr(CondBB);

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
    return Builder->CreateCall(CalleeF, FinalArgs, "calltmp");
}

// --- Generate Code for String Keywords ---
llvm::Value *StringOpAST::codegen() {
    // 1. Generate the string value
    llvm::Value *StrVal = Operand->codegen();
    if (!StrVal) return nullptr;

    // Safety check: Make sure the user actually passed a string (pointer)
    if (!StrVal->getType()->isPointerTy()) {
        return LogErrorV("Error: String operations require a string argument.");
    }

    // 2. Handle the specific operation
    switch (OpType) {
        
        case TOK_LEN: {
            // Call strlen()
            llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {StrVal}, "len_val");
            // Convert to 32-bit int for Quanta
            return Builder->CreateIntCast(Len64, Builder->getInt32Ty(), false, "len_32");
        }
        
        case TOK_ISUPPER:
            return Builder->CreateCall(getQuantaIsUpperFunc(), {StrVal}, "isup_res");
            
        case TOK_ISLOWER:
            return Builder->CreateCall(getQuantaIsLowerFunc(), {StrVal}, "islow_res");

        // --- OPERATIONS THAT RETURN A NEW HEAP STRING ---
        case TOK_UPPER:
        case TOK_LOWER:
        case TOK_REVERSE:
        case TOK_STRIP:
        case TOK_LSTRIP:
        case TOK_RSTRIP:
        case TOK_CAPITALIZE:
        case TOK_TITLE: {
            llvm::Value *NewStr = nullptr;
            if (OpType == TOK_UPPER)        NewStr = Builder->CreateCall(getQuantaUpperFunc(), {StrVal}, "up_res");
            else if (OpType == TOK_LOWER)   NewStr = Builder->CreateCall(getQuantaLowerFunc(), {StrVal}, "low_res");
            else if (OpType == TOK_REVERSE) NewStr = Builder->CreateCall(getQuantaReverseFunc(), {StrVal}, "rev_res");
            else if (OpType == TOK_STRIP)   NewStr = Builder->CreateCall(getQuantaStripFunc(), {StrVal}, "strip_res");
            else if (OpType == TOK_LSTRIP)  NewStr = Builder->CreateCall(getQuantaLstripFunc(), {StrVal}, "lstrip_res");
            else if (OpType == TOK_RSTRIP)  NewStr = Builder->CreateCall(getQuantaRstripFunc(), {StrVal}, "rstrip_res");
            else if (OpType == TOK_CAPITALIZE) NewStr = Builder->CreateCall(getQuantaCapitalizeFunc(), {StrVal}, "cap_res");
            else if (OpType == TOK_TITLE)   NewStr = Builder->CreateCall(getQuantaTitleFunc(), {StrVal}, "title_res");
            trackForAutoFree(NewStr);
            return NewStr;
        }

        case TOK_ISALPHA: return Builder->CreateCall(getQuantaIsAlphaFunc(), {StrVal}, "isalpha_res");
        case TOK_ISDIGIT: return Builder->CreateCall(getQuantaIsDigitFunc(), {StrVal}, "isdigit_res");
        case TOK_ISSPACE: return Builder->CreateCall(getQuantaIsSpaceFunc(), {StrVal}, "isspace_res");
        case TOK_ISALNUM: return Builder->CreateCall(getQuantaIsAlnumFunc(), {StrVal}, "isalnum_res");

        default:
            return LogErrorV("Unknown string operation.");
    }
}

llvm::Value *StringOp2AST::codegen() {
    llvm::Value *StrVal = Operand1->codegen();
    llvm::Value *SubVal = Operand2->codegen();
    if (!StrVal || !SubVal) return nullptr;
    if (!StrVal->getType()->isPointerTy() || !SubVal->getType()->isPointerTy()) {
        return LogErrorV("Error: Two-arg string operations require two string arguments.");
    }
    switch (OpType) {
        case TOK_FIND:        return Builder->CreateCall(getQuantaFindFunc(), {StrVal, SubVal}, "find_res");
        case TOK_COUNT:       return Builder->CreateCall(getQuantaCountFunc(), {StrVal, SubVal}, "count_res");
        case TOK_STARTSWITH:  return Builder->CreateCall(getQuantaStartswithFunc(), {StrVal, SubVal}, "startswith_res");
        case TOK_ENDSWITH:    return Builder->CreateCall(getQuantaEndswithFunc(), {StrVal, SubVal}, "endswith_res");
        default: return LogErrorV("Unknown two-arg string operation.");
    }
}

llvm::Value *StringOp3AST::codegen() {
    if (OpType != TOK_REPLACE) return LogErrorV("Unknown three-arg string operation.");
    llvm::Value *StrVal = Operand1->codegen();
    llvm::Value *OldVal = Operand2->codegen();
    llvm::Value *NewVal = Operand3->codegen();
    if (!StrVal || !OldVal || !NewVal) return nullptr;
    if (!StrVal->getType()->isPointerTy() || !OldVal->getType()->isPointerTy() || !NewVal->getType()->isPointerTy()) {
        return LogErrorV("Error: replace(s, old, new) requires three string arguments.");
    }
    llvm::Value *NewStr = Builder->CreateCall(getQuantaReplaceFunc(), {StrVal, OldVal, NewVal}, "replace_res");
    trackForAutoFree(NewStr);
    return NewStr;
}

llvm::Value *StringIndexAST::codegen() {
    llvm::Value *Base = BaseExpr->codegen();
    llvm::Value *Index = IndexExpr->codegen();
    if (!Base || !Index) return nullptr;
    if (!Base->getType()->isPointerTy()) return LogErrorV("String index base must be a string (pointer).");
    if (!Index->getType()->isIntegerTy()) return LogErrorV("String index must be an integer.");
    llvm::Value *Idx64 = Index->getType()->getIntegerBitWidth() < 64
        ? Builder->CreateSExt(Index, Builder->getInt64Ty(), "idx64")
        : (Index->getType()->getIntegerBitWidth() > 64 ? Builder->CreateTrunc(Index, Builder->getInt64Ty(), "idx64") : Index);
    /* Negative index: -1 = last char, use index + len */
    llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {Base}, "idx_len");
    llvm::Value *Zero64 = llvm::ConstantInt::get(Builder->getInt64Ty(), 0);
    llvm::Value *IsNeg = Builder->CreateICmpSLT(Idx64, Zero64, "idx_neg");
    llvm::Value *Adjusted64 = Builder->CreateSelect(IsNeg, Builder->CreateAdd(Idx64, Len64, "idx_adj"), Idx64, "idx_final");
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

llvm::Value *LoopOverStringAST::codegen() {
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    llvm::Value *StrVal = StringExpr->codegen();
    if (!StrVal || !StrVal->getType()->isPointerTy()) return LogErrorV("Loop over string requires a string expression.");
    llvm::Value *Len64 = Builder->CreateCall(getStrlenFunc(), {StrVal}, "str_len");
    llvm::Value *Len32 = Builder->CreateTrunc(Len64, Builder->getInt32Ty(), "len32");

    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(*TheContext, "loop_str_cond", TheFunction);
    llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(*TheContext, "loop_str_body");
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*TheContext, "loop_str_after");

    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    llvm::AllocaInst *VarAlloca = TmpB.CreateAlloca(Builder->getInt32Ty(), nullptr, VarName);
    TmpB.CreateStore(llvm::ConstantInt::get(Builder->getInt32Ty(), 0), VarAlloca);

    Builder->CreateBr(CondBB);

    Builder->SetInsertPoint(CondBB);
    llvm::Value *Cur = Builder->CreateLoad(Builder->getInt32Ty(), VarAlloca, "cur_i");
    llvm::Value *CondV = Builder->CreateICmpSLT(Cur, Len32, "loop_str_cmp");
    Builder->CreateCondBr(CondV, BodyBB, AfterBB);

    TheFunction->insert(TheFunction->end(), BodyBB);
    Builder->SetInsertPoint(BodyBB);
    NamedValues[VarName] = VarInfo{VarAlloca, Builder->getInt32Ty()};
    if (!Body->codegen()) return nullptr;
    llvm::Value *Next = Builder->CreateAdd(Cur, llvm::ConstantInt::get(Builder->getInt32Ty(), 1), "next_i");
    Builder->CreateStore(Next, VarAlloca);
    Builder->CreateBr(CondBB);

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
    
    NamedValues[VarName] = VarInfo{VarAlloca, Builder->getPtrTy()};

    return BufferPtr;
}