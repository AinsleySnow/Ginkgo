#include "CastExpr.h"

IR CastExpr::Generate(SymbolTable& st) const 
{
    if (unaryExpr)
    // cast-expr -> unary-expr
        return unaryExpr->Generate(st);
    else
    // cast-expr -> ( type-name ) cast-expr
    {
        TypeSpec type = typeName->GetSpec(st);
        IR castGen = castExpr->Generate(st);
        Quadruple quad = {
            IROper::assign,
            castGen.GetLastVar(),
            st.GenerateTempVar(type)
        };
        castGen.Append(quad);
        return castGen;
    }
}
