#pragma once

#include "ast.hpp"
#include "lexer.hpp"

#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
//  ParseError
// ============================================================

struct ParseError : std::runtime_error {
    int line, col;
    ParseError(const std::string& msg, int line, int col)
        : std::runtime_error(msg), line(line), col(col) {}
};

// ============================================================
//  Parser  (recursive descent)
//
//  Grammar (informal):
//
//  program       ::= function_decl* EOF
//  function_decl ::= 'int' IDENT '(' ')' block
//  block         ::= '{' stmt* '}'
//
//  stmt          ::= var_decl
//                  | assign_stmt
//                  | if_stmt
//                  | while_stmt
//                  | return_stmt
//                  | expr_stmt
//
//  var_decl      ::= 'int' IDENT ( '=' expr )? ';'
//  assign_stmt   ::= IDENT '=' expr ';'
//  if_stmt       ::= 'if' '(' expr ')' block ( 'else' block )?
//  while_stmt    ::= 'while' '(' expr ')' block
//  return_stmt   ::= 'return' expr? ';'
//  expr_stmt     ::= expr ';'
//
//  expr          ::= comparison
//  comparison    ::= addition ( ( '==' | '!=' | '<' | '>' | '<=' | '>=' ) addition )*
//  addition      ::= multiplication ( ( '+' | '-' ) multiplication )*
//  multiplication::= unary ( ( '*' | '/' ) unary )*
//  unary         ::= '-' unary | primary
//  primary       ::= INT_LIT | IDENT | call_expr | '(' expr ')'
//  call_expr     ::= IDENT '(' ( expr ( ',' expr )* )? ')'
// ============================================================

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
        : tokens_(std::move(tokens)), pos_(0) {}

    std::unique_ptr<Program> parse() {
        std::vector<NodePtr> decls;
        while (!check(TokenType::Eof)) {
            decls.push_back(parseFunctionDecl());
        }
        return std::make_unique<Program>(std::move(decls));
    }

