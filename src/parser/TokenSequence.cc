#include "parser/TokenSequence.h"


TokenList::iterator TokenSequence::GetInsertFrontPos()
{
    auto pos = begin_;
    if (pos == tokList_->begin())
        return pos;
    --pos;
    while (pos != tokList_->begin() && (*pos)->tag_ == Token::NEW_LINE)
        --pos;
    return ++pos;
}

void TokenSequence::Copy(const TokenSequence& other)
{
    tokList_ = new TokenList(other.begin_, other.end_);
    begin_ = tokList_->begin();
    end_ = tokList_->end();
    for (auto iter = begin_; iter != end_; ++iter)
        *iter = Token::GetToken(this, **iter);
}

void TokenSequence::UpdateHeadLocation(const SourceLocation& loc)
{
    assert(!Empty());
    auto tok = const_cast<Token*>(Peek());
    tok->loc_ = loc;
}

void TokenSequence::FinalizeSubst(bool leadingWS, const HideSet& hs)
{
    auto ts = *this;
    while (!ts.Empty())
    {
        auto tok = const_cast<Token*>(ts.Next());
        if (!tok->hs_)
            tok->hs_ = new HideSet(hs);
        else
            tok->hs_->insert(hs.begin(), hs.end());
    }
    // Even if the token sequence is empty
    const_cast<Token*>(Peek())->ws_ = leadingWS;
}

bool TokenSequence::Try(int tag)
{
    if (Peek()->tag_ == tag)
    {
        Next();
        return true;
    }
    return false;
}

const Token* TokenSequence::Next()
{
    auto ret = Peek();
    if (!ret->IsEOF())
    {
        ++begin_;
        Peek(); // May skip newline token, but why ?
    }
    else
        ++exceed_end;
    return ret;
}

void TokenSequence::PutBack()
{
    assert(begin_ != tokList_->begin());
    if (exceed_end > 0)
        --exceed_end;
    else
    {
        --begin_;
        if ((*begin_)->tag_ == Token::NEW_LINE)
            PutBack();
    }
}

const Token* TokenSequence::Peek2()
{
    if (Empty())
        return Peek(); // Return the Token::END
    Next();
    auto ret = Peek();
    PutBack();
    return ret;
}

const Token* TokenSequence::Back() const
{
    auto back = end_;
    return *--back;
}

void TokenSequence::PopBack()
{
    assert(!Empty());
    assert(end_ == tokList_->end());
    auto size_eq1 = tokList_->back() == *begin_;
    tokList_->pop_back();
    end_ = tokList_->end();
    if (size_eq1)
        begin_ = end_;
}

void TokenSequence::InsertBack(TokenSequence& ts)
{
    auto pos = tokList_->insert(end_, ts.begin_, ts.end_);
    if (begin_ == end_)
        begin_ = pos;
}

void TokenSequence::InsertBack(const Token* tok)
{
    auto pos = tokList_->insert(end_, tok);
    if (begin_ == end_)
        begin_ = pos;
}

TokenSequence TokenSequence::GetLine()
{
    auto begin = begin_;
    while (begin_ != end_ && (*begin_)->tag_ != Token::NEW_LINE)
        ++begin_;
    auto end = begin_;
    return { tokList_, begin, end };
}

void TokenSequence::InsertFront(TokenSequence& ts)
{
    auto pos = GetInsertFrontPos();
    begin_ = tokList_->insert(pos, ts.begin_, ts.end_);
}

void TokenSequence::InsertFront(const Token* tok)
{
    auto pos = GetInsertFrontPos();
    begin_ = tokList_->insert(pos, tok);
}

/*
 * If this seq starts from the begin of a line.
 * Called only after we have saw '#' in the token sequence.
 */
bool TokenSequence::IsBeginOfLine() const
{
    if (begin_ == tokList_->begin())
        return true;

    auto pre = begin_;
    --pre;

    // We do not insert a newline at the end of a source file.
    // Thus if two token have different filename, the second is
    // the begin of a line.
    return ((*pre)->tag_ == Token::NEW_LINE ||
        (*pre)->loc_.filename_ != (*begin_)->loc_.filename_);
}

const Token* TokenSequence::Peek()
{
    static auto eof = Token::GetToken(this, Token::END);
    if (begin_ != end_ && (*begin_)->tag_ == Token::NEW_LINE)
    {
        ++begin_;
        return Peek();
    }
    else if (begin_ == end_)
    {
        if (end_ != tokList_->begin())
            *eof = *Back();
        eof->tag_ = Token::END;
        return eof;
    }
    else if (parser_ && (*begin_)->tag_ == Token::IDENTIFIER &&
        (*begin_)->str_ == "__func__")
    {
        auto filename = Token::GetToken(this, *(*begin_));
        filename->tag_ = Token::LITERAL;
        filename->str_ = "\"" + parser_->CurFunc()->Name() + "\"";
        *begin_ = filename;
    }
    return *begin_;
}

const Token* TokenSequence::Expect(int expect)
{
    auto tok = Peek();
    if (!Try(expect))
    {
        Error(tok, "'%s' expected, but got '%s'",
            Token::Lexeme(expect), tok->str_.c_str());
    }
    return tok;
}

void TokenSequence::Print(FILE* fp) const
{
    unsigned lastLine = 0;
    auto ts = *this;
    while (!ts.Empty())
    {
        auto tok = ts.Next();
        if (lastLine != tok->loc_.line_)
        {
            fputs("\n", fp);
            for (unsigned i = 0; i < tok->loc_.column_; ++i)
                fputc(' ', fp);
        }
        else if (tok->ws_)
            fputc(' ', fp);
        fputs(tok->str_.c_str(), fp);
        fflush(fp);
        lastLine = tok->loc_.line_;
    }
    fputs("\n", fp);
}
