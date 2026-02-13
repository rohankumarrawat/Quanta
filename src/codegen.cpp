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

#include "llvm/Pass.h"                        // Needed for 'FunctionPass'
#include "llvm/Transforms/Scalar.h"           // Needed for 'PromoteMemoryToRegister' & 'SimplifyCFG'
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include <map>
#include <iostream>
#include <vector>

// --- GLOBALS ---
extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;

// --- SYMBOL TABLE ---
struct VarInfo {
    llvm::AllocaInst* Alloca;
    llvm::Type* Type;
    std::string TypeName; 
};


static std::map<std::string, VarInfo> NamedValues;
// Global Optimizer Engine
static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

void InitializeOptimizer() {
    TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    // 1. Promote Stack to Registers (CRITICAL)
    // Your 'var x' is stored in memory (alloca). 
    // This pass moves it to a CPU register so LLVM can see that 
    // "x is the same variable" in every if-check.
    TheFPM->add(llvm::createPromoteMemoryToRegisterPass());

    // 2. Combine Instructions
    // Cleans up math (e.g., 1+2 -> 3) so comparisons are obvious.
    TheFPM->add(llvm::createInstructionCombiningPass());

    // 3. Reassociate Expressions
    // Rearranges (x + 1) + 2 -> x + (1 + 2)
    TheFPM->add(llvm::createReassociatePass());

    // 4. Simplify Control Flow (THE AUTO-SWITCH PASS)
    // This detects the "if-else chain" pattern and converts it 
    // into a single 'switch' instruction or a Jump Table.
    TheFPM->add(llvm::createCFGSimplificationPass());

    // Initialize the pipeline
    TheFPM->doInitialization();
}

// --- 1. SETUP ---
void initializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("QuantaModule", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
    NamedValues.clear(); 
}



// --- 2. PRIMITIVE VALUES ---
llvm::Value *NumberAST::codegen() {
    return llvm::ConstantInt::get(*TheContext, llvm::APInt(64, Val, true));
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
    return Builder->CreateGlobalString(val, "strtmp");
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

// --- 4. VARIABLE DECLARATION ---
// --- 4. VARIABLE DECLARATION ---
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
        TargetType = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*TheContext));
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
        // --- IMPLICIT DECLARATION (Auto-Create) ---
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
        // INT <-> INT (Handles Bool/Char/Int conversions)
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
    // A. ASSIGNMENT
    // if (Op == '=') {
    //     VariableAST *LHSE = dynamic_cast<VariableAST*>(LHS.get());
    //     if (!LHSE) return nullptr;

    //     llvm::Value *Val = RHS->codegen();
    //     if (!Val) return nullptr;

    //     llvm::AllocaInst *Alloca = nullptr;
    //     llvm::Type *TargetType = nullptr;

    //     if (NamedValues.find(LHSE->Name) == NamedValues.end()) {
    //         // Implicit Declaration
    //         TargetType = Val->getType();
    //         std::string TypeName = "int";
    //         if (TargetType->isDoubleTy()) TypeName = "float";
            
    //         llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    //         llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    //         Alloca = TmpB.CreateAlloca(TargetType, nullptr, LHSE->Name);
    //         NamedValues[LHSE->Name] = {Alloca, TargetType, TypeName};
    //     } else {
    //         VarInfo& info = NamedValues[LHSE->Name];
    //         Alloca = info.Alloca;
    //         TargetType = info.Type;
    //     }

    //     llvm::Value *FinalVal = Val;
    //     if (Val->getType() != TargetType) {
    //         if (Val->getType()->isIntegerTy() && TargetType->isIntegerTy()) {
    //             bool isBool = (Val->getType()->getIntegerBitWidth() == 1);
    //             FinalVal = Builder->CreateIntCast(Val, TargetType, !isBool, "cast");
    //         }
    //         else if (Val->getType()->isDoubleTy() && TargetType->isIntegerTy()) {
    //             FinalVal = Builder->CreateFPToSI(Val, TargetType, "fptosi");
    //         }
    //         else if (Val->getType()->isIntegerTy() && TargetType->isDoubleTy()) {
    //             FinalVal = Builder->CreateSIToFP(Val, TargetType, "sitofp");
    //         }
    //         else if (Val->getType()->isFloatingPointTy() && TargetType->isFloatingPointTy()) {
    //             FinalVal = Builder->CreateFPCast(Val, TargetType, "fpcast");
    //         }
    //     }
    //     Builder->CreateStore(FinalVal, Alloca);
    //     return FinalVal;
    // }

    // B. MATH
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R) return nullptr;

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



