#include "UnaryExpr.h"

inline IR UnaryExpr::Inc_Dec_Sizeof_UnaryExpr(SymbolTable &st) const
{
    if (op == Tag::inc || op == Tag::dec)
    // unary-expr -> ++ unary-expr
    // unary-expr -> -- unary-expr
    {
        IR unaryGen = unaryExpr->Generate(st);
        if (unaryGen.Identifier.has_value())
        {
            if (unaryGen.Identifier.value().index() == 0) // Constant
            {
                Constant temp = std::get<0>(unaryGen.Identifier.value());
                unaryGen.Identifier = op == Tag::inc ? ++temp : --temp;
                return unaryGen;
            }
        }

        std::string lastVar = unaryGen.GetLastVar();
        if (lastVar[0] != '@')
            throw 1002;
        Quadruple quad{
            op == Tag::inc ? IROper::inc : IROper::dec,
            lastVar
        };

        unaryGen.Append(quad);
        return unaryGen;
    }
    else // unary-expr -> sizeof unary-expr
    {
        IR unaryGen = unaryExpr->Generate(st);
        std::string assignTo = st.GenerateTempVar(TypeSpec::uint64);
        std::string lastVarName = unaryGen.GetLastVar();

        Constant c{st[lastVarName].size, TypeSpec::uint64};
        Quadruple quad{
            IROper::assign,
            c,
            assignTo
        };

        unaryGen.Append(quad);
        return unaryGen;
    }
}

inline IR UnaryExpr::UnaryOp_CastExpr(SymbolTable &st) const
{
    IR castGen = castExpr->Generate(st);

    if (op == Tag::_and)
    // unary-expr -> & cast-expr
    {
        std::string lastVar = castGen.GetLastVar();
        Quadruple quad{
            IROper::getaddr,
            lastVar,
            st.GenerateTempVar(st[lastVar].specifier)
        };

        castGen.Append(quad);
        return castGen;
    }
    else if (op == Tag::star)
    // unary-expr -> * cast-expr
    {
        std::string lastVar = castGen.GetLastVar();
        Quadruple quad{
            IROper::dereference,
            lastVar,
            st.GenerateTempVar(st[lastVar].specifier)
        };

        castGen.Append(quad);
        return castGen;
    }
    else
    // unary-expr -> + cast-expr
    // unary-expr -> - cast-expr
    // unary-expr -> ~ cast-expr
    // unary-expr -> ! cast-expr
    {
        if (castGen.Identifier.has_value())
        {
            if (castGen.Identifier.value().index() == 0) // Constant
            {
                Constant temp = std::get<0>(castGen.Identifier.value());
                if (op != Tag::plus)
                {
                    castGen.Identifier = op == Tag::minus ? -temp : 
                        op == Tag::tilde ? ~temp : !temp;
                }
                return castGen;
            }
        }
        
        std::string lastVar = castGen.GetLastVar();
        IROper iroper = op == Tag::plus ? IROper::positive : 
                        op == Tag::minus ? IROper::negative : 
                        op == Tag::tilde ? IROper::bitwisenot : 
                        IROper::logicalnot;

        Quadruple quad{
            iroper,
            lastVar,
            st.GenerateTempVar(st[lastVar].specifier)
        };

        castGen.Append(quad);
        return castGen;
    }
}

inline IR UnaryExpr::Sizeof_Alignof_TypeName(SymbolTable &st) const
{
    if (op == Tag::_sizeof)
    // unary-expr -> sizeof ( type-name )
    {
        uint64_t typeSize = typeName->GetSize(st);
        std::string assignTo = st.GenerateTempVar(TypeSpec::uint64);
        Constant c{typeSize, TypeSpec::uint64};
        Quadruple quad{
            IROper::assign,
            c,
            assignTo
        };

        IR ir{};
        ir.Append(quad);
        return ir;
    }
    else // unary-expr -> alignof ( type-name )
    {
        uint64_t align = typeName->GetAlign(st);
        std::string assignTo = st.GenerateTempVar(TypeSpec::uint64);
        Constant c{align, TypeSpec::uint64};
        Quadruple quad{
            IROper::assign,
            c,
            assignTo
        };

        IR ir{};
        ir.Append(quad);
        return ir;
    }
}

IR UnaryExpr::Generate(SymbolTable &st) const
{
    if (postfixExpr)
        // unary-expr -> postfix-expr
        return postfixExpr->Generate(st);

    else if (unaryExpr &&
             (op == Tag::inc || op == Tag::dec || op == Tag::_sizeof))
        // unary-expr -> ++ unary-expr
        // unary-expr -> -- unary-expr
        // unary-expr -> sizeof unary-expr
        return Inc_Dec_Sizeof_UnaryExpr(st);

    else if (castExpr)
        // unary-expr -> unary-operator cast-expr
        return UnaryOp_CastExpr(st);

    else
        // unary-expr -> sizeof ( type-name )
        // unary-expr -> alignof ( type-name )
        return Sizeof_Alignof_TypeName(st);
}
