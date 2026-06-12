#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

// ============================================================
//  Token types
// ============================================================

enum class TokenType {
    // Literals
    IntLit,         // 42

    // Identifiers & keywords
    Ident,          // foo, bar
    KwInt,          // int
    KwIf,           // if
    KwElse,         // else
    KwWhile,        // while
    KwReturn,       // return

    // Arithmetic operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /

    // Comparison operators
    EqEq,           // ==
    BangEq,         // !=
    Lt,             // <
    Gt,             // >
    LtEq,           // <=
    GtEq,           // >=

    // Assignment
    Eq,             // =

    // Delimiters
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    Semicolon,      // ;
    Comma,          // ,

    // Special
    Eof,
};

inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::IntLit:    return "IntLit";
        case TokenType::Ident:     return "Ident";
        case TokenType::KwInt:     return "int";
        case TokenType::KwIf:      return "if";
        case TokenType::KwElse:    return "else";
        case TokenType::KwWhile:   return "while";
        case TokenType::KwReturn:  return "return";
        case TokenType::Plus:      return "+";
        case TokenType::Minus:     return "-";
        case TokenType::Star:      return "*";
        case TokenType::Slash:     return "/";
        case TokenType::EqEq:      return "==";
        case TokenType::BangEq:    return "!=";
        case TokenType::Lt:        return "<";
        case TokenType::Gt:        return ">";
        case TokenType::LtEq:      return "<=";
        case TokenType::GtEq:      return ">=";
        case TokenType::Eq:        return "=";
        case TokenType::LParen:    return "(";
        case TokenType::RParen:    return ")";
        case TokenType::LBrace:    return "{";
        case TokenType::RBrace:    return "}";
        case TokenType::Semicolon: return ";";
        case TokenType::Comma:     return ",";
        case TokenType::Eof:       return "<eof>";
    }
    return "?";
}

// ============================================================
//  Token
// ============================================================

struct Token {
    TokenType   type;
    std::string text;   // original source text
    int         line;
    int         col;

    // Convenience for integer literals
    int intValue() const { return std::stoi(text); }
};

// ============================================================
//  LexError
// ============================================================

struct LexError : std::runtime_error {
    int line, col;
    LexError(const std::string& msg, int line, int col)
        : std::runtime_error(msg), line(line), col(col) {}
};

// ============================================================
//  Lexer
// ============================================================

class Lexer {
public:
    explicit Lexer(std::string src)
        : src_(std::move(src)), pos_(0), line_(1), col_(1) {}

    /// Tokenise the entire source and return a flat token list.
    /// The last token is always Eof.
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            Token tok = nextToken();
            tokens.push_back(tok);
            if (tok.type == TokenType::Eof) break;
        }
        return tokens;
    }

