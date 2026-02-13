#include "../include/quanta.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>
std::map<int, int> BinopPrecedence;
// --- STATE MANAGEMENT ---
int currentToken = 0;
std::vector<Token> globalTokens;
bool HasError = false;

std::unique_ptr<ASTNode> parseVarDecl();
std::unique_ptr<ASTNode> parsePrint();
std::unique_ptr<ASTNode> parseBlock();
std::unique_ptr<ASTNode> parseIfExpr();
std::unique_ptr<ASTNode> parseExpression();
std::unique_ptr<ASTNode> parseUnary();
std::unique_ptr<ASTNode> parsePostfix();
std::unique_ptr<ASTNode> parseLoop();

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

std::unique_ptr<ASTNode> parseExpression();
// --- 1. PRIMARY PARSER ---
std::unique_ptr<ASTNode> parsePrimary() {
    Token t = getTok();
  

    if (t.type == TOK_STRING) {
        advance();
        return std::make_unique<StringAST>(t.value);
    }
   if (t.type == TOK_NUMBER) {
        advance();
        
        // --- 64-BIT SAFE PARSING (No Exceptions) ---
        errno = 0; // Reset error flag
        char* endPtr;
        
        // Parse 64-bit integer (Base 10) using C standard library
        int64_t val = std::strtoll(t.value.c_str(), &endPtr, 10);

        // Check for Overflow (If number is bigger than 9,223,372,036,854,775,807)
        if (errno == ERANGE) {
            std::cerr << "[Quanta Error] Number literal is too large." << std::endl;
            std::cerr << "  Value '" << t.value << "' exceeds 64-bit limit." << std::endl;
            std::cerr << "  Defaulting to 0 to continue parsing.\n" << std::endl;
            val = 0; // Safe fallback so compiler doesn't abort
        }
        
        return std::make_unique<NumberAST>(val);
    }

      // [NEW] Handle type(var)
    if (t.value == "type") {
        advance(); // Eat 'type' keyword
        
        if (getTok().value != "(") return LogError("Expected '(' after type");
        advance(); // Eat '('
        
        if (getTok().type != TOK_IDENTIFIER) return LogError("Expected variable name inside type(...)");
        std::string varName = getTok().value;
        advance(); // Eat Variable Name
        
        if (getTok().value != ")") return LogError("Expected ')' after variable name");
        advance(); // Eat ')'
        
        return std::make_unique<TypeofAST>(varName);
    }

    if (t.value == "bytesize") {
        advance(); // Eat 'bytesize'
        
        if (getTok().value != "(") return LogError("Expected '(' after bytesize");
        advance(); // Eat '('
        
        if (getTok().type != TOK_IDENTIFIER) return LogError("Expected variable name inside bytesize(...)");
        std::string varName = getTok().value;
        advance(); // Eat Variable Name
        
        if (getTok().value != ")") return LogError("Expected ')' after variable name");
        advance(); // Eat ')'
        
        return std::make_unique<ByteSizeAST>(varName);
    }
    // if (t.type == TOK_IDENTIFIER) {
    //     advance();
        
    //     return std::make_unique<VariableAST>(t.value);
    // }

    // --- 5. IDENTIFIERS & ASSIGNMENT ---
    if (t.type == TOK_IDENTIFIER) {
        std::string IdName = t.value;
        advance(); // Eat the identifier (e.g., "a")

        // [CRITICAL CHECK]
        // Is the very next token an '='?
        if (getTok().value == "=") {
            advance(); // Eat '='
            
            // It is an assignment (a = 1)! Parse the value.
            auto RHS = parseExpression();
            if (!RHS) return nullptr;

            return std::make_unique<AssignmentAST>(IdName, std::move(RHS));
        }

        // No '='? Then it is just a normal variable read (e.g. print(a))
        return std::make_unique<VariableAST>(IdName);
    }

    // --- Inside parsePrimary() in src/parser.cpp ---

  
    if (t.type == TOK_FLOAT) {
        advance();
        return std::make_unique<FloatAST>(std::stod(t.value));
    }

    if (t.type == TOK_TRUE) {
        advance();
        return std::make_unique<BoolAST>(true);
    }
    if (t.type == TOK_FALSE) {
        advance();
        return std::make_unique<BoolAST>(false);
    }
    if (t.type == TOK_CHAR) {
        advance(); // t.value is a string like "A", we want char 'A'
        return std::make_unique<CharAST>(t.value[0]); 
    }
    
   if (t.value == "(") {
        advance();
        auto expr = parseExpression();
        if (!expr) return nullptr;
        if (getTok().value != ")") return LogError("Expected ')' after expression");
        advance();
        return expr;
    }

    return LogError("Unexpected token '" + t.value + "'");

}