// --- 6. PRINT GENERATION (FIXED FOR FLOATS) ---
llvm::Value *PrintAST::codegen() {
    llvm::Value *Val = Expr->codegen();
    if (!Val) return nullptr;

    llvm::Function *PrintfFunc = TheModule->getFunction("printf");
    if (!PrintfFunc) {
        std::vector<llvm::Type*> Args;
        Args.push_back(llvm::PointerType::getUnqual(*TheContext)); 
        llvm::FunctionType *PrintfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*TheContext), Args, true);
        PrintfFunc = llvm::Function::Create(PrintfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    }

    llvm::Value *FormatStr;
    std::vector<llvm::Value*> ArgsV;
    llvm::Type *ValType = Val->getType();

    // 1. STRINGS
    if (ValType->isPointerTy()) {
        FormatStr = Builder->CreateGlobalString("%s\n", "fmt_str");
    } 
    // 2. FLOATS (32-bit Float OR 64-bit Double)
    // CRITICAL FIX: We must check 'isFloatingPointTy', not just 'isDoubleTy'
    else if (ValType->isFloatingPointTy()) {
         // Printf expects doubles. If it's a float, promote it.
         if (ValType->isFloatTy()) {
             Val = Builder->CreateFPExt(Val, llvm::Type::getDoubleTy(*TheContext), "float_to_double");
         }
         FormatStr = Builder->CreateGlobalString("%f\n", "fmt_float");
    }
    // 3. INTEGERS / CHARS / BOOLS
    else {
        // Safe because we know it is NOT a Float here.
        bool isChar = false;
        if (auto *Var = dynamic_cast<VariableAST*>(Expr.get())) {
            if (NamedValues.find(Var->Name) != NamedValues.end()) {
                if (NamedValues[Var->Name].TypeName == "char") isChar = true;
            }
        }

        if (isChar) {
            FormatStr = Builder->CreateGlobalString("%c\n", "fmt_char");
            Val = Builder->CreateZExt(Val, llvm::Type::getInt32Ty(*TheContext), "char_ext");
        } else {
            FormatStr = Builder->CreateGlobalString("%d\n", "fmt_num");
            // Check bitwidth ONLY here, where we know it's an integer
            if (ValType->getIntegerBitWidth() < 32) {
                 bool isSigned = (ValType->getIntegerBitWidth() != 1);
                 Val = Builder->CreateIntCast(Val, llvm::Type::getInt32Ty(*TheContext), isSigned, "int_promote");
            }
        }
    }

    ArgsV.push_back(FormatStr);
    ArgsV.push_back(Val);
    return Builder->CreateCall(PrintfFunc, ArgsV, "printcall");
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
    return Builder->CreateGlobalStringPtr(typeName);
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

llvm::Value *UpdateExprAST::codegen() {
    // 1. Look up Variable
    if (NamedValues.find(Name) == NamedValues.end()) {
        return LogErrorV("Unknown variable in update expression");
    }
    VarInfo& info = NamedValues[Name];
    
    // 2. Load Current Value
    llvm::Value *CurVal = Builder->CreateLoad(info.Type, info.Alloca, Name.c_str());
    
    // 3. Prepare "One" (Handle Float vs Int)
    llvm::Value *One = nullptr;
    if (CurVal->getType()->isFloatingPointTy()) {
        One = llvm::ConstantFP::get(CurVal->getType(), 1.0);
    } else {
        // Handles i1, i8, i32, i64
        One = llvm::ConstantInt::get(CurVal->getType(), 1);
    }

    // 4. Do Math (+ or -)
    llvm::Value *NewVal = nullptr;
    if (IsIncrement) {
        if (CurVal->getType()->isFloatingPointTy())
            NewVal = Builder->CreateFAdd(CurVal, One, "inc_tmp");
        else
            NewVal = Builder->CreateAdd(CurVal, One, "inc_tmp");
    } else {
        if (CurVal->getType()->isFloatingPointTy())
            NewVal = Builder->CreateFSub(CurVal, One, "dec_tmp");
        else
            NewVal = Builder->CreateSub(CurVal, One, "dec_tmp");
    }

    // 5. Save Result
    Builder->CreateStore(NewVal, info.Alloca);

    // 6. Return Value
    // Prefix (++a) returns NEW value. 
    // Postfix (a++) returns OLD value.
    return IsPrefix ? NewVal : CurVal;
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
// --- 7. FUNCTION WRAPPER ---
llvm::Function *FunctionAST::codegen() {
    // 0. Lazy Initialization: Ensure Optimizer is ready
    if (!TheFPM) {
        InitializeOptimizer();
    }

    // 1. Create Function
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());
    
    // 2. Create Entry Block
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    // 3. Generate Body Code
    for (auto &node : Body) {
        node->codegen();
    }

    // 4. Return Statement (Default 0)
    Builder->CreateRet(llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0)));

    // 5. Verify Function Logic
    if (llvm::verifyFunction(*F, &llvm::errs())) {
        std::cerr << "Error: Function verification failed.\n";
        return nullptr;
    }

    // 6. RUN THE OPTIMIZER (Auto-Switch Magic)
    // This modifies the IR in-place, converting compatible if-else chains into switches.
    TheFPM->run(*F);
    
    return F;
}

// --- 8. SAVE TO FILE ---
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
        return;
    }
    pass.run(*TheModule);
    dest.flush();
    std::cout << "[Success] Native object file 'output.o' created!" << std::endl;
}