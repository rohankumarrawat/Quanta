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
    TOK_INT8 = -9,
    
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
    TOK_RETURN = -26,
    TOK_IMPORT = -27, // Add this new token
    TOK_DOT = -28,    // Token for '.'
    TOK_ALL = -29 ,   // Token for '*'
    TOK_LEN = -30,
    TOK_UPPER = -31,
    TOK_LOWER = -32,
    TOK_REVERSE = -33,
    TOK_ISUPPER = -34,
    TOK_ISLOWER = -35,
    TOK_STRIP = -36,
    TOK_LSTRIP = -37,
    TOK_RSTRIP = -38,
    TOK_CAPITALIZE = -39,
    TOK_TITLE = -40,
    TOK_ISALPHA = -41,
    TOK_ISDIGIT = -42,
    TOK_ISSPACE = -43,
    TOK_ISALNUM = -44,
    TOK_FIND = -45,
    TOK_COUNT = -46,
    TOK_STARTSWITH = -47,
    TOK_ENDSWITH = -48,
    TOK_REPLACE = -49,
    TOK_IN = -50,

};
#include <set>
extern std::set<std::string> LoadedModules;
extern std::string RootDir;

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
    NumberAST(int64_t Val) : Val(Val) {
      
    } 
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
    
    // --- ADD THIS LINE ---
    const std::string &getName() const { return Name; }
    // ---------------------

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
    // OLD: std::unique_ptr<ASTNode> Expr;
    // NEW: Store a list of arguments
    std::vector<std::unique_ptr<ASTNode>> Args;

    // Update Constructor to take a vector
    PrintAST(std::vector<std::unique_ptr<ASTNode>> args) 
        : Args(std::move(args)) {}

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

// class CallAST : public ASTNode {
//     std::string Callee;
//     std::vector<std::unique_ptr<ASTNode>> Args;
// public:
//     CallAST(const std::string &Callee, std::vector<std::unique_ptr<ASTNode>> Args)
//         : Callee(Callee), Args(std::move(Args)) {}

//     llvm::Value *codegen() override;
// };

struct CallArg {
    std::string Name;             // Stores "rollno" (or empty "" if positional)
    std::shared_ptr<ASTNode> Val; // The value (e.g., 20)
};

// 2. Update CallAST to hold a vector of CallArg
class CallAST : public ASTNode {
    std::string Callee;
    std::vector<CallArg> Args;

public:
    CallAST(const std::string &Callee, std::vector<CallArg> Args)
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

// loop i in string { body } -- i is index (int), stack-only
class LoopOverStringAST : public ASTNode {
    std::string VarName;
    std::unique_ptr<ASTNode> StringExpr;
    std::unique_ptr<ASTNode> Body;

public:
    LoopOverStringAST(std::string varName, std::unique_ptr<ASTNode> stringExpr, std::unique_ptr<ASTNode> body)
        : VarName(std::move(varName)), StringExpr(std::move(stringExpr)), Body(std::move(body)) {}

    llvm::Value *codegen() override;
};

// s[i] -> returns char (i8), stack-only load
class StringIndexAST : public ASTNode {
    std::unique_ptr<ASTNode> BaseExpr;
    std::unique_ptr<ASTNode> IndexExpr;

public:
    StringIndexAST(std::unique_ptr<ASTNode> base, std::unique_ptr<ASTNode> index)
        : BaseExpr(std::move(base)), IndexExpr(std::move(index)) {}

