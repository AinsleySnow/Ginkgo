#ifndef _EXPRESSION_H_
#define _EXPRESSION_H_

#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expr.h"
#include <list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class Declaration;
class Register;
class ASTVisitor;


class ArrayExpr : public Expr
{
public:
    ArrayExpr(std::unique_ptr<Expr> ident, std::unique_ptr<Expr> index) :
        identifier_(std::move(ident)), index_(std::move(index)) {}

    void Accept(ASTVisitor* v) override;

    bool IsSubscript() const override { return true; }
    ArrayExpr* ToSubscript() override { return this; }
    const ArrayExpr* ToSubscript() const override { return this; }

    auto Addr() const { return addr_; }
    auto& Addr() { return addr_; }
    const auto& Identifier() const { return identifier_; }
    const auto& Index() const { return index_; }

private:
    friend class IRGen;
    const Register* addr_{};
    std::unique_ptr<Expr> identifier_{};
    std::unique_ptr<Expr> index_{};
};


class AssignExpr : public Expr
{
public:
    AssignExpr(std::unique_ptr<Expr> l, Tag t, std::unique_ptr<Expr> r) :
        left_(std::move(l)), op_(t), right_(std::move(r)) {}

    void Accept(ASTVisitor* v) override;

    const auto& Left() const { return left_; }
    Tag Op() const { return op_; }
    const auto& Right() const { return right_; }

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

    void Accept(ASTVisitor* v) override;
    bool IsConstant() const override;

    const auto& Left() const { return left_; }
    const auto& Right() const { return right_; }

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

    void Accept(ASTVisitor* v) override;

    const auto& Postfix() const { return postfix_; }
    const auto& ArgvList() const { return argvlist_; }

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

    void Accept(ASTVisitor* v) override;

    const auto& TypeName() const { return typename_; }
    const auto& Expression() const { return expr_; }

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
    
    void Accept(ASTVisitor* v) override;

    const auto& Condition() const { return cond_; }
    const auto& TrueExpr() const { return true_; }
    const auto& FalseExpr() const { return false_; }

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
    explicit ConstExpr(const std::string& s);
    explicit ConstExpr(uint64_t u, int b, const std::string& s);
    explicit ConstExpr(double d, char s);

    uint64_t GetInt() const;
    double GetFloat() const;
    bool IsZero() const;

    bool IsConstant() const override { return true; }
    ConstExpr* ToConstant() override { return this; }
    const ConstExpr* ToConstant() const override { return this; }

    void Accept(ASTVisitor* v) override;

private:
    std::variant<uint64_t, bool, double> val_{};
};


class EnumConst : public Expr
{
public:
    EnumConst(const std::string& n) : name_(n) {}
    EnumConst(const std::string& n, std::unique_ptr<Expr> e) :
        name_(n), expr_(std::move(e)) {}

    void Accept(ASTVisitor*) override;
    bool IsConstant() const override { return true; }

    const auto& Name() const { return name_; }
    const auto& ValueExpr() const { return expr_; }

private:
    std::string name_{};
    std::unique_ptr<Expr> expr_{};
};


class EnumList : public Expr
{
public:
    void Accept(ASTVisitor* v) override;

    void Append(std::unique_ptr<EnumConst>);
    auto Count() const { return exprlist_.size(); }

    auto begin() { return exprlist_.begin(); }
    auto end() { return exprlist_.end(); }
    auto front() { return exprlist_.front().get(); }
    auto back() { return exprlist_.back().get(); }

private:
    std::vector<std::unique_ptr<EnumConst>> exprlist_{};
};


class ExprList : public Expr
{
public:
    void Accept(ASTVisitor* v) override;

    void Append(std::unique_ptr<Expr> expr);
    auto begin() { return exprlist_.begin(); }
    auto end() { return exprlist_.end(); }

private:
    friend class IRGen;
    std::list<std::unique_ptr<Expr>> exprlist_{};
};


class IdentExpr : public Expr
{
public:
    IdentExpr(const std::string& n) : name_(n) {}

    void Accept(ASTVisitor*) override;
    bool IsLVal() const override { return true; }

    bool IsIdentifier() const override { return true; }
    IdentExpr* ToIdentifier() override { return this; }
    const IdentExpr* ToIdentifier() const override { return this; }

    auto Addr() const { return addr_; }
    auto& Addr() { return addr_; }
    auto Name() const { return name_; }

private:
    friend class IRGen;
    std::string name_{};
    const Register* addr_{};
};


class LogicalExpr : public Expr
{
public:
    LogicalExpr(std::unique_ptr<Expr> l, Tag t, std::unique_ptr<Expr> r) :
        left_(std::move(l)), op_(t), right_(std::move(r)) {}

    void Accept(ASTVisitor* v);

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

    void Accept(ASTVisitor* v) override;
    const auto& Content() const { return content_; }
    auto& Width() { return width_; }
    const auto Width() const { return width_; }

private:
    std::string content_{};
    int width_{};
};


class UnaryExpr : public Expr
{
public:
    UnaryExpr(Tag t, std::unique_ptr<Expr> c) :
        op_(t), content_(std::move(c)) {}

    void Accept(ASTVisitor* v) override;
    bool IsLVal() const override { return op_ == Tag::_and; }
    bool IsConstant() const override;

    bool IsUnary() const override { return true; }
    UnaryExpr* ToUnary() override { return this; }
    const UnaryExpr* ToUnary() const override { return this; }

    Tag Op() const { return op_; }
    const auto& Content() const { return content_; }

private:
    friend class IRGen;
    friend class CodeChk;
    Tag op_;
    std::unique_ptr<Expr> content_{};
};

#endif // _EXPRESSION_H_
