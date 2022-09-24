#include "CastExpr.h"

IR CastExpr::Generate(SymbolTable& st) const 
{
    if (unaryExpr)
    // cast-expr -> unary-expr
        return unaryExpr->Generate(st);
    else
    // cast-expr -> ( type-name ) cast-expr
    {
        Type type = typeName->GetSpec(st);
        IR castGen = castExpr->Generate(st);
        if (castGen.Identifier.has_value())
        {
            if (castGen.Identifier.value().index() == 0) // Constant
            {
                std::get<0>(castGen.Identifier.value()).type = type;
                return castGen;
            }
        }
        Quadruple quad = {
            IROper::assign,
            castGen.GetLastVar(),
            st.GenerateTempVar(type)
        };
        castGen.Append(quad);
        return castGen;
    }
}
