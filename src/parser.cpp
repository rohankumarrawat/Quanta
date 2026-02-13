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
std::unique_ptr<ASTNode> parseLoop() {
    advance(); // Eat 'loop'

    if (getTok().value != "(") return LogError("Expected '(' after loop");
    advance(); 

    auto Cond = parseExpression();
    if (!Cond) return nullptr;

    if (getTok().value != ")") return LogError("Expected ')' after loop condition");
    advance(); 

    if (getTok().value != "{") return LogError("Expected '{' to start loop body");
    
    // [FIX] Wrap the vector in BlockAST
    auto bodyStmts = parseBlock();
    auto Body = std::make_unique<BlockAST>(std::move(bodyStmts));

    return std::make_unique<LoopAST>(std::move(Cond), std::move(Body));
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
 
    // --- 5. IDENTIFIERS & ASSIGNMENT ---
   // --- 5. IDENTIFIERS, ASSIGNMENT & FUNCTION CALLS ---
    if (t.type == TOK_IDENTIFIER) {
        std::string IdName = t.value;
        advance(); // Eat the identifier (e.g., "a" or "add")

        // CASE A: Function Call? -> Is the next token '(' ?
        if (getTok().value == "(") {
            advance(); // Eat '('
            
            // Parse Arguments (e.g., 1, 2, x)
            std::vector<std::unique_ptr<ASTNode>> args;
            if (getTok().value != ")") {
                while (true) {
                    auto arg = parseExpression();
                    if (arg) args.push_back(std::move(arg));
                    else return nullptr;

                    if (getTok().value == ")") break;
                    
                    if (getTok().value != ",") {
                        return LogError("Expected ')' or ',' in argument list");
                    }
                    advance(); // Eat ','
                }
            }
            advance(); // Eat ')'
            
            // Return Call Node
            return std::make_unique<CallAST>(IdName, std::move(args));
        }

        // CASE B: Assignment? -> Is the next token '=' ?
        if (getTok().value == "=") {
            advance(); // Eat '='
            
            auto RHS = parseExpression();
            if (!RHS) return nullptr;

            return std::make_unique<AssignmentAST>(IdName, std::move(RHS));
        }

        // CASE C: Variable Read -> Just the name "a"
        return std::make_unique<VariableAST>(IdName);
    }
    // --- Inside parsePrimary() in src/parser.cpp ---

  
    if (t.type == TOK_FLOAT) {
        advance();
        return std::make_unique<FloatAST>(std::stod(t.value));
    }

    if (t.type == TOK_LOOP) {
        return parseLoop();
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

// --- PARSE FUNCTION ---
std::unique_ptr<FunctionAST> parseFunction() {
    // 1. Return Type
    std::string returnType = "void";
    if (getTok().type == TOK_INT) returnType = "int";
    else if (getTok().type == TOK_FLOAT) returnType = "float";
    else if (getTok().type == TOK_STRING) returnType = "string";
    else if (getTok().type == TOK_VOID) returnType = "void";
    advance(); // Eat type

    // 2. Function Name
    std::string name = getTok().value;
    advance(); // Eat name

    // 3. Arguments
    advance(); // Eat '('
    // (Argument parsing logic will go here later)
    advance(); // Eat ')'

    // 4. Body
    // parseBlock returns the vector of statements needed by FunctionAST
    auto body = parseBlock();

    return std::make_unique<FunctionAST>(
        returnType, 
        name, 
        std::vector<std::pair<std::string, std::string>>(), 
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
        advance(); // Eat 'return'
        std::unique_ptr<ASTNode> expr = nullptr;
        if (getTok().value != ";") expr = parseExpression();
        if (getTok().value == ";") advance(); // Eat ';'
        return expr; 
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

// --- 1. POSTFIX PARSER (Highest Priority: i++) ---
std::unique_ptr<ASTNode> parsePostfix() {
    auto LHS = parsePrimary(); // Gets 'i'
    if (!LHS) return nullptr;

    // Check for operator immediately after variable
    if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
        
        auto *Var = dynamic_cast<VariableAST*>(LHS.get());
        if (!Var) return LogError("Operand of ++/-- must be a variable");

        bool isInc = (getTok().type == TOK_INC);
        advance(); // Eat '++'
        
        // Return Update Node (IsPrefix = false)
        return std::make_unique<UpdateExprAST>(Var->Name, isInc, false);
    }
    return LHS;
}

// --- 2. PREFIX PARSER (Medium Priority: ++i) ---
std::unique_ptr<ASTNode> parseUnary() {
    // Check for operator BEFORE variable
    if (getTok().type == TOK_INC || getTok().type == TOK_DEC) {
        bool isInc = (getTok().type == TOK_INC);
        advance(); // Eat '++'
        
        // Recursively parse operand (allows things like ++(++i))
        auto Operand = parseUnary(); 
        if (!Operand) return nullptr;

        auto *Var = dynamic_cast<VariableAST*>(Operand.get());
        if (!Var) return LogError("Operand of ++/-- must be a variable");

        // Return Update Node (IsPrefix = true)
        return std::make_unique<UpdateExprAST>(Var->Name, isInc, true);
    }
    
    // If not prefix, fall down to Postfix
    return parsePostfix();
}

std::unique_ptr<ASTNode> parseExpression() {
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
ProgramAST parse(const std::vector<Token>& tokens) {
    globalTokens = tokens;
    currentToken = 0;
    HasError = false;

    ProgramAST program;
    std::vector<std::unique_ptr<ASTNode>> scriptBody; // Buffer for script code

    while (getTok().type != TOK_EOF) {
        
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
    if (!scriptBody.empty()) {
        if (hasExplicitMain) {
            std::cerr << "Error: Cannot mix top-level script code with an explicit 'main' function." << std::endl;
            HasError = true;
        } else {
            // Auto-generate: void main() { ... scriptBody ... }
            program.functions.push_back(std::make_unique<FunctionAST>(
                "void", 
                "main", 
                std::vector<std::pair<std::string, std::string>>(), 
                std::move(scriptBody)
            ));
        }
    } 
    // 3. Handle Empty File (prevent linker error)
    else if (!hasExplicitMain) {
         program.functions.push_back(std::make_unique<FunctionAST>(
            "void", "main", 
            std::vector<std::pair<std::string, std::string>>(), 
            std::vector<std::unique_ptr<ASTNode>>()
        ));
    }

    return program;
}