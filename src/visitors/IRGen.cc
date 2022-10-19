#include "IRGen.h"
#include "ast/AssignExpr.h"
#include "ast/BinaryExpr.h"
#include "ast/CondExpr.h"
#include "ast/Constant.h"
#include "ast/Label.h"
#include "ast/Object.h"
#include "ast/UnaryExpr.h"
#include "messages/Error.h"
#include <cassert>


IROper IRGen::Tag2Op(Tag t) const
{
    switch (t)
    {
    case Tag::plus: return IROper::add;
    case Tag::minus: return IROper::subtract;
    case Tag::star: return IROper::multiple;
    case Tag::slash: return IROper::divide;
    case Tag::percent: return IROper::mod;
    case Tag::cap: return IROper::bitwisexor;
    case Tag::_and: return IROper::bitwiseand;
    case Tag::_or: return IROper::bitwiseor;
    case Tag::tilde: return IROper::bitwisenot;
    case Tag::exclamation: return IROper::logicalnot;
    case Tag::lshift: return IROper::lshift;
    case Tag::rshift: return IROper::rshift;
    case Tag::greathan: return IROper::great;
    case Tag::lessthan: return IROper::less;
    case Tag::lessequal: return IROper::lessequal;
    case Tag::greatequal: return IROper::greatequal;
    case Tag::equal: return IROper::equal;
    case Tag::notequal: return IROper::notequal;
    case Tag::incl_or: return IROper::bitwiseor;
    case Tag::logical_and: return IROper::logicaland;
    case Tag::logical_or: return IROper::logicalor;
    case Tag::add_assign: return IROper::add;
    case Tag::and_assign: return IROper::bitwiseand;
    case Tag::div_assign: return IROper::divide;
    case Tag::assign: return IROper::assign;
    case Tag::left_assign: return IROper::lshift;
    case Tag::mod_assign: return IROper::mod;
    case Tag::mul_assign: return IROper::multiple;
    case Tag::or_assign: return IROper::bitwiseor;
    case Tag::right_assign: return IROper::rshift;
    case Tag::sub_assign: return IROper::subtract;
    case Tag::xor_assign: return IROper::bitwisexor;
    }

    assert(false);
}


inline const auto IRGen::GetLastestTempVar(unsigned long order = 0) const
{
    return ir_->GetScope()->GetLastestTempVar(order);
}

inline const auto IRGen::GenerateTempVar()
{
    return ir_->GetScope()->GenerateTempVar();
}


void IRGen::VisitAssignExpr(AssignExpr* assign)
{
    if (!assign->left_->IsLVal())
    {
        Error(ErrorId::needlval);
        return;
    }

    Quadruple quad;

    if (assign->left_->ToIdentifier())
    {
        assign->right_->Accept(this);
        quad.arg3 = assign->left_->ToIdentifier();
        quad.arg1 = quad.arg3;
        quad.arg2 = GetLastestTempVar();
    }
    else
    {
        assign->left_->Accept(this);
        quad.arg1 = GetLastestTempVar();
        assign->right_->Accept(this);
        quad.arg2 = GetLastestTempVar();
        quad.arg3 = std::get<2>(quad.arg1);
    }

    quad.op = Tag2Op(assign->op_);
    ir_->Append(quad);
}


void IRGen::VisitBinaryExpr(BinaryExpr* bin)
{
    Quadruple quad, label;

    if (bin->left_->ToConstant())
    {
        auto constant = bin->left_->ToConstant();
        if (bin->op_ == Tag::logical_and || bin->op_ == Tag::logical_or)
        {
            quad.arg1 = !!(constant->GetInt());
            quad.arg3 = GenerateTempVar();
            quad.op = IROper::assign;
            ir_->Append(quad);
            return;
        }

        if (constant->GetType()->IsFloat())
            quad.arg1 = constant->GetFloat();
        else
            quad.arg1 = constant->GetInt();
    }
    else
    {
        bin->left_->Accept(this);
        auto temp = GetLastestTempVar();
        if (!(temp->GetType()->ToArithm()) || !(temp->GetType()->ToPtr()))
        {
            Error(ErrorId::cannotconcatenate);
            return;
        }
        quad.arg1 = temp;
    }

    if (bin->op_ == Tag::logical_and || bin->op_ == Tag::logical_or)
    {
        label.op = IROper::label;
        label.arg3 = ir_->GetScope()->GenerateTempLabel();
        Quadruple jump {
            bin->op_ == Tag::logical_and ? IROper::jmpfalse : IROper::jmptrue,
            quad.arg1,
            label.arg3
        };
        ir_->Append(jump);
    }

    if (bin->right_->ToConstant())
    {
        auto constant = bin->right_->ToConstant();
        if (constant->GetType()->IsFloat())
            quad.arg2 = constant->GetFloat();
        else
            quad.arg2 = constant->GetInt();
    }
    else
    {
        bin->right_->Accept(this);
        auto temp = GetLastestTempVar();
        if (!(temp->GetType()->ToArithm()) || !(temp->GetType()->ToPtr()))
        {
            Error(ErrorId::cannotconcatenate);
            return;
        }
        quad.arg2 = GetLastestTempVar();
    }

    quad.arg3 = GenerateTempVar();
    quad.op = Tag2Op(bin->op_);
    ir_->Append(quad);
    if (bin->op_ == Tag::logical_and || bin->op_ == Tag::logical_or)
        ir_->Append(label);
}


void IRGen::VisitCondExpr(CondExpr* cond)
{
    auto falselabel = ir_->GetScope()->GenerateTempLabel();
    auto endlabel = ir_->GetScope()->GenerateTempLabel();

    cond->cond_->Accept(this);
    ir_->Append(Quadruple(
        IROper::jmpfalse, GetLastestTempVar(), falselabel));

    cond->true_->Accept(this);
    ir_->Append(Quadruple(IROper::jmp, endlabel));

    ir_->Append(Quadruple(IROper::label, falselabel));
    cond->false_->Accept(this);
    ir_->Append(Quadruple(IROper::label, endlabel));
}


void IRGen::VisitUnaryExpr(UnaryExpr* unary)
{
    Quadruple quad;
    
    if (unary->content_->ToConstant())
    {
        auto temp = unary->content_->ToConstant();
        if (temp->GetType()->IsFloat())
            quad.arg1 = temp->GetFloat();
        else
            quad.arg1 = temp->GetInt();
    }
    else
    {
        unary->Accept(this);
        quad.arg1 = GetLastestTempVar();
    }

    quad.op = Tag2Op(unary->op_);
    quad.arg3 = GenerateTempVar();
}