    llvm::Value *codegen() override;
};

// s[start:end] or s[start:end:step] -> new string (heap, one allocation)
class StringSliceAST : public ASTNode {
    std::unique_ptr<ASTNode> BaseExpr;
    std::unique_ptr<ASTNode> StartExpr;
    std::unique_ptr<ASTNode> EndExpr;
    std::unique_ptr<ASTNode> StepExpr;  // optional; null = 1

public:
    StringSliceAST(std::unique_ptr<ASTNode> base, std::unique_ptr<ASTNode> start, std::unique_ptr<ASTNode> end, std::unique_ptr<ASTNode> step)
        : BaseExpr(std::move(base)), StartExpr(std::move(start)), EndExpr(std::move(end)), StepExpr(std::move(step)) {}

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

// --- STRING OPERATION AST ---
class StringOpAST : public ASTNode {
    int OpType; // Stores the Token type (e.g., TOK_LEN, TOK_UPPER)
    std::unique_ptr<ASTNode> Operand;

public:
    StringOpAST(int opType, std::unique_ptr<ASTNode> operand)
        : OpType(opType), Operand(std::move(operand)) {}

    llvm::Value *codegen() override;
};

// Two-argument string ops: find(s, sub), count(s, sub), startswith(s, prefix), endswith(s, suffix)
class StringOp2AST : public ASTNode {
    int OpType;
    std::unique_ptr<ASTNode> Operand1;
    std::unique_ptr<ASTNode> Operand2;

public:
    StringOp2AST(int opType, std::unique_ptr<ASTNode> op1, std::unique_ptr<ASTNode> op2)
        : OpType(opType), Operand1(std::move(op1)), Operand2(std::move(op2)) {}

    llvm::Value *codegen() override;
};

// Three-argument: replace(s, old, new)
class StringOp3AST : public ASTNode {
    int OpType;
    std::unique_ptr<ASTNode> Operand1;
    std::unique_ptr<ASTNode> Operand2;
    std::unique_ptr<ASTNode> Operand3;

public:
    StringOp3AST(int opType, std::unique_ptr<ASTNode> op1, std::unique_ptr<ASTNode> op2, std::unique_ptr<ASTNode> op3)
        : OpType(opType), Operand1(std::move(op1)), Operand2(std::move(op2)), Operand3(std::move(op3)) {}

    llvm::Value *codegen() override;
};
class ReturnAST : public ASTNode {
    std::unique_ptr<ASTNode> Expr;
    int Line;

public:
    ReturnAST(std::unique_ptr<ASTNode> Expr, int Line) 
        : Expr(std::move(Expr)),Line(Line) {}

    llvm::Value *codegen() override;
};

class FixedStringDeclAST : public ASTNode {
    std::string VarName;
    int Capacity; 
    std::unique_ptr<ASTNode> InitValue;

public:
    FixedStringDeclAST(std::string varName, int capacity, std::unique_ptr<ASTNode> initValue)
        : VarName(varName), Capacity(capacity), InitValue(std::move(initValue)) {}

    llvm::Value *codegen() override;
};

class TypeofAST : public ASTNode {
    std::string Name;
public:
    TypeofAST(const std::string &Name) : Name(Name) {}
    llvm::Value *codegen() override;
};

struct ArgInfo {
    std::string Name;
    std::string Type;
    std::shared_ptr<ASTNode> DefaultValue;
};

struct FunctionInfo {
    std::string Name;
    std::vector<ArgInfo> Args;
};
extern std::map<std::string, FunctionInfo> FunctionRegistry;

// 1. Define the Argument Structure
struct FuncArg {
    std::string Type;
    std::string Name;
    // You can add 'std::unique_ptr<ASTNode> DefaultValue' here later if needed
};



// 2. Updated FunctionAST Class
class FunctionAST : public ASTNode {
public:
    std::string ReturnType; 
    std::string Name;      
    std::vector<FuncArg> Args; // Now uses the struct
    std::vector<std::unique_ptr<ASTNode>> Body; 
    
    FunctionAST(const std::string& type, 
                const std::string& name, 
                std::vector<FuncArg> args, // Matches the struct vector
                std::vector<std::unique_ptr<ASTNode>> body)
        : ReturnType(type), Name(name), Args(std::move(args)), Body(std::move(body)) {}
        
    // Helper to get the function name easily
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