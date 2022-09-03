#include "AssignExpr.h"

IR AssignExpr::Generate(SymbolTable& st) const
{
    if (condExpr)
    // assign-expr -> cond-expr
        return condExpr->Generate(st);
    else
    // assign-expr -> unary-expr assign-op assign-expr
    {
        IR ueGen = unaryExpr->Generate(st);
        IR assignGen = assignExpr->Generate(st);
        IROper irop;

        switch (op)
        {
        case Tag::equal:
            irop = IROper::assign;
            break;
        case Tag::mul_assign:
            irop = IROper::multiple;
            break;
        case Tag::div_assign:
            irop = IROper::divide;
            break;
        case Tag::mod_assign:
            irop = IROper::mod;
            break;
        case Tag::add_assign:
            irop = IROper::add;
            break;
        case Tag::sub_assign:
            irop = IROper::subtract;
            break;
        case Tag::left_assign:
            irop = IROper::lshift;
            break;
        case Tag::right_assign:
            irop = IROper::rshift;
            break;
        case Tag::and_assign:
            irop = IROper::bitwiseand;
            break;
        case Tag::xor_assign:
            irop = IROper::bitwisexor;
            break;
        case Tag::or_assign:
            irop = IROper::bitwiseor;
            break;
        default:
            break;
        }

        std::string assignTo = ueGen.GetLastVar();
        std::string multiplier = assignGen.GetLastVar();
        Quadruple quad { irop, assignTo };
        if (irop == IROper::assign)
            quad.arg1 = multiplier;
        else
        {
            quad.arg1 = assignTo;
            quad.arg2 = multiplier;
        }

        ueGen.Join(assignGen);
        ueGen.Append(quad);
        return ueGen;
    }
}
