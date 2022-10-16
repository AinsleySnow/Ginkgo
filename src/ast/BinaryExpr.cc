#include "BinaryExpr.h"

IR BinaryExpr::Generate(SymbolTable &st) const
{
    if (!left)
        // binary-expr -> another-expr
        return right->Generate(st);
    else
    // binary-expr -> left-expr op right-expr
    {
        IR firstGen = left->Generate(st);
        IR secondGen = right->Generate(st);

        if (firstGen.Identifier.has_value() &&
            secondGen.Identifier.has_value())
        {
            if (firstGen.Identifier.value().index() == 0 &&
                secondGen.Identifier.value().index() == 0)
            {
                Constant temp1 = std::get<0>(
                    firstGen.Identifier.value());
                Constant temp2 = std::get<0>(
                    secondGen.Identifier.value());
                switch (op)
                {
                case Tag::plus:
                    irop = IROper::add; break;
                case Tag::minus:
                    irop = IROper::subtract; break;
                case Tag::star:
                    irop = IROper::multiple; break;
                case Tag::slash:
                    irop = IROper::divide; break;
                case Tag::_and:
                    irop = IROper::bitwiseand; break;
                case Tag::incl_or:
                    irop = IROper::bitwiseor; break;
                case Tag::logical_and:
                    irop = IROper::logicaland; break;
                case Tag::logical_or:
                    irop = IROper::logicalor; break;
                case Tag::lshift:
                    irop = IROper::lshift; break;
                case Tag::rshift:
                    irop = IROper::rshift; break;
                case Tag::lessthan:
                    irop = IROper::less; break;
                case Tag::greathan:
                    irop = IROper::great; break;
                case Tag::lessequal:
                    irop = IROper::lessequal; break;
                case Tag::greatequal:
                    irop = IROper::greatequal; break;
                case Tag::cap:
                    irop = IROper::bitwisexor; break;
                case Tag::equal:
                    irop = IROper::equal; break;
                case Tag::notequal:
                    irop = IROper::notequal; break;           
                default: return;
                }
                return firstGen;
            }
        }

        IROper irop{};
        switch (op)
        {
        case Tag::plus:
            irop = IROper::add; break;
        case Tag::minus:
            irop = IROper::subtract; break;
        case Tag::star:
            irop = IROper::multiple; break;
        case Tag::slash:
            irop = IROper::divide; break;
        case Tag::_and:
            irop = IROper::bitwiseand; break;
        case Tag::incl_or:
            irop = IROper::bitwiseor; break;
        case Tag::logical_and:
            irop = IROper::logicaland; break;
        case Tag::logical_or:
            irop = IROper::logicalor; break;
        case Tag::lshift:
            irop = IROper::lshift; break;
        case Tag::rshift:
            irop = IROper::rshift; break;
        case Tag::lessthan:
            irop = IROper::less; break;
        case Tag::greathan:
            irop = IROper::great; break;
        case Tag::lessequal:
            irop = IROper::lessequal; break;
        case Tag::greatequal:
            irop = IROper::greatequal; break;
        case Tag::cap:
            irop = IROper::bitwisexor; break;
        case Tag::equal:
            irop = IROper::equal; break;
        case Tag::notequal:
            irop = IROper::notequal; break; 
        default: return;
        }

        std::string firstAns, secondAns;
        if (firstGen.Identifier.has_value())
        {
            firstAns = st.GenerateTempVar(
                std::get<0>(firstGen.Identifier.value()).type);
            firstGen.Append(Quadruple(
                IROper::assign,
                firstGen.Identifier.value(),
                firstAns));
            firstGen.Identifier.reset();
        }
        else if (secondGen.Identifier.has_value())
        {
            secondAns = st.GenerateTempVar(
                std::get<0>(secondGen.Identifier.value()).type);
            firstGen.Append(Quadruple(
                IROper::assign,
                secondGen.Identifier.value(),
                secondAns));
        }
        else
        {
            firstAns = firstGen.GetLastVar();
            secondAns = secondGen.GetLastVar();
        }

        Quadruple quad{
            op,
            firstAns,
            secondAns,
            st.GenerateTempVar(
                std::max(st[firstAns].specifier,
                         st[secondAns].specifier))};

        firstGen.Join(secondGen);
        firstGen.Append(quad);
        return firstGen;
    }
}