private:
    std::string src_;
    size_t      pos_;
    int         line_;
    int         col_;

    // ----------------------------------------------------------
    //  Character helpers
    // ----------------------------------------------------------

    bool atEnd() const { return pos_ >= src_.size(); }

    char peek(size_t offset = 0) const {
        size_t i = pos_ + offset;
        return i < src_.size() ? src_[i] : '\0';
    }

    char advance() {
        char c = src_[pos_++];
        if (c == '\n') { ++line_; col_ = 1; }
        else           { ++col_; }
        return c;
    }

    bool match(char expected) {
        if (atEnd() || src_[pos_] != expected) return false;
        advance();
        return true;
    }

    // ----------------------------------------------------------
    //  Skip whitespace and line comments
    // ----------------------------------------------------------

    void skipWhitespaceAndComments() {
        while (!atEnd()) {
            char c = peek();

            // Whitespace
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance();
                continue;
            }

            // Line comment: //
            if (c == '/' && peek(1) == '/') {
                while (!atEnd() && peek() != '\n') advance();
                continue;
            }

            // Block comment: /* ... */
            if (c == '/' && peek(1) == '*') {
                int startLine = line_, startCol = col_;
                advance(); advance();   // consume /*
                while (!atEnd()) {
                    if (peek() == '*' && peek(1) == '/') {
                        advance(); advance();   // consume */
                        goto next_char;
                    }
                    advance();
                }
                throw LexError("Unterminated block comment", startLine, startCol);
            }

            break;
            next_char:;
        }
    }

    // ----------------------------------------------------------
    //  Token factories
    // ----------------------------------------------------------

    Token makeToken(TokenType type, std::string text, int line, int col) {
        return Token{type, std::move(text), line, col};
    }

    // ----------------------------------------------------------
    //  Scan one token
    // ----------------------------------------------------------

    Token nextToken() {
        skipWhitespaceAndComments();

        if (atEnd())
            return makeToken(TokenType::Eof, "", line_, col_);

        int startLine = line_;
        int startCol  = col_;
        char c = advance();

        // --- Integer literal ---
        if (isDigit(c)) {
            std::string text(1, c);
            while (!atEnd() && isDigit(peek())) text += advance();
            return makeToken(TokenType::IntLit, text, startLine, startCol);
        }

        // --- Identifier or keyword ---
        if (isAlpha(c)) {
            std::string text(1, c);
            while (!atEnd() && isAlphaNum(peek())) text += advance();
            return makeToken(keywordOrIdent(text), text, startLine, startCol);
        }

        // --- Operators and delimiters ---
        switch (c) {
            case '+': return makeToken(TokenType::Plus,      "+", startLine, startCol);
            case '-': return makeToken(TokenType::Minus,     "-", startLine, startCol);
            case '*': return makeToken(TokenType::Star,      "*", startLine, startCol);
            case '/': return makeToken(TokenType::Slash,     "/", startLine, startCol);
            case '(': return makeToken(TokenType::LParen,    "(", startLine, startCol);
            case ')': return makeToken(TokenType::RParen,    ")", startLine, startCol);
            case '{': return makeToken(TokenType::LBrace,    "{", startLine, startCol);
            case '}': return makeToken(TokenType::RBrace,    "}", startLine, startCol);
            case ';': return makeToken(TokenType::Semicolon, ";", startLine, startCol);
            case ',': return makeToken(TokenType::Comma,     ",", startLine, startCol);

            case '=':
                if (match('=')) return makeToken(TokenType::EqEq,  "==", startLine, startCol);
                return             makeToken(TokenType::Eq,     "=",  startLine, startCol);

            case '!':
                if (match('=')) return makeToken(TokenType::BangEq, "!=", startLine, startCol);
                throw LexError("Unexpected character '!'", startLine, startCol);

            case '<':
                if (match('=')) return makeToken(TokenType::LtEq, "<=", startLine, startCol);
                return             makeToken(TokenType::Lt,   "<",  startLine, startCol);

            case '>':
                if (match('=')) return makeToken(TokenType::GtEq, ">=", startLine, startCol);
                return             makeToken(TokenType::Gt,   ">",  startLine, startCol);

            default:
                throw LexError(
                    std::string("Unexpected character '") + c + "'",
                    startLine, startCol);
        }
    }

    // ----------------------------------------------------------
    //  Character classification
    // ----------------------------------------------------------

    static bool isDigit(char c)    { return c >= '0' && c <= '9'; }
    static bool isAlpha(char c)    { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    static bool isAlphaNum(char c) { return isAlpha(c) || isDigit(c); }

    // ----------------------------------------------------------
    //  Keyword table
    // ----------------------------------------------------------

    static TokenType keywordOrIdent(const std::string& text) {
        if (text == "int")    return TokenType::KwInt;
        if (text == "if")     return TokenType::KwIf;
        if (text == "else")   return TokenType::KwElse;
        if (text == "while")  return TokenType::KwWhile;
        if (text == "return") return TokenType::KwReturn;
        return TokenType::Ident;
    }
};
