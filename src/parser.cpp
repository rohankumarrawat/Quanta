#include "../include/quanta.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <set>

// --- 1. GLOBAL DEFINITIONS ---
// These MUST exist here because they were marked 'extern' in quanta.h

std::set<std::string> LoadedModules;
std::map<std::string, FunctionInfo> FunctionRegistry; // Matches your quanta.h type
std::vector<std::unique_ptr<FunctionAST>> ImportedFunctionsHook;

// --- 2. FORWARD DECLARATIONS ---
// Tells the compiler these functions exist later in the file
bool isFunctionDefinition(); 
std::unique_ptr<FunctionAST> parseFunction();
extern std::vector<Token> tokenize(std::string source); 

// --- 3. HELPER FUNCTIONS ---
bool readFile(const std::string& filename, std::string& content) {
    std::string fullPath = RootDir + filename;
    std::ifstream file(fullPath);
    
    if (!file.is_open()) {
        // Try current directory as fallback
        file.open(filename);
    }

    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

std::map<int, int> BinopPrecedence;
// Forward Declaration
std::unique_ptr<ASTNode> parseBinOpRHS(int ExprPrec, std::unique_ptr<ASTNode> LHS);
// --- STATE MANAGEMENT ---
int currentToken = 0;
std::vector<Token> globalTokens;
bool HasError = false;

std::unique_ptr<ASTNode> parseVarDecl();
std::unique_ptr<ASTNode> parsePrint();
std::vector<std::unique_ptr<ASTNode>> parseBlock();
std::unique_ptr<ASTNode> parseIfExpr();
std::unique_ptr<ASTNode> parseExpression();
std::unique_ptr<ASTNode> parseUnary();
std::unique_ptr<FunctionAST> parseFunction();


Token getTok() {
    if (currentToken < globalTokens.size()) return globalTokens[currentToken];
    int lastLine = (!globalTokens.empty()) ? globalTokens.back().line : 1;
    return {TOK_EOF, "", lastLine};
}

void advance() { currentToken++; }

// Forward Declaration


// 1. Log Error but don't exit
std::unique_ptr<ASTNode> LogError(const std::string& msg) {
    Token t = getTok();
    std::cerr << "[Quanta Error] " << msg << " at line " << t.line << std::endl;
    HasError = true;
    
    return nullptr; // Return null so the parser knows this failed
}
// --- ERROR RECOVERY ---
void synchronize() {
    // 1. Remember the line where the error happened
    int errorLine = getTok().line;

    // 2. Loop until EOF
    while (getTok().type != TOK_EOF) {
        
        // CASE A: Found a semicolon? Eat it and stop. We are synchronized.
        if (getTok().value == ";") {
            advance();
            return;
        }

        // CASE B: Did we move to a new line? Stop!
        // This handles cases where the user forgot the semicolon.
        if (getTok().line > errorLine) {
            return;
        }

        // CASE C: Did we hit a keyword that starts a new statement? Stop!
        // This is a "safe" synchronization point.
        auto t = getTok().type;
        if (t == TOK_INT || t == TOK_FLOAT || t == TOK_BOOL || 
            t == TOK_CHAR || t == TOK_STRING || t == TOK_VAR ||
            t == TOK_PRINT ) {
            return;
        }

        // Otherwise, keep skipping the bad tokens
        advance();
    }
}


// --- Get Precedence of Current Token ---
int GetTokPrecedence() {
    int type = getTok().type;
    
    // Look up in the global map defined in main.cpp
    // This works for chars ('+') and negative tokens (TOK_EQ = -15)
    int TokPrec = BinopPrecedence[type];
    
    // If not found or <= 0, it's not a binary operator
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

void parseImport() {
    advance(); // Eat 'import'

    // 1. Get Module Name
    if (getTok().type != TOK_IDENTIFIER) {
        LogError("Expected module name after 'import'");
        return;
    }
    std::string ModuleName = getTok().value;
    advance(); // Eat module name

    // 2. Check for Specific Import (e.g., .all or .functionName)
    std::string SpecificFunc = "";
    bool ImportAll = false;

    if (getTok().type == TOK_DOT) {
        advance(); // Eat '.'
        
        // Handle 'all' keyword
        if (getTok().type == TOK_ALL || getTok().value == "all") { 
            ImportAll = true;
            advance(); // Eat 'all'
        } 
        // Handle specific identifier
        else if (getTok().type == TOK_IDENTIFIER) {
            SpecificFunc = getTok().value;
            advance(); // Eat function name
        } 
        else {
            LogError("Expected 'all' or function name after '.'");
            return;
        }
    }

    // 3. Prevent Double Loading
    std::string Filename = ModuleName + ".qnt";
    
    // If already loaded, we just verify the specific function if needed
    if (LoadedModules.find(Filename) != LoadedModules.end()) {
        if (!SpecificFunc.empty()) {
             if (FunctionRegistry.find(SpecificFunc) == FunctionRegistry.end()) {
                 LogError(("Import Error: Function '" + SpecificFunc + "' not found in module '" + ModuleName + "'").c_str());
            }
        }
        return; 
    }

    // 4. Read the file content
    std::string NewSource;
    if (!readFile(Filename, NewSource)) {
        LogError(("Module not found: " + Filename).c_str());
        return;
    }

    // 5. CONTEXT SWITCH (Pause current file -> Parse new file -> Resume)
    std::vector<Token> OldTokens = globalTokens;
    int OldPos = currentToken;

    globalTokens = tokenize(NewSource);
    currentToken = 0;
    
    // Mark as loaded before parsing to handle circular imports
    LoadedModules.insert(Filename); 
    std::cout << "[Quanta] Importing " << Filename << "..." << std::endl;

    // 6. Parse the Library file
   // 6. Parse the Library
    while (getTok().type != TOK_EOF) {
        if (getTok().type == TOK_IMPORT) {
            parseImport(); 
        } 
        else if (isFunctionDefinition()) {
            if (auto Fn = parseFunction()) {
                std::string Name = Fn->getName();

                // --- SELECTIVE IMPORT FILTER ---
                // If we asked for a specific function (e.g., "add")
                // AND this function is NOT "add"...
                if (!SpecificFunc.empty() && Name != SpecificFunc) {
                    // 1. Don't generate code for it (skip Hook)
                    // 2. Remove it from the Registry so the parser doesn't think it exists
                    FunctionRegistry.erase(Name); 
                } 
                else {
                    // It matches (or we are importing everything). Keep it!
                    ImportedFunctionsHook.push_back(std::move(Fn));
                }
            }
        } 
        else {
            advance(); 
        }
    }

    // 7. Restore Context back to the original file
    globalTokens = OldTokens;
    currentToken = OldPos;

    // 8. FINAL VERIFICATION: Did the file actually contain the specific function?
    if (!SpecificFunc.empty()) {
        if (FunctionRegistry.find(SpecificFunc) == FunctionRegistry.end()) {
             LogError(("Import Error: Function '" + SpecificFunc + "' not found in module '" + ModuleName + "'").c_str());
        }
    }
}
std::unique_ptr<ASTNode> parseLoop() {
    advance(); // Eat 'loop'

    // loop i in string { body } -- index loop over string
    if (getTok().type == TOK_IDENTIFIER) {
        std::string varName = getTok().value;
        advance();
        if (getTok().type != TOK_IN) return LogError("Expected 'in' after loop variable");
        advance();
        auto strExpr = parseExpression();
        if (!strExpr) return nullptr;
        if (getTok().value != "{") return LogError("Expected '{' to start loop body");
        auto bodyStmts = parseBlock();
        auto Body = std::make_unique<BlockAST>(std::move(bodyStmts));
        return std::make_unique<LoopOverStringAST>(varName, std::move(strExpr), std::move(Body));
    }

    if (getTok().value != "(") return LogError("Expected '(' after loop or 'id in expr'");
    advance(); 

    auto Cond = parseExpression();
    if (!Cond) return nullptr;

    if (getTok().value != ")") return LogError("Expected ')' after loop condition");
    advance(); 

    if (getTok().value != "{") return LogError("Expected '{' to start loop body");
    
    auto bodyStmts = parseBlock();
    auto Body = std::make_unique<BlockAST>(std::move(bodyStmts));

    return std::make_unique<LoopAST>(std::move(Cond), std::move(Body));
}

std::unique_ptr<ASTNode> parseExpression();
// --- 1. PRIMARY PARSER ---
std::unique_ptr<ASTNode> parsePrimary() {
    Token t = getTok();

    // --- 1. STRINGS ---
    if (t.type == TOK_STRING) {
        std::string strVal = t.value; 
        advance();                    
        return std::make_unique<StringAST>(strVal);
    }

    // --- 2. NUMBERS ---
    if (t.type == TOK_NUMBER) {
        errno = 0;
        char* endPtr;
        int64_t val = std::strtoll(t.value.c_str(), &endPtr, 10);
        
        if (errno == ERANGE) {
            std::cerr << "[Quanta Error] Number literal too large, defaulting to 0.\n";
            val = 0;
        }
        
        advance(); 
        return std::make_unique<NumberAST>(val);
    }

    // --- 3. FLOATS ---
    if (t.type == TOK_FLOAT) {
        double dVal = std::stod(t.value); 
        advance();                        
        return std::make_unique<FloatAST>(dVal);
    }

    // --- 4. CHARS ---
    if (t.type == TOK_CHAR) {
        char cVal = t.value[0]; 
        advance();              
        return std::make_unique<CharAST>(cVal);
    }

    // --- 5. BOOLEANS ---
    if (t.type == TOK_TRUE) {
        advance();
        return std::make_unique<BoolAST>(true);
    }
    if (t.type == TOK_FALSE) {
        advance();
        return std::make_unique<BoolAST>(false);
    }

    // --- 6. PARENTHESES (Expression) ---
    if (t.value == "(") {
        advance();
        auto expr = parseExpression();
        if (!expr) return nullptr;
        if (getTok().value != ")") return LogError("Expected ')' after expression");
        advance();
        return expr;
    }

    // --- 6.5 ARRAYS / LISTS [a, b, c] ---
    if (t.value == "[") {
        advance(); // Eat '['
        std::vector<std::unique_ptr<ASTNode>> Elements;
        if (getTok().value != "]") {
            while (true) {
                auto arg = parseExpression();
                if (!arg) return nullptr;
                Elements.push_back(std::move(arg));
                if (getTok().value == "]") break;
                if (getTok().value != ",") return LogError("Expected ',' or ']' in array literal");
                advance(); // Eat ','
            }
        }
        advance(); // Eat ']'
        return std::make_unique<ArrayExprAST>(std::move(Elements));
    }

    // --- 7. LOOPS ---
    if (t.type == TOK_LOOP) {
        return parseLoop();
    }

    // --- 8. TYPE / BYTESIZE ---
    if (t.value == "type") {
        advance(); 
        if (getTok().value != "(") return LogError("Expected '(' after type");
        advance(); 
        if (getTok().type != TOK_IDENTIFIER) return LogError("Expected variable name inside type(...)");
        std::string varName = getTok().value;
        advance(); 
        if (getTok().value != ")") return LogError("Expected ')' after variable name");
        advance(); 
        return std::make_unique<TypeofAST>(varName);
    }

    if (t.value == "bytesize") {
        advance(); 
        if (getTok().value != "(") return LogError("Expected '(' after bytesize");
        advance(); 
        if (getTok().type != TOK_IDENTIFIER) return LogError("Expected variable name inside bytesize(...)");
        std::string varName = getTok().value;
        advance(); 
        if (getTok().value != ")") return LogError("Expected ')' after variable name");
        advance(); 
        return std::make_unique<ByteSizeAST>(varName);
    }

    

    // --- 8.5 STRING KEYWORDS (single-arg: len, upper, strip, etc.) ---
    // Standalone string keyword functions have been migrated to OOP methods
    // --- 9. IDENTIFIERS, MODULE ACCESS & CALLS ---
 if (t.type == TOK_IDENTIFIER) {
        // [CRITICAL] 1. Define IdName FIRST
        // --- Inside your variable declaration parsing logic ---
        std::string IdName = t.value; 
        advance(); // Eat Identifier Token

        // --- 2. MODULE LOGIC (test.getArea) ---
        // We evaluate if it's a module. If it's an object property access (.push()), we drop down and let Postfix handle it.
        if (getTok().type == TOK_DOT) {
            std::string ModuleName = IdName;
            std::string ExpectedFile = ModuleName + ".qnt";
            if (LoadedModules.find(ExpectedFile) != LoadedModules.end()) {
                advance(); // Eat '.'
                std::string FuncName = getTok().value;
                if (FunctionRegistry.find(FuncName) == FunctionRegistry.end()) {
                     return LogError(("Error: Function '" + FuncName + "' is not defined in module '" + ModuleName + "'.").c_str());
                }
                IdName = FuncName; // Update to function name
                advance(); // Eat function name
            }
        }

        // --- 3. FUNCTION CALL LOGIC (identifier(...)) ---
        if (getTok().value == "(") {
            
            // [NEW] GLOBAL VALIDATION CHECK
            // We check if the function exists in the Registry.
            // We skip built-in functions like "print" or "input".
            if (IdName != "print" && IdName != "input" && IdName != "delay" && 
                IdName != "type" && IdName != "bytesize") {
                
                if (FunctionRegistry.find(IdName) == FunctionRegistry.end()) {
                    return LogError(("Error: Function '" + IdName + "' is not defined. Did you mean to import it?").c_str());
                }
            }
            // ------------------------------------------------

            advance(); // Eat '('
            
            std::vector<CallArg> args; 

            if (getTok().value != ")") {
                while (true) {
                    std::string argName = "";
                    std::shared_ptr<ASTNode> argVal = nullptr;

                    // Keyword Argument Detection
                    if (getTok().type == TOK_IDENTIFIER) {
                        std::string tempName = getTok().value; 
                        advance(); 
                        
                        if (getTok().value == "=") {
                            advance(); // Eat '='
                            argName = tempName;          
                            argVal = parseExpression(); 
                        } 
                        else {
                            // Backtrack: It was just a variable like 'x'
                            auto lhs = std::make_unique<VariableAST>(tempName);
                            argVal = parseBinOpRHS(0, std::move(lhs));
                        }
                    } 
                    else {
                        argVal = parseExpression();
                    }

                    if (!argVal) return nullptr;
                    args.push_back({argName, std::move(argVal)});

                    if (getTok().value == ")") break;
                    if (getTok().value != ",") return LogError("Expected ')' or ','");
                    advance(); // Eat ','
                }
            }
            advance(); // Eat ')'
            return std::make_unique<CallAST>(IdName, std::move(args));
        }

        // --- 4. ASSIGNMENT LOGIC (identifier = ...) ---
        if (getTok().value == "=") {
            advance(); // Eat '='
            if (auto RHS = parseExpression())
                return std::make_unique<AssignmentAST>(IdName, std::move(RHS));
            return nullptr;
        }

        // --- 5. VARIABLE USAGE (identifier) ---
        return std::make_unique<VariableAST>(IdName);
    }
    
    
    
    return LogError("Unexpected token '" + t.value + "'");
}


bool isFunctionDefinition() {
    int savePos = currentToken; // Save current position
    
    // 1. Check for Type
    int t = getTok().type;
    bool hasType = (t == TOK_INT || t == TOK_VOID || t == TOK_FLOAT || 
                    t == TOK_STRING || t == TOK_BOOL);
    
    if (!hasType) return false; 
    advance(); // Eat type

    // 2. Check for Name
    if (getTok().type != TOK_IDENTIFIER) {
        currentToken = savePos; 
        return false;
    }
    advance(); // Eat name

    // 3. Check for '('
    bool isFunc = (getTok().value == "(");
    
    currentToken = savePos; // Reset position
    return isFunc;
}


std::unique_ptr<FunctionAST> parseFunction() {
    // 1. Parse Return Type
    std::string returnType = "void"; 
    if (getTok().type == TOK_INT || getTok().type == TOK_INT8 || 
        getTok().type == TOK_FLOAT || getTok().type == TOK_STRING || 
        getTok().type == TOK_VOID) {
        returnType = getTok().value;
        advance(); 
    }

    // 2. Parse Function Name
    if (getTok().type != TOK_IDENTIFIER) {
        LogError("Expected function name."); // <--- CHANGED
        return nullptr;                      // <--- CHANGED
    }
    
    std::string name = getTok().value;
    advance();

    // 3. Parse Arguments: "("
    if (getTok().value != "(") {
        LogError("Expected '(' after function name"); // <--- CHANGED
        return nullptr;                               // <--- CHANGED
    }
    advance(); 

    std::vector<FuncArg> astArgs;      
    std::vector<ArgInfo> registryArgs; 

    if (getTok().value != ")") {
        while (true) {
            std::string argType = "int"; // Default to int if unknown
            bool typeSpecified = false;  
            std::string argName = "";
            std::shared_ptr<ASTNode> defaultVal = nullptr;

            // 1. Check for Explicit Type (e.g., "string name")
            if (getTok().value == "int" || getTok().value == "int8" || 
                getTok().value == "float" || getTok().value == "string") {
                argType = getTok().value;
                typeSpecified = true; 
                advance(); 
            }

            // 2. Parse Argument Name
            if (getTok().type != TOK_IDENTIFIER) {
                LogError("Expected argument name.");
                return nullptr;
            }
            argName = getTok().value;
            advance(); 

            // 3. Parse Default Value & INFER TYPE
            if (getTok().value == "=") {
                advance(); 
                defaultVal = parseExpression(); 
                if (!defaultVal) return nullptr;

                // --- TYPE INFERENCE LOGIC ---
                if (!typeSpecified) {
                    // Check if default value is a String Literal
                    // FIX: Use 'StringAST' (your class name) instead of 'StringExprAST'
                    if (dynamic_cast<StringAST*>(defaultVal.get())) {
                        argType = "string";
                    }
                    // Optional: Check for floats vs ints here if needed
                }
            }

            // 4. Store Argument
            astArgs.push_back({argType, argName});
            registryArgs.push_back({argName, argType, defaultVal});

            if (getTok().value == ")") break;
            
            if (getTok().value != ",") {
                LogError("Expected ',' or ')' in argument list");
                return nullptr;
            }
            advance(); 
        }
    
    }
    advance(); 

    FunctionRegistry[name] = {name, registryArgs};

    // 5. Parse Body
    if (getTok().value != "{") {
        LogError("Expected '{' to start function body"); // <--- CHANGED
        return nullptr;                                  // <--- CHANGED
    }
    
    auto body = parseBlock(); 

    return std::make_unique<FunctionAST>(
        returnType, 
        name, 
        std::move(astArgs), 
        std::move(body)
    );
}


// --- HELPER 2: Parse a Single Statement ---
// Unifies logic for Blocks and Top-Level Scripts
std::unique_ptr<ASTNode> parseStatement() {
    int t = getTok().type;

    if (t == TOK_INT || t == TOK_FLOAT || t == TOK_BOOL || 
        t == TOK_STRING || t == TOK_VAR) {
        return parseVarDecl();
    } 
    else if (t == TOK_PRINT) {
        return parsePrint();
    } 
    else if (t == TOK_IF) {
        return parseIfExpr();
    } 
   else if (t == TOK_RETURN) {
    int line = getTok().line;
        advance(); // Eat 'return'
        
        std::unique_ptr<ASTNode> expr = nullptr;
        
        // 1. Parse the return value (if it exists)
        if (getTok().value != ";") {
            expr = parseExpression();
            if (!expr) return nullptr; // Safety check: if parsing failed, stop here
        }

        // 2. Consume the semicolon
        if (getTok().value == ";") {
            advance(); // Eat ';'
        } 

        // [CRITICAL FIX] 
        // Wrap the expression in ReturnAST. 
        // This ensures Codegen calls Builder.CreateRet() instead of just evaluating the number.
        return std::make_unique<ReturnAST>(std::move(expr),line); 
    }
    else {
        return parseExpression();
    }
}

// --- Parse Block { ... } ---
std::vector<std::unique_ptr<ASTNode>> parseBlock() {
    if (getTok().value != "{") {
        LogError("Expected '{' to start block");
        return {};
    }
    advance(); // Eat '{'

    std::vector<std::unique_ptr<ASTNode>> stmts;

    while (getTok().type != TOK_EOF && getTok().value != "}") {
        auto stmt = parseStatement(); // Use the new helper
        if (stmt) {
            stmts.push_back(std::move(stmt));
            if (getTok().value == ";") advance(); // Eat optional ';'
        } else {
            synchronize();
        }
    }

    if (getTok().value == "}") advance(); // Eat '}'
    return stmts;
}
// --- Parse If / Elif / Else ---
std::unique_ptr<ASTNode> parseIfExpr() {
    advance(); // Eat 'if'

    // 1. Condition
    auto Cond = parseExpression();
    if (!Cond) return nullptr;

    // 2. Then Block
    if (getTok().value != "{") return LogError("Expected '{' after if condition");
    
    // [FIX] Wrap the vector in BlockAST
    auto thenStmts = parseBlock(); 
    auto Then = std::make_unique<BlockAST>(std::move(thenStmts));

    // 3. Else / Elif Logic
    std::unique_ptr<ASTNode> Else = nullptr;
    
    if (getTok().type == TOK_ELIF) {
        Else = parseIfExpr(); // Recursively parse 'elif'
    } 
    else if (getTok().type == TOK_ELSE) {
        advance(); // Eat 'else'
        
        if (getTok().type == TOK_IF) {
            Else = parseIfExpr(); // "else if"
        } else {
            if (getTok().value != "{") return LogError("Expected '{' after else");
            
            // [FIX] Wrap the else block vector in BlockAST
            auto elseStmts = parseBlock();
            Else = std::make_unique<BlockAST>(std::move(elseStmts));
        }
    }

    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}

std::unique_ptr<ASTNode> parseVarDecl() {
    Token typeTok = getTok();
    advance(); 

   
    int capacity = 0; 
    bool isDynamicList = false;
    bool isFixedArray = false;
    bool isFixedString = false;
    
    // Check if the very next token is "["
    if (getTok().value == "[") {
        advance(); // Eat '['
        if (getTok().value == "]") {
            isDynamicList = true;
            advance(); // Eat ']'
        } else if (getTok().type == TOK_NUMBER) {
            capacity = std::stoi(getTok().value);
            if (typeTok.type == TOK_STRING) {
                isFixedString = true;
            } else {
                isFixedArray = true;
            }
            advance(); // Eat the number
            if (getTok().value != "]") return LogError("Expected ']' after capacity");
            advance(); // Eat ']'
        } else {
            return LogError("Expected number or ']' after '[' in type declaration");
        }
    }
    // ==========================================================
    
    // 1. Expect Name
    if (getTok().type != TOK_IDENTIFIER) return LogError("Expected variable name");
    std::string name = getTok().value;
    advance(); // Eat Name

    // 2. Expect '='
    if (getTok().value != "=") return LogError("Expected '=' in declaration");
    advance(); // Eat '='

    // 3. Parse Value
    auto init = parseExpression();
    if (!init) return nullptr;

    // 4. DETERMINE TYPE & SIZE
    // We calculate this AFTER parsing the value so we can support 'var'
    std::string typeStr = typeTok.value;
    int bytes = 4; // Default

    // CASE A: Auto-Detect (var)
    if (typeTok.type == TOK_VAR) {
        if (dynamic_cast<StringAST*>(init.get())) {
            typeStr = "string";
            bytes = 8;
        }
        else if (dynamic_cast<FloatAST*>(init.get())) {
            typeStr = "float";
            bytes = 8;
        }
        else if (dynamic_cast<BoolAST*>(init.get())) {
            typeStr = "bool";
            bytes = 1;
        }
        else if (dynamic_cast<CharAST*>(init.get())) {
            typeStr = "char";
            bytes = 1;
        }
        else {
            typeStr = "int"; // Default to int for numbers
            bytes = 4;
        }
    } 
    // CASE B: Explicit Types
    else if (typeTok.type == TOK_BOOL || typeTok.type == TOK_CHAR) {
        bytes = 1;
    } 
    else if (typeTok.type == TOK_STRING) { 
        // Note: Use TOK_TYPE_STRING (the keyword), not TOK_STRING (the value "hello")
        typeStr = "string"; 
        bytes = 8;
    }
    else if (typeTok.type == TOK_INT) {
        if (typeTok.value == "int") {
            bytes = 4;
        } else {
            // Get the number part (e.g., "int9" -> "9")
            std::string numStr = typeTok.value.substr(3);
            
            // USE std::atoi INSTEAD OF std::stoi (No exceptions!)
            bytes = std::atoi(numStr.c_str());

            // 1. Check for garbage input
            if (bytes == 0) {
                 return LogError("Invalid integer size format.");
            }

            // 2. Check Limit (Max 8 bytes)
            if (bytes > 8) {
                return LogError("Integer size cannot exceed 8 bytes (64-bit). You requested: " + std::to_string(bytes) + " bytes.");
            }
        }
    }
 else if (typeTok.type == TOK_FLOAT) {
        if (typeTok.value == "float") {
            bytes = 4; // Default to 4 bytes (Standard 32-bit float)
        } else {
            // Handle custom sizes like "float8", "float5"
            // "float" is 5 letters, so substring starts at index 5
            std::string numStr = typeTok.value.substr(5); 
            bytes = std::atoi(numStr.c_str());

            // 1. Safety Check: Garbage input
            if (bytes == 0) return LogError("Invalid float size.");

            // 2. Minimum Size Check: Floats need at least 4 bytes (32 bits) to work
            if (bytes < 4) {
                 return LogError("Float size must be at least 4 bytes (float4).");
            }

            // 3. Limit Check: Max 8 bytes (64-bit Double)
            // We allow 5, 6, 7 because Codegen will safely 'promote' them to 8.
            if (bytes > 8) {
                return LogError("Float size cannot exceed 8 bytes (64-bit).");
            }
        }
    }

    if (isFixedString) {
        return std::make_unique<FixedStringDeclAST>(name, capacity, std::move(init));
    }
    if (isDynamicList) {
        return std::make_unique<DynamicListDeclAST>(name, typeStr, std::move(init));
    }
    if (isFixedArray) {
        return std::make_unique<FixedArrayDeclAST>(name, typeStr, capacity, std::move(init));
    }
    return std::make_unique<VarDeclAST>(name, typeStr, bytes, std::move(init));
}

std::unique_ptr<ASTNode> parseBinOpRHS(int ExprPrec, std::unique_ptr<ASTNode> LHS) {
    while (true) {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        // Example: if we have "a + b" and see ";", prec is -1, so we return.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = getTok().type; // Store the operator (e.g., TOK_EQ or '+')
        advance(); // Eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = parseUnary();
        if (!RHS) return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) return nullptr;
        }

        // Merge LHS/RHS. 
        // using 'int' for BinOp ensures TOK_EQ works.
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

// --- 1. POSTFIX PARSER (Highest Priority: i++, s[i], s[a:b]) ---
std::unique_ptr<ASTNode> parsePostfix() {
    auto LHS = parsePrimary();
    if (!LHS) return nullptr;

    // String index s[i] or slice s[start:end] or s[start:end:step]
    while (getTok().value == "[") {
        advance(); // Eat '['
        auto first = parseExpression();
        if (!first) return nullptr;
        if (getTok().value == "]") {
            advance();
            
            // --- NEW: Array/String Index Assignment arr[i] = x ---
            if (getTok().value == "=") {
                advance(); // Eat '='
                auto RHS = parseExpression();
                if (!RHS) return nullptr;
                LHS = std::make_unique<IndexAssignAST>(std::move(LHS), std::move(first), std::move(RHS));
                continue;
            }
            
            LHS = std::make_unique<StringIndexAST>(std::move(LHS), std::move(first));
            continue;
        }
        if (getTok().value != ":") return LogError("Expected ']' or ':' in subscript");
        advance();
        auto endExpr = parseExpression();
        if (!endExpr) return nullptr;
        if (getTok().value == "]") {
            advance();
            LHS = std::make_unique<StringSliceAST>(std::move(LHS), std::move(first), std::move(endExpr), nullptr);
            continue;
        }
        if (getTok().value != ":") return LogError("Expected ']' or ':' after slice end");
        advance();
        auto stepExpr = parseExpression();
        if (!stepExpr) return nullptr;
        if (getTok().value != "]") return LogError("Expected ']' after slice step");
        advance();
        LHS = std::make_unique<StringSliceAST>(std::move(LHS), std::move(first), std::move(endExpr), std::move(stepExpr));
    }

    // Check for operator immediately after variable
    while (getTok().type == TOK_INC || getTok().type == TOK_DEC || getTok().type == TOK_DOT) {
        if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
            auto *Var = dynamic_cast<VariableAST*>(LHS.get());
            if (!Var) return LogError("Operand of ++/-- must be a variable");
            bool isInc = (getTok().type == TOK_INC);
            advance();
            LHS = std::make_unique<UpdateExprAST>(Var->Name, isInc, false);
            continue;
        }

        if (getTok().type == TOK_DOT) {
            advance(); // Eat '.'
            
            // Allow native property/method names, plus string keywords that have been hijacked
            if (getTok().type != TOK_IDENTIFIER && 
                getTok().type != TOK_LEN && getTok().type != TOK_UPPER &&
                getTok().type != TOK_LOWER && getTok().type != TOK_REVERSE &&
                getTok().type != TOK_STRIP && getTok().type != TOK_REPLACE &&
                getTok().type != TOK_FIND && getTok().type != TOK_COUNT &&
                getTok().type != TOK_STARTSWITH && getTok().type != TOK_ENDSWITH &&
                getTok().type != TOK_ISUPPER && getTok().type != TOK_ISLOWER &&
                getTok().type != TOK_ISALPHA && getTok().type != TOK_ISDIGIT &&
                getTok().type != TOK_ISSPACE && getTok().type != TOK_ISALNUM &&
                getTok().type != TOK_CAPITALIZE && getTok().type != TOK_TITLE &&
                getTok().type != TOK_LSTRIP && getTok().type != TOK_RSTRIP) {
                return LogError(("Expected method name after '.', got: " + getTok().value).c_str());
            }
            
            std::string MethodName = getTok().value;
            advance(); // Eat method name

            if (getTok().value != "(") return LogError("Expected '(' after method name");
            advance(); // Eat '('

            std::vector<std::unique_ptr<ASTNode>> args;
            if (getTok().value != ")") {
                while (true) {
                    auto arg = parseExpression();
                    if (!arg) return nullptr;
                    args.push_back(std::move(arg));
                    if (getTok().value == ")") break;
                    if (getTok().value != ",") return LogError("Expected ',' in method args");
                    advance(); // eat ','
                }
            }
            advance(); // Eat ')'
            
            LHS = std::make_unique<MethodCallAST>(std::move(LHS), MethodName, std::move(args));
            continue;
        }
    }
    
    return LHS;
}

// --- 2. PREFIX PARSER (Medium Priority: ++i, unary -) ---
std::unique_ptr<ASTNode> parseUnary() {
    // Unary minus (e.g. -1, -i for negative indexing/slicing)
    if (getTok().value == "-" && getTok().type == (int)'-') {
        advance();
        auto Operand = parseUnary();
        if (!Operand) return nullptr;
        return std::make_unique<BinaryExprAST>('-', std::make_unique<NumberAST>(0), std::move(Operand));
    }
    // Check for operator BEFORE variable
    if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
        bool isInc = (getTok().type == TOK_INC);
        advance(); // Eat '++'
        
        auto Operand = parseUnary(); 
        if (!Operand) return nullptr;

        auto *Var = dynamic_cast<VariableAST*>(Operand.get());
        if (!Var) return LogError("Operand of ++/-- must be a variable");

        return std::make_unique<UpdateExprAST>(Var->Name, isInc, true);
    }
    
    return parsePostfix();
}

