#include "../include/quanta.h"
#include <vector>
#include <string>
#include <memory>

// --- STATE MANAGEMENT ---
int currentToken = 0;
std::vector<Token> globalTokens;

Token getTok() {
    if (currentToken < globalTokens.size()) return globalTokens[currentToken];
    return {TOK_EOF, ""};
}

void advance() { currentToken++; }

// Forward Declaration
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
        return std::make_unique<NumberAST>(std::stoi(t.value));
    }
    if (t.type == TOK_IDENTIFIER) {
        advance();
        return std::make_unique<VariableAST>(t.value);
    }
    return nullptr;
}
// --- 2. EXPRESSION PARSER ---
std::unique_ptr<ASTNode> parseExpression() {
    auto LHS = parsePrimary();
    if (!LHS) return nullptr;

    Token t = getTok();
    
    // Check for operators: +, -, *, /, =
    if (t.type == '+' || t.type == '-' || t.type == '*' || t.type == '/' || t.type == '=') {
        char op = (char)t.type;
        advance(); 
        
        auto RHS = parseExpression(); 
        if (RHS) {
            return std::make_unique<BinaryExprAST>(op, std::move(LHS), std::move(RHS));
        }
    }

    return LHS; 
}

// --- 3. PRINT PARSER ---
// Parses: print( ... )
std::unique_ptr<ASTNode> parsePrint() {
    advance(); // Eat 'print'
    
    if (getTok().value != "(") return nullptr;
    advance(); // Eat '('

    auto Expr = parseExpression();

    if (getTok().value != ")") return nullptr;
    advance(); // Eat ')'

    return std::make_unique<PrintAST>(std::move(Expr));
}

// --- 4. THE MAIN PARSER ---
// This returns a list of instructions
std::vector<std::unique_ptr<ASTNode>> parse(const std::vector<Token>& tokens) {
    globalTokens = tokens;
    currentToken = 0;
    std::vector<std::unique_ptr<ASTNode>> statements;

    while (getTok().type != TOK_EOF) {
        if (getTok().type == TOK_PRINT) {
            auto stmt = parsePrint();
            if (stmt) statements.push_back(std::move(stmt));
        } else {
            auto stmt = parseExpression();
            if (stmt) statements.push_back(std::move(stmt));
        }
    }
    return statements;
}