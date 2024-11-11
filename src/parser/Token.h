#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "messages/Error.h"
#include "utils/Pool.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <unordered_map>

class Generator;
class Parser;
class Scanner;
class Token;
class TokenSequence;

using HideSet = std::set<std::string>;
using TokenList = std::list<const Token*>;

struct SourceLocation
{
    const std::string* filename_;
    const char* lineBegin_;
    unsigned line_;
    unsigned column_;

    const char* Begin() const { return lineBegin_ + column_ - 1; }
};

class Token
{
public:
    enum
    {
        // Punctuators
        LPAR = '(',
        RPAR = ')',
        LSQB = '[',
        RSQB = ']',
        COLON = ':',
        COMMA = ',',
        SEMI = ';',
        ADD = '+',
        SUB = '-',
        MUL = '*',
        DIV = '/',
        OR = '|',
        AND = '&',
        XOR = '^',
        LESS = '<',
        GREATER = '>',
        EQUAL = '=',
        DOT = '.',
        MOD = '%',
        LBRACE = '{',
        RBRACE = '}',
        TILDE = '~',
        NOT = '!',
        COND = '?',
        SHARP = '#',
        NEW_LINE = '\n',

        DSHARP = 128, // '##'
        PTR,
        INC,
        DEC,
        LEFT,
        RIGHT,
        LE,
        GE,
        EQ,
        NE,
        LOGICAL_AND,
        LOGICAL_OR,

        MUL_ASSIGN,
        DIV_ASSIGN,
        MOD_ASSIGN,
        ADD_ASSIGN,
        SUB_ASSIGN,
        LEFT_ASSIGN,
        RIGHT_ASSIGN,
        AND_ASSIGN,
        XOR_ASSIGN,
        OR_ASSIGN,

        ELLIPSIS,
        // Punctuators end

        // KEYWORD BEGIN
        // TYPE QUALIFIER BEGIN
        CONST,
        RESTRICT,
        VOLATILE,
        ATOMIC,
        // TYPE QUALIFIER END

        // TYPE SPECIFIER BEGIN
        VOID,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        SIGNED,
        UNSIGNED,
        BOOL,    // _Bool or bool
        COMPLEX, // _Complex
        DEC32, // _Decimal32
        DEC64, // _Decimal64
        DEC128, // _Decimal128
        STRUCT,
        UNION,
        ENUM,
        // TYPE SPECIFIER END

        ATTRIBUTE, // GNU extension __attribute__
        // FUNCTION SPECIFIER BEGIN
        INLINE,
        NORETURN, // _Noreturn
        // FUNCTION SPECIFIER END

        ALIGNAS, // _Alignas
        // For syntactic convenience
        STATIC_ASSERT, // _Static_assert
        // STORAGE CLASS SPECIFIER BEGIN
        TYPEDEF,
        TYPEOF, // new in C23
        TYPEOF_UNQUAL, // new in C23

        TRUE, // new in C23
        FALSE, // new in C23
        NULLPTR, // new in C23

        EXTERN,
        STATIC,
        THREAD, // _Thread_local or thread_local
        AUTO,
        REGISTER,
        // STORAGE CLASS SPECIFIER END
        BREAK,
        CASE,
        CONTINUE,
        DEFAULT,
        DO,
        ELSE,
        FOR,
        GOTO,
        IF,
        RETURN,
        SIZEOF,
        SWITCH,
        WHILE,
        ALIGNOF,   // _Alignof
        GENERIC,   // _Generic
        IMAGINARY, // _Imaginary
        BITINT, // _BitInt, new in C23

        // KEYWORD END

        IDENTIFIER,
        CONSTANT,
        I_CONSTANT,
        C_CONSTANT,
        F_CONSTANT,
        LITERAL,

        // For the parser, a identifier is a typedef name or user defined type
        POSTFIX_INC,
        POSTFIX_DEC,
        PREFIX_INC,
        PREFIX_DEC,
        ADDR,  // '&'
        DEREF, // '*'
        PLUS,
        MINUS,
        CAST,

        // For preprocessor
        PP_IF,
        PP_IFDEF,
        PP_IFNDEF,
        PP_ELIF,
        PP_ELSE,
        PP_ENDIF,
        PP_INCLUDE,
        PP_DEFINE,
        PP_UNDEF,
        PP_LINE,
        PP_ERROR,
        PP_PRAGMA,
        PP_NONE,
        PP_EMPTY,

        IGNORE,
        INVALID,
        END,
        NOTOK = -1,
    };

    explicit Token(int tag) : tag_(tag) {}
    Token(int tag, const SourceLocation& loc,
        const std::string& str, bool ws = false)
        : tag_(tag), ws_(ws), loc_(loc), str_(str) {
    }
    Token(const Token& other) { *this = other; }

    static Token* GetToken(Pool<Token>*, int);
    static Token* GetToken(Pool<Token>*, const Token&);
    static Token* GetToken(Pool<Token>*, int,
        const SourceLocation&, const std::string&,
        bool = false);

    Token& operator=(const Token& other)
    {
        tag_ = other.tag_;
        ws_ = other.ws_;
        loc_ = other.loc_;
        str_ = other.str_;
        hs_ = other.hs_ ? new HideSet(*other.hs_) : nullptr;
        return *this;
    }
    virtual ~Token() {}

    // Token::NOTOK represents not a kw.
    static int KeyWordTag(const std::string& key)
    {
        auto kwIter = kewordTypeMap_.find(key);
        if (kewordTypeMap_.end() == kwIter)
            return Token::NOTOK; // Not a key word type
        return kwIter->second;
    }

    static bool IsKeyWord(int tag) { return CONST <= tag && tag < IDENTIFIER; }
    bool IsKeyWord() const { return IsKeyWord(tag_); }
    bool IsPunctuator() const { return 0 <= tag_ && tag_ <= ELLIPSIS; }
    bool IsLiteral() const { return tag_ == LITERAL; }
    bool IsConstant() const { return CONSTANT <= tag_ && tag_ <= F_CONSTANT; }
    bool IsIdentifier() const { return IDENTIFIER == tag_; }
    bool IsEOF() const { return tag_ == Token::END; }
    bool IsTypeSpecQual() const { return CONST <= tag_ && tag_ <= ENUM; }
    bool IsDecl() const { return CONST <= tag_ && tag_ <= REGISTER; }
    static const char* Lexeme(int tag)
    {
        auto iter = tagLexemeMap_.find(tag);
        if (iter == tagLexemeMap_.end())
            return nullptr;

        return iter->second;
    }

    int tag_;

    // 'ws_' standards for weither there is preceding white space
    // This is to simplify the '#' operator(stringize) in macro expansion
    bool ws_{ false };
    SourceLocation loc_;

    std::string str_;
    HideSet* hs_{ nullptr };

private:
    static const std::unordered_map<std::string, int> kewordTypeMap_;
    static const std::unordered_map<int, const char*> tagLexemeMap_;
};

#endif // _TOKEN_H_