std::unique_ptr<ASTNode> parseLoop() {
    advance(); // Eat 'loop'

    // Optional '('
    if (getTok().value == "(") advance(); 
    
    // Parse Condition
    auto Cond = parseExpression();
    if (!Cond) return nullptr;
    
    // Optional ')'
    if (getTok().value == ")") advance();

    // --- DEBUG CHECK ---
    if (getTok().value != "{") {
        std::cerr << "[Debug] parseLoop Error: Expected '{', but found Token: '" 
                  << getTok().value << "' (Type: " << getTok().type << ")\n";
        return LogError("Expected '{' to start loop body");
    }
    
    auto Body = parseBlock(); 
    if (!Body) return nullptr;

    return std::make_unique<LoopAST>(std::move(Cond), std::move(Body));
}

// --- Parse Block { ... } ---
std::unique_ptr<ASTNode> parseBlock() {
    // 1. Check for opening brace
    if (getTok().value != "{") return LogError("Expected '{' to start block");
    advance(); // Eat '{'

    std::vector<std::unique_ptr<ASTNode>> stmts;

    // 2. Loop until we hit '}' or end of file
    while (getTok().value != "}" && getTok().type != TOK_EOF) {
        std::unique_ptr<ASTNode> stmt = nullptr;
        
        // --- Reuse your existing logic to parse statements ---
        int t = getTok().type;
        if (t == TOK_INT || t == TOK_FLOAT || t == TOK_BOOL || 
            t == TOK_CHAR || t == TOK_STRING || t == TOK_VAR) {
            stmt = parseVarDecl();
        } 
        else if (t == TOK_PRINT) {
            stmt = parsePrint();
        } 
        else if (t == TOK_IF) {
            stmt = parseIfExpr(); // Recursive! Blocks can contain Ifs
        } 
        else {
            stmt = parseExpression();
        }

        // 3. Add to list
        if (stmt) {
            stmts.push_back(std::move(stmt));
            // Eat optional semicolons
            if (getTok().value == ";") advance();
        } else {
            advance(); // Skip errors to avoid infinite loop
        }
    }

    // 4. Check for closing brace
    if (getTok().value != "}") return LogError("Expected '}' to end block");
    advance(); // Eat '}'

    return std::make_unique<BlockAST>(std::move(stmts));
}


// --- DEBUG VERSION of parsePostfix ---
std::unique_ptr<ASTNode> parsePostfix() {
    auto LHS = parsePrimary();
    if (!LHS) return nullptr;

    // Check for Postfix Operators (a++, a--)
    if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
        
        // Ensure we are modifying a variable
        auto *Var = dynamic_cast<VariableAST*>(LHS.get());
        if (!Var) {
            return LogError("Operand of ++/-- must be a variable (e.g., 'a++', not '5++')");
        }

        bool isInc = (getTok().type == TOK_INC);
        advance(); // Eat operator
        
        // Return Update Node
        return std::make_unique<UpdateExprAST>(Var->Name, isInc, false);
    }

    return LHS;
}
// --- DEBUG VERSION of parseUnary ---
std::unique_ptr<ASTNode> parseUnary() {
    // Check for Prefix Operators (++a, --a)
    if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
        bool isInc = (getTok().type == TOK_INC);
        advance(); // Eat operator
        
        auto Operand = parseUnary();
        if (!Operand) return nullptr;

        auto *Var = dynamic_cast<VariableAST*>(Operand.get());
        
        if (!Var) {
            std::cerr << "[Debug] Prefix Error! Expected Variable, but got something else.\n";
            if (dynamic_cast<NumberAST*>(Operand.get())) {
                std::cerr << "        -> It was a NUMBER (e.g. ++5).\n";
            } 
            else if (dynamic_cast<UpdateExprAST*>(Operand.get())) {
                std::cerr << "        -> It was ALREADY an UpdateExpr (e.g. ++(++a)).\n";
            }
            else {
                std::cerr << "        -> It was some complex expression (not a plain variable).\n";
            }

            return LogError("Operand of ++/-- must be a variable");
        }

        return std::make_unique<UpdateExprAST>(Var->Name, isInc, true);
    }

    return parsePostfix();
}

// --- Parse If / Elif / Else ---
std::unique_ptr<ASTNode> parseIfExpr() {
    advance(); // Eat 'if' or 'elif'

    // 1. Condition
    auto Cond = parseExpression();
    if (!Cond) return nullptr;

    // 2. Then Block
    if (getTok().value != "{") return LogError("Expected '{' after if condition");
    auto Then = parseBlock();
    if (!Then) return nullptr;

    // 3. Else / Elif Logic
    std::unique_ptr<ASTNode> Else = nullptr;
    
    // CASE A: 'elif' -> Treat as a nested IF (Recursion)
    if (getTok().type == TOK_ELIF) {
        Else = parseIfExpr(); // Recursively parse the 'elif'
    } 
    // CASE B: 'else'
    else if (getTok().type == TOK_ELSE) {
        advance(); // Eat 'else'
        
        // Handle "else if" (Standard C-style)
        if (getTok().type == TOK_IF) {
            Else = parseIfExpr();
        } else {
            // Handle "else { ... }"
            if (getTok().value != "{") return LogError("Expected '{' after else");
            Else = parseBlock();
        }
    }

    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}
