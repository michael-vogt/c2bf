#pragma once

#include <memory>
#include <string>
#include <vector>

// ============================================================
//  Forward declarations
// ============================================================

struct ASTNode;
struct Program;
struct FunctionDecl;
struct VarDecl;
struct Block;

// Statements
struct AssignStmt;
struct IfStmt;
struct WhileStmt;
struct ReturnStmt;
struct ExprStmt;

// Expressions
struct IntLiteral;
struct VarRef;
struct BinaryExpr;
struct CallExpr;

// ============================================================
//  Visitor interface
// ============================================================

struct Visitor {
    virtual ~Visitor() = default;

    // Declarations / top-level
    virtual void visit(Program&)      = 0;
    virtual void visit(FunctionDecl&) = 0;
    virtual void visit(VarDecl&)      = 0;
    virtual void visit(Block&)        = 0;

    // Statements
    virtual void visit(AssignStmt&)   = 0;
    virtual void visit(IfStmt&)       = 0;
    virtual void visit(WhileStmt&)    = 0;
    virtual void visit(ReturnStmt&)   = 0;
    virtual void visit(ExprStmt&)     = 0;

    // Expressions
    virtual void visit(IntLiteral&)   = 0;
    virtual void visit(VarRef&)       = 0;
    virtual void visit(BinaryExpr&)   = 0;
    virtual void visit(CallExpr&)     = 0;
};

// ============================================================
//  Base node
// ============================================================

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& v) = 0;
};

using NodePtr = std::unique_ptr<ASTNode>;

// ============================================================
//  Expressions
// ============================================================

/// Integer literal:  42
struct IntLiteral : ASTNode {
    int value;
    explicit IntLiteral(int v) : value(v) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

/// Variable reference:  x
struct VarRef : ASTNode {
    std::string name;
    explicit VarRef(std::string n) : name(std::move(n)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

/// Binary expression:  lhs OP rhs
///   Arithmetic : + - * /
///   Comparison : == != < > <= >=
struct BinaryExpr : ASTNode {
    enum class Op {
        Add, Sub, Mul, Div,
        Eq, Ne, Lt, Gt, Le, Ge
    };

    Op            op;
    NodePtr       lhs;
    NodePtr       rhs;

    BinaryExpr(Op op, NodePtr lhs, NodePtr rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(Visitor& v) override { v.visit(*this); }

    static std::string opName(Op op) {
        switch (op) {
            case Op::Add: return "+";
            case Op::Sub: return "-";
            case Op::Mul: return "*";
            case Op::Div: return "/";
            case Op::Eq:  return "==";
            case Op::Ne:  return "!=";
            case Op::Lt:  return "<";
            case Op::Gt:  return ">";
            case Op::Le:  return "<=";
            case Op::Ge:  return ">=";
        }
        return "?";
    }
};

/// Function call:  putchar(expr)  |  getchar()
struct CallExpr : ASTNode {
    std::string           callee;
    std::vector<NodePtr>  args;

    CallExpr(std::string callee, std::vector<NodePtr> args)
        : callee(std::move(callee)), args(std::move(args)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

// ============================================================
//  Statements
// ============================================================

/// Variable declaration with optional initializer:  int x = expr;
struct VarDecl : ASTNode {
    std::string name;
    NodePtr     init;   // may be nullptr

    VarDecl(std::string name, NodePtr init = nullptr)
        : name(std::move(name)), init(std::move(init)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// Assignment:  x = expr;
struct AssignStmt : ASTNode {
    std::string name;
    NodePtr     value;

    AssignStmt(std::string name, NodePtr value)
        : name(std::move(name)), value(std::move(value)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// Block of statements:  { stmt* }
struct Block : ASTNode {
    std::vector<NodePtr> stmts;

    explicit Block(std::vector<NodePtr> stmts)
        : stmts(std::move(stmts)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// If statement:  if (cond) then [else else_]
struct IfStmt : ASTNode {
    NodePtr cond;
    NodePtr then;
    NodePtr else_;  // may be nullptr

    IfStmt(NodePtr cond, NodePtr then, NodePtr else_ = nullptr)
        : cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// While loop:  while (cond) body
struct WhileStmt : ASTNode {
    NodePtr cond;
    NodePtr body;

    WhileStmt(NodePtr cond, NodePtr body)
        : cond(std::move(cond)), body(std::move(body)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// Return statement:  return expr;
struct ReturnStmt : ASTNode {
    NodePtr value;  // may be nullptr for bare return;

    explicit ReturnStmt(NodePtr value = nullptr)
        : value(std::move(value)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// Expression used as a statement:  putchar(x);
struct ExprStmt : ASTNode {
    NodePtr expr;

    explicit ExprStmt(NodePtr expr)
        : expr(std::move(expr)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

// ============================================================
//  Top-level
// ============================================================

/// Function declaration:  int main() { body }
struct FunctionDecl : ASTNode {
    std::string name;
    NodePtr     body;   // always a Block

    FunctionDecl(std::string name, NodePtr body)
        : name(std::move(name)), body(std::move(body)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};

/// Root of the tree
struct Program : ASTNode {
    std::vector<NodePtr> decls;  // FunctionDecl nodes

    explicit Program(std::vector<NodePtr> decls)
        : decls(std::move(decls)) {}

    void accept(Visitor& v) override { v.visit(*this); }
};
