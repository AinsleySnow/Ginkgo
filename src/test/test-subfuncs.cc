#include "utils/Scope.h"
#include "ast/Constant.h"
#include "ast/Object.h"

#define check_if(expr)                                      \
    if(!static_cast<bool>(expr)) {                          \
        printf("check %s failed at line %d, at %s in %s.",  \
            #expr, __LINE__, __func__, __FILE__);           \
        return false;}                                      \


bool test_evaluate(const Scope& s)
{
    check_if(s["a"]->ToConstant()->GetInt() == 2);
    check_if(s["b"]->ToConstant()->GetInt() == 6);
    check_if(s["c"]->ToConstant()->GetInt() == 2);
    check_if(s["d"]->ToConstant()->GetInt() == -1);
    check_if(s["e"]->ToConstant()->GetInt() == 5);
    check_if(s["f"]->ToConstant()->GetInt() == 7);
    check_if(s["g"]->ToConstant()->GetInt() == 24);
    check_if(s["h"]->ToConstant()->GetInt() == 21);
    check_if(s["i"]->ToConstant()->GetInt() == 0);
    check_if(s["j"]->ToConstant()->GetInt() == 3);
    check_if(s["l"]->ToConstant()->GetInt() == 3);
    // check_if(s["m"]->ToConstant()->GetInt() == 256);
    check_if(s["n"]->ToConstant()->GetInt() == -2);
    check_if(s["o"]->ToConstant()->GetInt() == 3);
    check_if(s["p"]->ToConstant()->GetInt() == -5);
    check_if(s["q"]->ToConstant()->GetInt() == 0);
    check_if(s["r"]->ToConstant()->GetInt() == 48);
    check_if(s["s"]->ToConstant()->GetInt() == 1);
    check_if(s["t"]->ToConstant()->GetInt() == 0);
    check_if(s["u"]->ToConstant()->GetInt() == 0);
    check_if(s["v"]->ToConstant()->GetInt() == 1);
    check_if(s["w"]->ToConstant()->GetInt() == 3);
    check_if(s["x"]->ToConstant()->GetInt() == 6);
    check_if(s["y"]->ToConstant()->GetInt() == 7);

    return true;
}

bool test_speccomb(const Scope& s)
{
    check_if(s["a"]->GetType()->ToArithm()->GetSize() == 1);
    check_if(s["b"]->GetType()->ToArithm()->GetSize() == 1);
    check_if(s["c"]->GetType()->ToArithm()->GetSize() == 1);

    check_if(s["d"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["e"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["f"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["g"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["h"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["i"]->GetType()->ToArithm()->GetSize() == 2);
    
    check_if(s["j"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["k"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["l"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["m"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["n"]->GetType()->ToArithm()->GetSize() == 4);

    check_if(s["o"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["p"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["q"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["r"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["s"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["t"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["u"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["v"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["w"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["x"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["y"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["z"]->GetType()->ToArithm()->GetSize() == 8);
    
    check_if(s["aa"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["ab"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["ac"]->GetType()->ToArithm()->GetSize() == 8);
    
    return true;
}

bool test_speclist(const Scope& s)
{
    check_if(s["a"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["b"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["c"]->GetType()->ToArithm()->GetSize() == 4);

    check_if(s["d"]->GetType()->ToArithm()->GetSize() == 1);
    check_if(s["e"]->GetType()->ToArithm()->GetSize() == 1);
    check_if(s["f"]->GetType()->ToArithm()->GetSize() == 1);
    check_if(s["g"]->GetType()->ToArithm()->GetSize() == 1);

    check_if(s["m"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["n"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["o"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["p"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["q"]->GetType()->ToArithm()->GetSize() == 8);

    check_if(s["r"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["s"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["t"]->GetType()->ToArithm()->GetSize() == 2);
    check_if(s["u"]->GetType()->ToArithm()->GetSize() == 2); 
    check_if(s["v"]->GetType()->ToArithm()->GetSize() == 2);

    check_if(s["w"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["x"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["y"]->GetType()->ToArithm()->GetSize() == 8);
    check_if(s["z"]->GetType()->ToArithm()->GetSize() == 8);

    check_if(s["aa"]->GetType()->ToArithm()->GetSize() == 4); 
    check_if(s["ab"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["ac"]->GetType()->ToArithm()->GetSize() == 4); 
    check_if(s["ad"]->GetType()->ToArithm()->GetSize() == 4);
    
    return true;
}

bool test_qualspec(const Scope& s)
{
    check_if(s["a"]->GetType()->GetQual().IsConstant());
    check_if(s["b"]->GetType()->GetQual().IsConstant());
    check_if(s["c"]->GetType()->GetQual().IsConstant() &&
             s["c"]->GetType()->GetQual().IsVolatile());
    check_if(s["d"]->GetType()->GetQual().IsConstant() &&
             s["d"]->GetType()->GetQual().IsVolatile());
    check_if(s["e"]->GetType()->GetQual().IsVolatile());
    check_if(s["f"]->GetType()->GetQual().IsAtomic() &&
             s["f"]->GetType()->GetQual().IsVolatile());
    check_if(s["f"]->GetType()->GetQual().IsAtomic() &&
             s["f"]->GetType()->GetQual().IsVolatile());

    check_if(s["a"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["b"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["c"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["d"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["e"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["f"]->GetType()->ToArithm()->GetSize() == 4);
    check_if(s["g"]->GetType()->ToArithm()->GetSize() == 4);
    
    return true;
}