std::unique_ptr<ASTNode> parseVarDecl() {
    Token typeTok = getTok();
    advance(); // Eat 'int', 'bool', 'string', 'var', etc.
    
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

    return std::make_unique<VarDeclAST>(name, typeStr, bytes, std::move(init));
}
// 1. Parse Multiplication & Division (Higher Priority)
// std::unique_ptr<ASTNode> parseTerm() {
//     auto LHS = parsePrimary();
//     if (!LHS) return nullptr;

//     while (true) {
//         Token t = getTok();
//         // Check for High Priority Operators (*, /)
//         if (t.type == '*' || t.type == '/') {
//             char op = (char)t.type;
//             advance(); // Eat operator
            
//             auto RHS = parsePrimary(); // Parse the next number immediately
//             if (!RHS) return nullptr;

//             // Combine them: (LHS * RHS)
//             LHS = std::make_unique<BinaryExprAST>(op, std::move(LHS), std::move(RHS));
//         } else {
//             // If it's not * or /, stop. Let parseExpression handle + or -
//             return LHS;
//         }
//     }
// }

// --- Parse Binary Operators (RHS) ---
// Inside src/parser.cpp

std::unique_ptr<ASTNode> parseBinOpRHS(int ExprPrec, std::unique_ptr<ASTNode> LHS) {
    while (true) {
        int TokPrec = GetTokPrecedence();

        // If this is a token with less precedence, return LHS
        if (TokPrec < ExprPrec)
            return LHS;

        // [FIX] Use getTok().type instead of CurTok.type
        int BinOp = getTok().type;
        
        advance(); // Eat binary operator

        // [CRITICAL FIX] Use parseUnary() to handle 'a * b++' correctly
        // (Old code used parsePrimary(), which breaks precedence)
        auto RHS = parseUnary(); 
        if (!RHS) return nullptr;

        // If the next operator binds more tightly, recurse
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) return nullptr;
        }

        // Merge LHS/RHS
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}
std::unique_ptr<ASTNode> parseExpression() {
    // Start with Unary (handles ++a, a++, or just a)
    auto LHS = parseUnary(); 
    if (!LHS) return nullptr;

    return parseBinOpRHS(0, std::move(LHS));
}
std::unique_ptr<ASTNode> parsePrint() {
    advance(); // Eat 'print'
    
    if (getTok().value != "(") return LogError("Expected '(' after print");
    advance(); // Eat '('

    auto Expr = parseExpression();
    if (!Expr) return nullptr; // Error inside arguments

    if (getTok().value != ")") return LogError("Expected ')' after arguments");
    advance(); // Eat ')'

    return std::make_unique<PrintAST>(std::move(Expr));
}

// --- 4. THE MAIN PARSER ---
// This returns a list of instructions
// --- MAIN PARSE LOOP ---
std::unique_ptr<FunctionAST> parse(const std::vector<Token>& tokens) {
    globalTokens = tokens;
    currentToken = 0;
    HasError = false; // Reset error flag
    
    std::vector<std::unique_ptr<ASTNode>> body;

    while (getTok().type != TOK_EOF) {
        std::unique_ptr<ASTNode> stmt = nullptr;

        // 1. Variable Declarations
        if (getTok().type == TOK_INT || getTok().type == TOK_BOOL || 
            getTok().type == TOK_CHAR || getTok().type == TOK_FLOAT || 
            getTok().type == TOK_STRING || getTok().type == TOK_VAR) {
            stmt = parseVarDecl();
        }

        // 2. Print Statements
        else if (getTok().type == TOK_PRINT) {
            stmt = parsePrint();
        } 

        // 3. If/Else Statements
        else if (getTok().type == TOK_IF) {
            stmt = parseIfExpr();
        }

        // 4. [NEW] Loop Statements
        else if (getTok().type == TOK_LOOP) {
            stmt = parseLoop();
        }
        
        // 5. General Expressions (Math, Assignment, etc.)
        else {
            stmt = parseExpression();
        }

        if (stmt) {
            // Success! Add to body.
            body.push_back(std::move(stmt));
            
            // Handle optional semicolon
            if (getTok().value == ";") advance();
            
        } else {
            // FAILURE: Statement failed to parse.
            // Skip to the next semicolon to recover.
            synchronize(); 
        }
    }

    // Return the "main" function containing all parsed code
    return std::make_unique<FunctionAST>("main", std::move(body));
}