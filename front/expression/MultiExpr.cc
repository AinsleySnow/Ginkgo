#include "MultiExpr.h"

IR MultiExpr::Generate(SymbolTable& st) const 
{
    if (castExpr)
    // multiplicative-expr -> cast-expr
        return castExpr->Generate(st);
    else
    // multiplicative-expr -> multiplicative-expr * cast-expr
    // multiplicative-expr -> multiplicative-expr / cast-expr
    // multiplicative-expr -> multiplicative-expr % cast-expr
    {
        IR multiGen = multiExpr->Generate(st);
        IR castGen = castExpr->Generate(st);
        std::string multiAns = multiGen.GetLastVar();
        std::string castAns = castGen.GetLastVar();

        Quadruple quad{
            op == Tag::star ? IROper::multiple :
            op == Tag::slash ? IROper::divide :
            IROper::mod,
            multiAns, castAns,
            st.GenerateTempVar(
                std::max(st[multiAns].specifier, 
                         st[castAns].specifier))
        };

        multiGen.Join(castGen);
        multiGen.Append(quad);
        return multiGen;
    }
}
