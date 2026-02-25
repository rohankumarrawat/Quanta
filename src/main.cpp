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
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

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
        std::cerr << "\n\033[1;31m[Fatal]\033[0m Compilation failed due to type errors." << std::endl;
        return 1;
    }

    // 6. JIT-compile and run using LLVM ORC JIT
    // No external compiler needed — everything runs in-process!
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto JIT = llvm::orc::LLJITBuilder().create();
    if (!JIT) {
        llvm::errs() << "[Fatal] Failed to create JIT: " << llvm::toString(JIT.takeError()) << "\n";
        return 1;
    }

    // Add the generated IR module to the JIT
    auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext));
    if (auto Err = (*JIT)->addIRModule(std::move(TSM))) {
        llvm::errs() << "[Fatal] Failed to add module: " << llvm::toString(std::move(Err)) << "\n";
        return 1;
    }

    // Register symbols from the current process (quanta.exe itself) so the JIT
    // can find quanta_lib functions (quanta_upper, quanta_lower, printf, etc.)
    auto &MainJD = (*JIT)->getMainJITDylib();
    auto ProcessSymsGen = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
        (*JIT)->getDataLayout().getGlobalPrefix());
    if (!ProcessSymsGen) {
        llvm::errs() << "[Fatal] Failed to create process symbol generator: "
                     << llvm::toString(ProcessSymsGen.takeError()) << "\n";
        return 1;
    }
    MainJD.addGenerator(std::move(*ProcessSymsGen));

    // Explicitly register all quanta_lib symbols so they are always
    // visible to the JIT even when the linker strips dead exports.
    {
        // These functions are defined in quanta_lib.c (C linkage)
        // declared in extern "C" at the top of this file
        auto &ES = (*JIT)->getExecutionSession();
        auto EF = llvm::JITSymbolFlags::None |
                  llvm::JITSymbolFlags::Callable |
                  llvm::JITSymbolFlags::Exported;

        llvm::orc::SymbolMap libSyms;
        auto addSym = [&](const char* name, void* ptr) {
            libSyms[ES.intern(name)] = {
                llvm::orc::ExecutorAddr::fromPtr(ptr), EF
            };
        };

        addSym("quanta_panic",      (void*)quanta_panic);
        addSym("quanta_upper",      (void*)quanta_upper);
        addSym("quanta_lower",      (void*)quanta_lower);
        addSym("quanta_reverse",    (void*)quanta_reverse);
        addSym("quanta_isupper",    (void*)quanta_isupper);
        addSym("quanta_islower",    (void*)quanta_islower);
        addSym("quanta_strip",      (void*)quanta_strip);
        addSym("quanta_lstrip",     (void*)quanta_lstrip);
        addSym("quanta_rstrip",     (void*)quanta_rstrip);
        addSym("quanta_capitalize", (void*)quanta_capitalize);
        addSym("quanta_title",      (void*)quanta_title);
        addSym("quanta_isalpha",    (void*)quanta_isalpha);
        addSym("quanta_isdigit",    (void*)quanta_isdigit);
        addSym("quanta_isspace",    (void*)quanta_isspace);
        addSym("quanta_isalnum",    (void*)quanta_isalnum);
        addSym("quanta_find",       (void*)quanta_find);
        addSym("quanta_count",      (void*)quanta_count);
        addSym("quanta_startswith", (void*)quanta_startswith);
        addSym("quanta_endswith",   (void*)quanta_endswith);
        addSym("quanta_replace",    (void*)quanta_replace);
        addSym("quanta_slice",      (void*)quanta_slice);

        if (auto Err = MainJD.define(llvm::orc::absoluteSymbols(std::move(libSyms)))) {
            llvm::errs() << "[Warning] Could not register quanta_lib symbols: "
                         << llvm::toString(std::move(Err)) << "\n";
        }
    }

#ifdef _WIN32
    // On Windows/MinGW, the C runtime startup calls __main() for global constructors.
    // Our JIT-compiled code doesn't use global constructors, so we provide a no-op stub
    // to prevent the JIT from failing to resolve this symbol.
    {
        static int noopMainStub = 0; // Dummy so we have an address to use
        static auto stubFn = []() -> void {};
        llvm::orc::SymbolMap extraSyms;
        extraSyms[(*JIT)->getExecutionSession().intern("__main")] = {
            llvm::orc::ExecutorAddr::fromPtr(+stubFn),
            llvm::JITSymbolFlags::Exported
        };
        if (auto Err = MainJD.define(llvm::orc::absoluteSymbols(std::move(extraSyms)))) {
            // Non-fatal: log and continue — lookup may still succeed
            llvm::errs() << "[Warning] Could not define __main stub: " << llvm::toString(std::move(Err)) << "\n";
        }
    }
#endif

    // Look up and call the user's main() function
    auto MainSym = (*JIT)->lookup("main");
    if (!MainSym) {
        llvm::errs() << "[Fatal] Could not find 'main': " << llvm::toString(MainSym.takeError()) << "\n";
        return 1;
    }

    auto *MainFn = MainSym->toPtr<int(*)()>();
    int result = MainFn();
    fflush(stdout);  // flush buffered output (important when piped through Electron)
    fflush(stderr);
    return result;
}