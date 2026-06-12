#pragma once

#include "ast.hpp"
#include <iostream>
#include <string>

// ============================================================
//  ASTPrinter  — prints a human-readable tree to stdout
// ============================================================

class ASTPrinter : public Visitor {
public:
    explicit ASTPrinter(std::ostream& out = std::cout) : out_(out) {}

    void visit(Program& node) override {
        line("Program");
        indent([&]{ for (auto& d : node.decls) d->accept(*this); });
    }

    void visit(FunctionDecl& node) override {
        line("FunctionDecl '" + node.name + "'");
        indent([&]{ node.body->accept(*this); });
    }

    void visit(Block& node) override {
        line("Block");
        indent([&]{ for (auto& s : node.stmts) s->accept(*this); });
    }

    void visit(VarDecl& node) override {
        line("VarDecl '" + node.name + "'");
        if (node.init)
            indent([&]{ node.init->accept(*this); });
    }

    void visit(AssignStmt& node) override {
        line("AssignStmt '" + node.name + "'");
        indent([&]{ node.value->accept(*this); });
    }

    void visit(IfStmt& node) override {
        line("IfStmt");
        indent([&]{
            line("Cond:");
            indent([&]{ node.cond->accept(*this); });
            line("Then:");
            indent([&]{ node.then->accept(*this); });
            if (node.else_) {
                line("Else:");
                indent([&]{ node.else_->accept(*this); });
            }
        });
    }

    void visit(WhileStmt& node) override {
        line("WhileStmt");
        indent([&]{
            line("Cond:");
            indent([&]{ node.cond->accept(*this); });
            line("Body:");
            indent([&]{ node.body->accept(*this); });
        });
    }

    void visit(ReturnStmt& node) override {
        line("ReturnStmt");
        if (node.value)
            indent([&]{ node.value->accept(*this); });
    }

    void visit(ExprStmt& node) override {
        line("ExprStmt");
        indent([&]{ node.expr->accept(*this); });
    }

    void visit(IntLiteral& node) override {
        line("IntLiteral " + std::to_string(node.value));
    }

    void visit(VarRef& node) override {
        line("VarRef '" + node.name + "'");
    }

    void visit(BinaryExpr& node) override {
        line("BinaryExpr '" + BinaryExpr::opName(node.op) + "'");
        indent([&]{
            node.lhs->accept(*this);
            node.rhs->accept(*this);
        });
    }

    void visit(CallExpr& node) override {
        line("CallExpr '" + node.callee + "'");
        indent([&]{
            for (auto& a : node.args) a->accept(*this);
        });
    }

private:
    std::ostream& out_;
    int           depth_ = 0;

    void line(const std::string& text) {
        out_ << std::string(depth_ * 2, ' ') << text << "\n";
    }

    template<typename F>
    void indent(F fn) { ++depth_; fn(); --depth_; }
};