std::unique_ptr<ASTNode> parseExpression() {
    auto LHS = parseUnary();
    if (!LHS) return nullptr;
    return parseBinOpRHS(0, std::move(LHS));
}
std::unique_ptr<ASTNode> parsePrint() {
    advance(); // Eat 'print'
    
    if (getTok().value != "(") {
        return LogError("Expected '(' after print");
    }
    advance(); // Eat '('

    std::vector<std::unique_ptr<ASTNode>> args;

    // Check if there are any arguments (handle empty print())
    if (getTok().value != ")") {
        while (true) {
            // Using parseExpression() allows keywords like len() or upper()
            auto arg = parseExpression();
            if (!arg) return nullptr;
            
            args.push_back(std::move(arg));

            // If we see ')', we are done with the argument list
            if (getTok().value == ")") {
                break;
            }

            // If it's not a ')', it MUST be a comma to separate arguments
            if (getTok().value != ",") {
                return LogError("Expected ',' or ')' in print arguments");
            }
            advance(); // Eat ','
        }
    }

    advance(); // Eat ')'
    
    // Check for trailing semicolon (optional depending on your grammar)
    if (getTok().value == ";") {
        advance();
    }

    return std::make_unique<PrintAST>(std::move(args));
}


ProgramAST parse(const std::vector<Token>& tokens) {
    globalTokens = tokens;
    currentToken = 0;
    HasError = false;
    ImportedFunctionsHook.clear();

    ProgramAST program;
    std::vector<std::unique_ptr<ASTNode>> scriptBody; // Buffer for script code

    while (getTok().type != TOK_EOF) {

        if (getTok().type == TOK_IMPORT) {
            parseImport();
            continue;
        }
        
        // CASE A: It looks like a Function (int main() ...)
        if (isFunctionDefinition()) {
            auto func = parseFunction();
            if (func) program.functions.push_back(std::move(func));
        } 
        
        // CASE B: It looks like a Script Statement (print("hi");)
        else {
            auto stmt = parseStatement();
            if (stmt) {
                scriptBody.push_back(std::move(stmt));
                if (getTok().value == ";") advance();
            } else {
                advance(); // Skip errors
            }
        }
    }

    for (auto& Fn : ImportedFunctionsHook) {
        program.functions.push_back(std::move(Fn));
    }
    ImportedFunctionsHook.clear(); // Clean up
    // =========================================================

    // --- AUTO-MAIN LOGIC ---
    
    // 1. Check if user wrote their own main
    bool hasExplicitMain = false;
    for (const auto& func : program.functions) {
        if (func->getName() == "main") {
            hasExplicitMain = true;
            break;
        }
    }

    // 2. Decide what to do with script code
   // 2. Wrap Script Code into 'main'
    if (!scriptBody.empty()) {
        if (hasExplicitMain) {
            std::cerr << "Error: Cannot mix top-level script code with an explicit 'main' function." << std::endl;
            HasError = true;
        } else {
            // Auto-generate: void main() { ... scriptBody ... }
            program.functions.push_back(std::make_unique<FunctionAST>(
                "void", 
                "main", 
                std::vector<FuncArg>(),  // <--- CHANGED THIS
                std::move(scriptBody)
            ));
        }
    } 
    // 3. Handle Empty File (prevent linker error)
    else if (!hasExplicitMain) {
         program.functions.push_back(std::make_unique<FunctionAST>(
            "void", "main", 
            std::vector<FuncArg>(),      // <--- CHANGED THIS
            std::vector<std::unique_ptr<ASTNode>>()
        ));
    }

    return program;
}