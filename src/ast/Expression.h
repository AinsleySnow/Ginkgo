#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expr.h"
#include <deque>
#include <memory>

class Visitor;


class AssignExpr : public Expr
{
public:
    AssignExpr(std::unique_ptr<Expr> l, Tag t, std::unique_ptr<Expr> r) :
        left_(std::move(l)), op_(t), right_(std::move(r)) {}

    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    friend class CodeChk;
    std::unique_ptr<Expr> left_{};
    Tag op_;
    std::unique_ptr<Expr> right_{};
};


class BinaryExpr : public Expr
{
public:
    BinaryExpr(std::unique_ptr<Expr> l, Tag t, std::unique_ptr<Expr> r) :
        left_(std::move(l)), op_(t), right_(std::move(r)) {}

    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    friend class CodeChk;
    std::unique_ptr<Expr> left_{};
    Tag op_;
    std::unique_ptr<Expr> right_{};
};


class ExprList;
class CallExpr : public Expr
{
public:
    CallExpr(std::unique_ptr<Expr> e) : postfix_(std::move(e)) {}
    CallExpr(std::unique_ptr<Expr> e, std::unique_ptr<ExprList> el) :
        postfix_(std::move(e)), argvlist_(std::move(el)) {}

    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Expr> postfix_{};
    std::unique_ptr<ExprList> argvlist_{};
};


class CastExpr : public Expr
{
public:
    CastExpr(std::unique_ptr<Declaration> tn, std::unique_ptr<Expr> e) :
        typename_(std::move(tn)), expr_(std::move(e)) {}

    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Declaration> typename_{};
    std::unique_ptr<Expr> expr_{};
};


class CondExpr : public Expr
{
public:
    CondExpr(std::unique_ptr<Expr> c, std::unique_ptr<Expr> t,
        std::unique_ptr<Expr> f) : cond_(std::move(c)), true_(std::move(t)), false_(std::move(f)) {}
    
    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    friend class CodeChk;
    std::unique_ptr<Expr> cond_{};
    std::unique_ptr<Expr> true_{};
    std::unique_ptr<Expr> false_{};
};


class ConstExpr : public Expr
{
public:
    ConstExpr() {}
    explicit ConstExpr(uint64_t u);
    explicit ConstExpr(double d);
    explicit ConstExpr(bool b);

    ConstExpr(ConstExpr&&) = default;
    ConstExpr& operator=(ConstExpr&&) = default;

    uint64_t GetInt() const { return val_.intgr_; }
    double GetFloat() const { return val_.flt_; }
    bool IsZero() const { return val_.intgr_ == 0; }

    void Accept(Visitor* v) override;

    static bool DoCalc(Tag, const ConstExpr*, ConstExpr&);
    static bool DoCalc(Tag, const ConstExpr*, const ConstExpr*, ConstExpr&);

private:
    union
    {
        uint64_t intgr_;
        double flt_;
    } val_;
};


class ExprList : public Expr
{
public:
    void Accept(Visitor* v) override;

    void Append(std::unique_ptr<Expr> expr);
    auto begin() { return exprlist_.begin(); }
    auto end() { return exprlist_.end(); }

private:
    friend class IRGen;
    std::deque<std::unique_ptr<Expr>> exprlist_{};
};


class IdentExpr : public Expr
{
public:
    IdentExpr(const std::string& n) : name_(n) {}

    void Accept(Visitor*) override;
    bool IsLVal() const override { return true; }

private:
    friend class IRGen;
    std::string name_{};
};


class LogicalExpr : public Expr
{
public:
    LogicalExpr(std::unique_ptr<Expr> l, Tag t, std::unique_ptr<Expr> r) :
        left_(std::move(l)), op_(t), right_(std::move(r)) {}

    void Accept(Visitor* v);

private:
    friend class IRGen;
    friend class CodeChk;
    std::unique_ptr<Expr> left_{};
    Tag op_;
    std::unique_ptr<Expr> right_{};
};


class StrExpr : public Expr
{
public:
    StrExpr(const std::string& s) : content_(s) {}

    void Accept(Visitor* v) override;

private:
    std::string content_{};
};


class UnaryExpr : public Expr
{
public:
    UnaryExpr(Tag t, std::unique_ptr<Expr> c) :
        op_(t), content_(std::move(c)) {}

    void Accept(Visitor* v) override;

private:
    friend class IRGen;
    friend class CodeChk;
    Tag op_;
    std::unique_ptr<Expr> content_{};
};

#endif // _EXPRESSION_H_
