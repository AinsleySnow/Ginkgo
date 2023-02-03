#include "visitast/CodeChk.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "messages/Error.h"
#include <memory>


CArithmType CodeChk::MatchArithmType(const CArithmType* lhs, const CArithmType* rhs)
{
    if (lhs->operator>(*rhs))
        return *lhs;
    else if (lhs->operator<(*rhs))
        return *rhs;
    else if (lhs->IsFloat())      
        return *lhs;
    else
    {
        if (lhs->IsUnsigned()) return *lhs;
        else return *rhs;
    }
}


void CodeChk::VisitAssignExpr(AssignExpr* assign)
{
    assign->left_->Accept(this);
    assign->right_->Accept(this);
    auto arithm = static_cast<const CArithmType*>(assign->left_->RawType());
    assign->Type() = std::make_unique<CArithmType>(*arithm);
}

void CodeChk::VisitBinaryExpr(BinaryExpr* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);
    auto lhs = static_cast<const CArithmType*>(bin->left_->RawType());
    auto rhs = static_cast<const CArithmType*>(bin->right_->RawType());
    bin->Type() = std::make_unique<CArithmType>(MatchArithmType(lhs, rhs));
}

void CodeChk::VisitCondExpr(CondExpr* cond)
{
    cond->cond_->Accept(this);
    cond->true_->Accept(this);
    cond->false_->Accept(this);
    auto lhs = static_cast<const CArithmType*>(cond->true_->RawType());
    auto rhs = static_cast<const CArithmType*>(cond->false_->RawType());
    cond->Type() = std::make_unique<CArithmType>(MatchArithmType(lhs, rhs));
}

void CodeChk::VisitConstant(ConstExpr* constant)
{
    // nothing to do for now
}

void CodeChk::VisitFuncDef(FuncDef* def)
{
    // nothing to do for now
}

void CodeChk::VisitLogicalExpr(LogicalExpr* expr)
{
    expr->left_->Accept(this);
    expr->right_->Accept(this);
    expr->Type() = std::make_unique<CArithmType>(TypeTag::uint8);
}

void CodeChk::VisitObjDef(ObjDef*)
{
    // nothing to do for now
}

void CodeChk::VisitUnaryExpr(UnaryExpr* unary)
{
    // only consider arithmetic type for now
    unary->content_->Accept(this);
    auto arithm = static_cast<const CArithmType*>(unary->content_->RawType());
    unary->type_ = std::make_unique<CArithmType>(*arithm);
}


void CodeChk::VisitForStmt(ForStmt*)
{

}

void CodeChk::VisitIfStmt(IfStmt*)
{

}

void CodeChk::VisitWhileStmt(WhileStmt*)
{

}
