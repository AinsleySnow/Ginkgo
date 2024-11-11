#ifndef _TOKEN_SEQUENCE_H_
#define _TOKEN_SEQUENCE_H_

#include "parser/Token.h"


class TokenSequence
{
public:
    TokenSequence() : tokList_(new TokenList()),
        begin_(tokList_->begin()), end_(tokList_->end()) {
    }
    explicit TokenSequence(Token* tok)
    {
        TokenSequence();
        InsertBack(tok);
    }
    explicit TokenSequence(TokenList* tokList)
        : tokList_(tokList),
        begin_(tokList->begin()),
        end_(tokList->end()) {
    }
    TokenSequence(TokenList* tokList,
        TokenList::iterator begin,
        TokenList::iterator end)
        : tokList_(tokList), begin_(begin), end_(end) {
    }
    ~TokenSequence() {}
    TokenSequence(const TokenSequence& other) { *this = other; }
    const TokenSequence& operator=(const TokenSequence& other)
    {
        tokList_ = other.tokList_;
        begin_ = other.begin_;
        end_ = other.end_;
        return *this;
    }
    void Copy(const TokenSequence& other);
    void UpdateHeadLocation(const SourceLocation& loc);
    void FinalizeSubst(bool leadingWS, const HideSet& hs);

    const Token* Expect(int expect);
    bool Try(int tag);
    bool Test(int tag) { return Peek()->tag_ == tag; }
    const Token* Next();
    void PutBack();
    const Token* Peek() const;
    const Token* Peek2();
    const Token* Back() const;
    void PopBack();

    TokenList::iterator Mark() { return begin_; }
    void ResetTo(TokenList::iterator mark) { begin_ = mark; }
    bool Empty() const { return Peek()->tag_ == Token::END; }
    void InsertBack(TokenSequence& ts);
    void InsertBack(const Token* tok);

    // If there is preceding newline
    void InsertFront(TokenSequence& ts);
    void InsertFront(const Token* tok);
    bool IsBeginOfLine() const;
    TokenSequence GetLine();
    void SetParser(Parser* parser) { parser_ = parser; }
    void Print(FILE* fp = stdout) const;

private:
    // Find a insert position with no preceding newline
    TokenList::iterator GetInsertFrontPos();

    TokenList* tokList_;
    mutable TokenList::iterator begin_;
    TokenList::iterator end_;
    Parser* parser_{ nullptr };
    int exceed_end{ 0 };
};

#endif // _TOKEN_SEQUENCE_H_
