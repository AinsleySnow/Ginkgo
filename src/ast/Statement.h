#ifndef _STATEMENT_H_
#define _STATEMENT_H_

class BasicBlock;
class Instr;
class BrInstr;
class Declaration;
class ASTVisitor;

#include "ast/Expr.h"
#include "ast/Declaration.h"
#include <list>
#include <memory>
#include <string>
#include <vector>


class Statement
{
public:
    virtual ~Statement() {}

    virtual void Accept(ASTVisitor*) {}

    const auto& NextList() const { return nextlist_; }
    auto& NextList() { return nextlist_; }

    void PushBrInstr(Instr*);

protected:
    std::list<BrInstr*> nextlist_{};
};

class IterStmt : public Statement
{
protected:
    friend class IRGen;
    virtual void Accept(ASTVisitor*) {}

    BasicBlock* continuepoint_{};
};


class BreakStmt : public Statement
{
public:
    void Accept(ASTVisitor*) override;
};


class CaseStmt : public Statement
{
public:
    CaseStmt(std::unique_ptr<Expr> c) : const_(std::move(c)) {}
    void Accept(ASTVisitor* v) override;
    void AddStatement(std::unique_ptr<Statement> s) { stmt_ = std::move(s); }

private:
    friend class IRGen;
    std::unique_ptr<Expr> const_{};
    std::unique_ptr<Statement> stmt_{};
};


class CompoundStmt : public Statement
{
public:
    void Accept(ASTVisitor* v) override;
    void Append(std::unique_ptr<Statement> stmt);

private:
    friend class IRGen;
    std::vector<std::unique_ptr<Statement>> stmtlist_{};
};


class ContinueStmt : public Statement
{
public:
    void Accept(ASTVisitor* v) override;
};


class DeclStmt : public Statement
{
public:
    DeclStmt(std::unique_ptr<Declaration> d) :
        decl_(std::move(d)) {}
    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Declaration> decl_{};
};


class DoWhileStmt : public IterStmt
{
public:
    DoWhileStmt(std::unique_ptr<Expr> e, std::unique_ptr<Statement> s) :
        expr_(std::move(e)), stmt_(std::move(s)) {}
    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Expr> expr_{};
    std::unique_ptr<Statement> stmt_{};
};


class ExprStmt : public Statement
{
public:
    ExprStmt(std::unique_ptr<Expr> e) : expr_(std::move(e)) {}
    void Accept(ASTVisitor* v) override;

    bool Empty() const { return expr_ == nullptr; }

private:
    friend class IRGen;
    std::unique_ptr<Expr> expr_{};
};


class ForStmt : public IterStmt
{
public:
    ForStmt(std::unique_ptr<Expr> init, std::unique_ptr<Expr> cond,
        std::unique_ptr<Expr> inc, std::unique_ptr<Statement> s) :
            init_(std::move(init)), condition_(std::move(cond)),
            increment_(std::move(inc)), body_(std::move(s)) {}

    ForStmt(std::unique_ptr<Statement> decl, std::unique_ptr<Expr> cond,
        std::unique_ptr<Expr> inc, std::unique_ptr<Statement> s) : 
            decl_(std::move(decl)), condition_(std::move(cond)),
            increment_(std::move(inc)), body_(std::move(s)) {}

    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Expr> init_{};
    std::unique_ptr<Statement> decl_{};
    std::unique_ptr<Expr> condition_{};
    std::unique_ptr<Expr> increment_{};
    std::unique_ptr<Statement> body_{};
};


class GotoStmt : public Statement
{
public:
    GotoStmt(const std::string& i) : ident_(i) {}
    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::string ident_{};
};


class IfStmt : public Statement
{
public:
    IfStmt(std::unique_ptr<Expr> e, std::unique_ptr<Statement> t) :
        expr_(std::move(e)), true_(std::move(t)) {}
    IfStmt(std::unique_ptr<Expr> e, std::unique_ptr<Statement> t,
        std::unique_ptr<Statement> f) :
        expr_(std::move(e)), true_(std::move(t)), false_(std::move(f)) {}

    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Expr> expr_{};
    std::unique_ptr<Statement> true_{};
    std::unique_ptr<Statement> false_{};
};


class LabelStmt : public Statement
{
public:
    LabelStmt(const std::string& l) : label_(l) {}
    void Accept(ASTVisitor* v) override;
    void AddStatement(std::unique_ptr<Statement> s) { stmt_ = std::move(s); }

private:
    friend class IRGen;
    std::string label_{};
    std::unique_ptr<Statement> stmt_{};
};


class RetStmt : public Statement
{
public:
    RetStmt() {}
    RetStmt(std::unique_ptr<Expr> e) :
        retvalue_(std::move(e)) {}
    void Accept(ASTVisitor* v);

private:
    friend class IRGen;
    std::unique_ptr<Expr> retvalue_{};
};


class SwitchStmt : public Statement
{
public:
    SwitchStmt(std::unique_ptr<Expr> e, std::unique_ptr<Statement> s) :
        expr_(std::move(e)), stmt_(std::move(s)) {}
    void Accept(ASTVisitor* v);

private:
    friend class IRGen;
    std::unique_ptr<Expr> expr_{};
    std::unique_ptr<Statement> stmt_{};
};


class TransUnit : public Statement
{
public:
    void AddDecl(std::unique_ptr<DeclStmt>);
    void Accept(ASTVisitor*);

private:
    friend class IRGen;
    std::vector<std::unique_ptr<DeclStmt>> declist_{};
};


class WhileStmt : public IterStmt
{
public:
    WhileStmt(std::unique_ptr<Expr> e, std::unique_ptr<Statement> s) :
        expr_(std::move(e)), stmt_(std::move(s)) {}
    void Accept(ASTVisitor* v) override;

private:
    friend class IRGen;
    std::unique_ptr<Expr> expr_{};
    std::unique_ptr<Statement> stmt_{};
};


#endif // _STATEMENT_H_