private:
    std::vector<Token> tokens_;
    size_t             pos_;

    // ----------------------------------------------------------
    //  Token navigation
    // ----------------------------------------------------------

    const Token& current() const { return tokens_[pos_]; }
    const Token& peek(size_t offset = 1) const {
        size_t i = pos_ + offset;
        return i < tokens_.size() ? tokens_[i] : tokens_.back();
    }

    bool check(TokenType t) const { return current().type == t; }

    bool checkAny(std::initializer_list<TokenType> types) const {
        for (auto t : types) if (check(t)) return true;
        return false;
    }

    Token advance() {
        Token t = current();
        if (!check(TokenType::Eof)) ++pos_;
        return t;
    }

    Token expect(TokenType t) {
        if (!check(t)) {
            auto& cur = current();
            throw ParseError(
                "Expected '" + tokenTypeName(t) +
                "' but got '" + tokenTypeName(cur.type) +
                "' (\"" + cur.text + "\")",
                cur.line, cur.col);
        }
        return advance();
    }

    bool match(TokenType t) {
        if (!check(t)) return false;
        advance();
        return true;
    }

    // Helper: error at current position
    ParseError error(const std::string& msg) const {
        return ParseError(msg, current().line, current().col);
    }

    // ----------------------------------------------------------
    //  Top-level
    // ----------------------------------------------------------

    // function_decl ::= 'int' IDENT '(' ')' block
    std::unique_ptr<FunctionDecl> parseFunctionDecl() {
        expect(TokenType::KwInt);
        Token name = expect(TokenType::Ident);
        expect(TokenType::LParen);
        expect(TokenType::RParen);
        NodePtr body = parseBlock();
        return std::make_unique<FunctionDecl>(name.text, std::move(body));
    }

    // ----------------------------------------------------------
    //  Block  ::= '{' stmt* '}'
    // ----------------------------------------------------------

    std::unique_ptr<Block> parseBlock() {
        expect(TokenType::LBrace);
        std::vector<NodePtr> stmts;
        while (!check(TokenType::RBrace) && !check(TokenType::Eof)) {
            stmts.push_back(parseStmt());
        }
        expect(TokenType::RBrace);
        return std::make_unique<Block>(std::move(stmts));
    }

    // ----------------------------------------------------------
    //  Statements
    // ----------------------------------------------------------

    NodePtr parseStmt() {
        // var_decl
        if (check(TokenType::KwInt))
            return parseVarDecl();

        // if_stmt
        if (check(TokenType::KwIf))
            return parseIfStmt();

        // while_stmt
        if (check(TokenType::KwWhile))
            return parseWhileStmt();

        // return_stmt
        if (check(TokenType::KwReturn))
            return parseReturnStmt();

        // assign_stmt:  IDENT '=' expr ';'
        // Disambiguate from expr_stmt by looking one token ahead.
        if (check(TokenType::Ident) && peek().type == TokenType::Eq)
            return parseAssignStmt();

        // expr_stmt
        return parseExprStmt();
    }

    // var_decl ::= 'int' IDENT ( '=' expr )? ';'
    NodePtr parseVarDecl() {
        expect(TokenType::KwInt);
        Token name = expect(TokenType::Ident);
        NodePtr init;
        if (match(TokenType::Eq))
            init = parseExpr();
        expect(TokenType::Semicolon);
        return std::make_unique<VarDecl>(name.text, std::move(init));
    }

    // assign_stmt ::= IDENT '=' expr ';'
    NodePtr parseAssignStmt() {
        Token name = expect(TokenType::Ident);
        expect(TokenType::Eq);
        NodePtr value = parseExpr();
        expect(TokenType::Semicolon);
        return std::make_unique<AssignStmt>(name.text, std::move(value));
    }

    // if_stmt ::= 'if' '(' expr ')' block ( 'else' block )?
    NodePtr parseIfStmt() {
        expect(TokenType::KwIf);
        expect(TokenType::LParen);
        NodePtr cond = parseExpr();
        expect(TokenType::RParen);
        NodePtr then = parseBlock();
        NodePtr else_;
        if (match(TokenType::KwElse))
            else_ = parseBlock();
        return std::make_unique<IfStmt>(std::move(cond), std::move(then), std::move(else_));
    }

    // while_stmt ::= 'while' '(' expr ')' block
    NodePtr parseWhileStmt() {
        expect(TokenType::KwWhile);
        expect(TokenType::LParen);
        NodePtr cond = parseExpr();
        expect(TokenType::RParen);
        NodePtr body = parseBlock();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }

    // return_stmt ::= 'return' expr? ';'
    NodePtr parseReturnStmt() {
        expect(TokenType::KwReturn);
        NodePtr value;
        if (!check(TokenType::Semicolon))
            value = parseExpr();
        expect(TokenType::Semicolon);
        return std::make_unique<ReturnStmt>(std::move(value));
    }

    // expr_stmt ::= expr ';'
    NodePtr parseExprStmt() {
        NodePtr expr = parseExpr();
        expect(TokenType::Semicolon);
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    // ----------------------------------------------------------
    //  Expressions  (precedence climbing via recursive descent)
    //
    //  Precedence (low → high):
    //    comparison   == != < > <= >=
    //    addition     + -
    //    multiply     * /
    //    unary        - (prefix)
    //    primary
    // ----------------------------------------------------------

    NodePtr parseExpr() { return parseComparison(); }

    NodePtr parseComparison() {
        NodePtr lhs = parseAddition();
        while (checkAny({TokenType::EqEq, TokenType::BangEq,
                         TokenType::Lt,   TokenType::Gt,
                         TokenType::LtEq, TokenType::GtEq})) {
            Token op = advance();
            NodePtr rhs = parseAddition();
            lhs = std::make_unique<BinaryExpr>(
                tokenToBinaryOp(op), std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    NodePtr parseAddition() {
        NodePtr lhs = parseMultiplication();
        while (checkAny({TokenType::Plus, TokenType::Minus})) {
            Token op = advance();
            NodePtr rhs = parseMultiplication();
            lhs = std::make_unique<BinaryExpr>(
                tokenToBinaryOp(op), std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    NodePtr parseMultiplication() {
        NodePtr lhs = parseUnary();
        while (checkAny({TokenType::Star, TokenType::Slash})) {
            Token op = advance();
            NodePtr rhs = parseUnary();
            lhs = std::make_unique<BinaryExpr>(
                tokenToBinaryOp(op), std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    NodePtr parseUnary() {
        if (check(TokenType::Minus)) {
            Token op = advance();
            NodePtr operand = parseUnary();
            // Encode unary minus as  0 - operand
            return std::make_unique<BinaryExpr>(
                BinaryExpr::Op::Sub,
                std::make_unique<IntLiteral>(0),
                std::move(operand));
        }
        return parsePrimary();
    }

    NodePtr parsePrimary() {
        // Integer literal
        if (check(TokenType::IntLit)) {
            Token t = advance();
            return std::make_unique<IntLiteral>(t.intValue());
        }

        // Identifier or call
        if (check(TokenType::Ident)) {
            // call:  IDENT '(' ... ')'
            if (peek().type == TokenType::LParen)
                return parseCallExpr();

            Token t = advance();
            return std::make_unique<VarRef>(t.text);
        }

        // Parenthesised expression
        if (check(TokenType::LParen)) {
            advance();
            NodePtr expr = parseExpr();
            expect(TokenType::RParen);
            return expr;
        }

        throw error("Expected expression, got '" +
                    tokenTypeName(current().type) + "'");
    }

    // call_expr ::= IDENT '(' ( expr ( ',' expr )* )? ')'
    NodePtr parseCallExpr() {
        Token name = expect(TokenType::Ident);
        expect(TokenType::LParen);
        std::vector<NodePtr> args;
        if (!check(TokenType::RParen)) {
            args.push_back(parseExpr());
            while (match(TokenType::Comma))
                args.push_back(parseExpr());
        }
        expect(TokenType::RParen);
        return std::make_unique<CallExpr>(name.text, std::move(args));
    }

    // ----------------------------------------------------------
    //  Helper: map Token -> BinaryExpr::Op
    // ----------------------------------------------------------

    static BinaryExpr::Op tokenToBinaryOp(const Token& t) {
        switch (t.type) {
            case TokenType::Plus:   return BinaryExpr::Op::Add;
            case TokenType::Minus:  return BinaryExpr::Op::Sub;
            case TokenType::Star:   return BinaryExpr::Op::Mul;
            case TokenType::Slash:  return BinaryExpr::Op::Div;
            case TokenType::EqEq:   return BinaryExpr::Op::Eq;
            case TokenType::BangEq: return BinaryExpr::Op::Ne;
            case TokenType::Lt:     return BinaryExpr::Op::Lt;
            case TokenType::Gt:     return BinaryExpr::Op::Gt;
            case TokenType::LtEq:   return BinaryExpr::Op::Le;
            case TokenType::GtEq:   return BinaryExpr::Op::Ge;
            default:
                throw ParseError("Not a binary operator: " + t.text, t.line, t.col);
        }
    }
};
