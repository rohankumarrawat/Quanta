#ifndef QUANTA_H
#define QUANTA_H

#include <string>
#include <vector>
#include <memory>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h" 
#include <map>

extern bool HasError;
extern std::map<int, int> BinopPrecedence;

// --- 1. LEXER (Vocabulary) ---
enum TokenType {
    TOK_EOF = -1,
    TOK_PRINT = -2,
    TOK_IDENTIFIER = -3,
    TOK_NUMBER = -4,
    TOK_STRING = -5,
    TOK_IF = -6,       // <--- ADDED
    TOK_THEN = -7,     // <--- ADDED
    TOK_ELSE = -8,     // <--- ADDED
    TOK_ELIF = -19,
    TOK_VAR = -20,
    TOK_INT   = -10, // int, int1, int4, int8
    TOK_FLOAT = -11, // float, float4, float8
    TOK_BOOL  = -12, // bool
    TOK_CHAR  = -13, // char
    TOK_TRUE  = -14, // true
    TOK_FALSE = -15 , // false
    TOK_EQ = -16,
};

struct Token {
    int type;
    std::string value;
    int line;
};

std::vector<Token> tokenize(std::string source);

// --- 2. AST (The Shapes) ---


class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value *codegen() = 0; 
};

// Shape: "10"
// include/quanta.h

class NumberAST : public ASTNode {
    int64_t Val; // Changed from 'int' to 'int64_t'
public:
    NumberAST(int64_t Val) : Val(Val) {} 
    llvm::Value *codegen() override;
};

class FloatAST : public ASTNode {
public:
    double Val;
    FloatAST(double val) : Val(val) {}
    llvm::Value *codegen() override;
};

class BoolAST : public ASTNode {
public:
    bool Val;
    BoolAST(bool val) : Val(val) {}
    llvm::Value *codegen() override;
};

// Shape: "'A'"
class CharAST : public ASTNode {
public:
    char Val;
    CharAST(char val) : Val(val) {}
    llvm::Value *codegen() override;
};

// Shape: "hello"
struct StringAST : public ASTNode {
    std::string val;
    StringAST(std::string v) : val(v) {}
    llvm::Value *codegen() override;
};

class VariableAST : public ASTNode {
public:
    std::string Name;
    VariableAST(const std::string &name) : Name(name) {}
    llvm::Value *codegen() override;
};
// --- Assignment AST (e.g., x = 5) ---
class AssignmentAST : public ASTNode {
    std::string Name;
    std::unique_ptr<ASTNode> RHS;
public:
    AssignmentAST(const std::string &Name, std::unique_ptr<ASTNode> RHS)
        : Name(Name), RHS(std::move(RHS)) {}

    llvm::Value *codegen() override;
};
// Shape: "x"
class VarDeclAST : public ASTNode {
public:
    std::string Name;
    std::string Type; // "int", "float", "bool", "char"
    int Bytes;        // 1, 4, 8, etc.
    std::unique_ptr<ASTNode> InitVal;
    
    VarDeclAST(const std::string &name, const std::string &type, int bytes, std::unique_ptr<ASTNode> init)
        : Name(name), Type(type), Bytes(bytes), InitVal(std::move(init)) {}
        
    llvm::Value *codegen() override;
};
// Shape: "x + 5"
class BinaryExprAST : public ASTNode {
public:
    char Op;
    std::unique_ptr<ASTNode> LHS, RHS;
    BinaryExprAST(char op, std::unique_ptr<ASTNode> LHS, std::unique_ptr<ASTNode> RHS)
        : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    llvm::Value *codegen() override;
};



// Shape: "print(x)"
class PrintAST : public ASTNode {
public:
    std::unique_ptr<ASTNode> Expr;
    PrintAST(std::unique_ptr<ASTNode> expr) : Expr(std::move(expr)) {}
    llvm::Value *codegen() override;
};

class ByteSizeAST : public ASTNode {
    std::string Name;
public:
    ByteSizeAST(const std::string &Name) : Name(Name) {}
    llvm::Value *codegen() override;
};


// 1. BLOCK AST (Scope)
class BlockAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> Statements;
public:
    BlockAST(std::vector<std::unique_ptr<ASTNode>> Statements)
        : Statements(std::move(Statements)) {}
    llvm::Value *codegen() override;
};

// 2. IF AST
class IfExprAST : public ASTNode {
    std::unique_ptr<ASTNode> Cond;
    std::unique_ptr<ASTNode> Then;
    std::unique_ptr<ASTNode> Else;
public:
    IfExprAST(std::unique_ptr<ASTNode> Cond, std::unique_ptr<ASTNode> Then,
              std::unique_ptr<ASTNode> Else)
        : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}
    llvm::Value *codegen() override;
};

class TypeofAST : public ASTNode {
    std::string Name;
public:
    TypeofAST(const std::string &Name) : Name(Name) {}
    llvm::Value *codegen() override;
};

// --- NEW: Function Wrapper ---
// This is required because parser.cpp returns this now.
class FunctionAST : public ASTNode {
public:
    std::string Name;
    std::vector<std::unique_ptr<ASTNode>> Body;
    
    FunctionAST(const std::string &name, std::vector<std::unique_ptr<ASTNode>> body)
        : Name(name), Body(std::move(body)) {}
        
    // Returns llvm::Function*, not just llvm::Value*
    llvm::Function *codegen() override;
};

// --- 3. PARSER ---
// FIXED: Returns a single FunctionAST, not a vector.
std::unique_ptr<FunctionAST> parse(const std::vector<Token>& tokens);

// --- 4. UTILS ---
void initializeModule();
void generateObjectCode();

#endif