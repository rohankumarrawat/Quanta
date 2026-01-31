#ifndef QUANTA_H
#define QUANTA_H

#include <string>
#include <vector>
#include <memory>
#include "llvm/IR/Value.h"

// --- 1. LEXER (Vocabulary) ---
// We removed DEF, IF, ELSE, RETURN. We added PRINT.
enum TokenType {
    TOK_EOF = -1,
    TOK_PRINT = -2,      // The 'print' command
    TOK_IDENTIFIER = -3, // Variables like 'x'
    TOK_NUMBER = -4 ,     // Numbers like '10'
    TOK_STRING = -5
};

struct Token {
    int type;
    std::string value;
};

std::vector<Token> tokenize(std::string source);

// --- 2. AST (The Shapes) ---

class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // This allows the Compiler to work
    virtual llvm::Value *codegen() = 0; 

    // THIS IS THE FIX: This allows the Interpreter (eval) to work
    virtual int eval() { return 0; } 
};

// Shape: "10"
class NumberAST : public ASTNode {
public:
    int Val;
    NumberAST(int val) : Val(val) {}
    llvm::Value *codegen() override;
};

// 1. The Box to hold "hello"
struct StringAST : public ASTNode {
    std::string val;
    StringAST(std::string v) : val(v) {}

    int eval() override { return 0; } // Strings don't have a numeric value
    llvm::Value *codegen() override;  // We will define this later
};

// Shape: "x"
class VariableAST : public ASTNode {
public:
    std::string Name;
    VariableAST(const std::string &name) : Name(name) {}
    llvm::Value *codegen() override;
};

// Shape: "x + 5" or "x = 10"
class BinaryExprAST : public ASTNode {
public:
    char Op; // '+', '-', '*', '/', '='
    std::unique_ptr<ASTNode> LHS, RHS;
    BinaryExprAST(char op, std::unique_ptr<ASTNode> LHS, std::unique_ptr<ASTNode> RHS)
        : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    llvm::Value *codegen() override;
};

// Shape: "print(x)"
// This is new! It allows us to see output.
class PrintAST : public ASTNode {
public:
    std::unique_ptr<ASTNode> Expr;
    PrintAST(std::unique_ptr<ASTNode> expr) : Expr(std::move(expr)) {}
    llvm::Value *codegen() override;
};

// --- 3. PARSER (The Architect) ---
// Instead of looking for a Function, we just return a list of instructions
// Example: { x=10, y=x+5, print(y) }
// std::vector<std::unique_ptr<ASTNode>> parse(const std::vector<Token>& tokens);
// Change this line in quanta.h
std::vector<std::unique_ptr<ASTNode>> parse(const std::vector<Token>& tokens);

// --- 4. UTILS ---
void initializeModule();
void generateObjectCode();

#endif