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
    TOK_IF = -6,       
    TOK_THEN = -7,     
    TOK_ELSE = -8,     
    
    TOK_INT   = -10, 
    TOK_FLOAT = -11, 
    TOK_BOOL  = -12, 
    TOK_CHAR  = -13, 
    TOK_TRUE  = -14, 
    TOK_FALSE = -15, 
    TOK_EQ = -16,

    TOK_INC = -17, 
    TOK_DEC = -18, 
    TOK_ELIF = -19,
    TOK_VAR = -20,

    TOK_LOOP = -21,
    TOK_NEQ = -22,  // !=
    TOK_GEQ = -23,  // >=
    TOK_LEQ = -24,  // <=
    
    TOK_VOID = -25,
    TOK_RETURN = -26
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

// [IMPORTANT] Forward Declaration
class FunctionAST;

// [IMPORTANT] Container for the whole program (Functions + Main)
struct ProgramAST {
    std::vector<std::unique_ptr<FunctionAST>> functions;
};

// --- Expression Classes ---

class NumberAST : public ASTNode {
    int64_t Val; 
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

class CharAST : public ASTNode {
public:
    char Val;
    CharAST(char val) : Val(val) {}
    llvm::Value *codegen() override;
};

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

class AssignmentAST : public ASTNode {
    std::string Name;
    std::unique_ptr<ASTNode> RHS;
public:
    AssignmentAST(const std::string &Name, std::unique_ptr<ASTNode> RHS)
        : Name(Name), RHS(std::move(RHS)) {}

    llvm::Value *codegen() override;
};

class VarDeclAST : public ASTNode {
public:
    std::string Name;
    std::string Type; 
    int Bytes;        
    std::unique_ptr<ASTNode> InitVal;
    
    VarDeclAST(const std::string &name, const std::string &type, int bytes, std::unique_ptr<ASTNode> init)
        : Name(name), Type(type), Bytes(bytes), InitVal(std::move(init)) {}
        
    llvm::Value *codegen() override;
};

class BinaryExprAST : public ASTNode {
public:
    char Op;
    std::unique_ptr<ASTNode> LHS, RHS;
    BinaryExprAST(char op, std::unique_ptr<ASTNode> LHS, std::unique_ptr<ASTNode> RHS)
        : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    llvm::Value *codegen() override;
};

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

class UpdateExprAST : public ASTNode {
    std::string Name;
    bool IsIncrement; 
    bool IsPrefix;    

public:
    UpdateExprAST(const std::string &name, bool isIncrement, bool isPrefix)
        : Name(name), IsIncrement(isIncrement), IsPrefix(isPrefix) {}

    llvm::Value *codegen() override;
};

class BlockAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> Statements;
public:
    BlockAST(std::vector<std::unique_ptr<ASTNode>> Statements)
        : Statements(std::move(Statements)) {}
    llvm::Value *codegen() override;
};
// Shape: "add(1, 2)"
class CallAST : public ASTNode {
    std::string Callee;
    std::vector<std::unique_ptr<ASTNode>> Args;
public:
    CallAST(const std::string &Callee, std::vector<std::unique_ptr<ASTNode>> Args)
        : Callee(Callee), Args(std::move(Args)) {}

    llvm::Value *codegen() override;
};
class LoopAST : public ASTNode {
    std::unique_ptr<ASTNode> Cond;
    std::unique_ptr<ASTNode> Body;

public:
    LoopAST(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}

    llvm::Value *codegen() override;
};

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

// --- FUNCTION AST ---
class FunctionAST : public ASTNode {
public:
    std::string ReturnType; 
    std::string Name;      
    std::vector<std::pair<std::string, std::string>> Args; 
    std::vector<std::unique_ptr<ASTNode>> Body; 
    
    FunctionAST(const std::string& type, 
                const std::string& name, 
                std::vector<std::pair<std::string, std::string>> args,
                std::vector<std::unique_ptr<ASTNode>> body)
        : ReturnType(type), Name(name), Args(std::move(args)), Body(std::move(body)) {}
        
    const std::string& getName() const { return Name; }
        
    llvm::Function *codegen() override;
};

// --- 3. PARSER ---
// [IMPORTANT] Returns ProgramAST (List of Functions), not a single function pointer.
ProgramAST parse(const std::vector<Token>& tokens);

// --- 4. UTILS ---
void initializeModule();
void generateObjectCode();

#endif