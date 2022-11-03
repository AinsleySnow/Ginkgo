#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include "Expr.h"
#include "types/Type.h"
#include "visitors/Visitor.h"
#include <deque>
#include <memory>

class AssignExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};


public:
    AssignExpr(std::shared_ptr<Expr> l, Tag t, std::shared_ptr<Expr> r) :
        left_(l), op_(t), right_(r) {}

    void Accept(Visitor* v) override { v->VisitAssignExpr(this); }
    AssignExpr* ToAssign() override { return this; }
};


class BinaryExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};


public:
    BinaryExpr(std::shared_ptr<Expr> l, Tag t, std::shared_ptr<Expr> r) :
        left_(l), op_(t), right_(r) {}

    void Accept(Visitor* v) override { v->VisitBinaryExpr(this); }
    BinaryExpr* ToBinary() override { return this; }
};


class CondExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> cond_{};
    std::shared_ptr<Expr> true_{};
    std::shared_ptr<Expr> false_{};


public:
    CondExpr(std::shared_ptr<Expr> c, std::shared_ptr<Expr> t,
        std::shared_ptr<Expr> f) : cond_(c), true_(t), false_(f) {}
    
    void Accept(Visitor* v) { v->VisitCondExpr(this); }
    CondExpr* ToCondition() { return this; }
};


class Constant : public Expr
{
private:
    union
    {
        uint64_t intgr_;
        double flt_;
    } val_;


public:
    Constant() {}
    explicit Constant(uint64_t u);
    explicit Constant(double d);
    explicit Constant(bool b);
    ~Constant() {}

    uint64_t GetInt() const { return val_.intgr_; }
    double GetFloat() const { return val_.flt_; }

    void Accept(Visitor* v) override { v->VisitConstant(this); }
    Constant* ToConstant() override { return this; }

    static bool DoCalc(Tag, const Constant*, Constant&);
    static bool DoCalc(Tag, const Constant*, const Constant*, Constant&);
};


class ExprList : public Expr
{
private:
    std::deque<std::shared_ptr<Expr>> exprlist_{};

public:
    void Accept(Visitor* v) override { v->VisitExprList(this); }
    ExprList* ToExprList() override { return this; }

    void Append(std::shared_ptr<Expr> expr) { exprlist_.push_back(expr); }
    auto begin() { return exprlist_.begin(); }
    auto end() { return exprlist_.end(); }
};


class LogicalExpr : public Expr
{
private:
    friend class IRGen;
    std::shared_ptr<Expr> left_{};
    Tag op_;
    std::shared_ptr<Expr> right_{};


public:
    LogicalExpr(std::shared_ptr<Expr> l, Tag t, std::shared_ptr<Expr> r) :
        left_(l), op_(t), right_(r) {}

    void Accept(Visitor* v) { v->VisitLogicalExpr(this); }
    LogicalExpr* ToLogical() { return this; }
};


class Str : public Expr
{
private:
    std::string content_{};

public:
    Str(std::string s) : content_(s) {}

    void Accept(Visitor* v) override { v->VisitStr(this); }
    Str* ToStr() override { return this; }
};


class UnaryExpr : public Expr
{
private:
    friend class IRGen;
    Tag op_;
    std::shared_ptr<Expr> content_{};


public:
    UnaryExpr(Tag t, std::shared_ptr<Expr> c) :
        op_(t), content_(c) {}

    void Accept(Visitor* v) override { v->VisitUnaryExpr(this); }
    UnaryExpr* ToUnary() override { return this; }
};

#endif // _EXPRESSION_H_
