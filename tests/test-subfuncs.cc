#include "../utilities/SymbolTable.h"

#define check_if(expr)                                      \
    if(!static_cast<bool>(expr)) {                          \
        printf("check %s failed at line %d, at %s in %s.",  \
            #expr, __LINE__, __func__, __FILE__);           \
        return false;}                                      \


bool test_evaluate(const SymbolTable& st)
{
    check_if(st["a"].initial->GetConstant().GetU64() == 2);
    check_if(st["b"].initial->GetConstant().GetU64() == 6);
    check_if(st["c"].initial->GetConstant().GetU64() == 2);
    check_if(st["d"].initial->GetConstant().GetU64() == -1);
    check_if(st["e"].initial->GetConstant().GetU64() == 5);
    check_if(st["f"].initial->GetConstant().GetU64() == 7);
    check_if(st["g"].initial->GetConstant().GetU64() == 24);
    check_if(st["h"].initial->GetConstant().GetU64() == 21);
    check_if(st["i"].initial->GetConstant().GetU64() == 0);
    check_if(st["j"].initial->GetConstant().GetU64() == 3);
    check_if(st["l"].initial->GetConstant().GetU64() == 3);
    // check_if(st["m"].initial->GetConstant().GetU64() == 256);
    check_if(st["n"].initial->GetConstant().GetU64() == -2);
    check_if(st["o"].initial->GetConstant().GetU64() == 3);
    check_if(st["p"].initial->GetConstant().GetU64() == -5);
    check_if(st["q"].initial->GetConstant().GetU64() == 0);
    check_if(st["r"].initial->GetConstant().GetU64() == 48);
    check_if(st["s"].initial->GetConstant().GetU64() == 1);
    check_if(st["t"].initial->GetConstant().GetU64() == 0);
    check_if(st["u"].initial->GetConstant().GetU64() == 0);
    check_if(st["v"].initial->GetConstant().GetU64() == 1);
    check_if(st["w"].initial->GetConstant().GetU64() == 3);
    check_if(st["x"].initial->GetConstant().GetU64() == 6);
    check_if(st["y"].initial->GetConstant().GetU64() == 7);

    return true;
}

bool test_speccomb(const SymbolTable& st)
{
    check_if(st.GetSymbol("a").specifier == Type::int8);
    check_if(st.GetSymbol("b").specifier == Type::uint8);
    check_if(st.GetSymbol("c").specifier == Type::int8);

    check_if(st.GetSymbol("d").specifier == Type::int16);
    check_if(st.GetSymbol("e").specifier == Type::int16);
    check_if(st.GetSymbol("f").specifier == Type::uint16);
    check_if(st.GetSymbol("g").specifier == Type::int16);
    check_if(st.GetSymbol("h").specifier == Type::uint16);
    check_if(st.GetSymbol("i").specifier == Type::int16);
    
    check_if(st.GetSymbol("j").specifier == Type::int32);
    check_if(st.GetSymbol("k").specifier == Type::int32);
    check_if(st.GetSymbol("l").specifier == Type::uint32);
    check_if(st.GetSymbol("m").specifier == Type::int32);
    check_if(st.GetSymbol("n").specifier == Type::uint32);

    check_if(st.GetSymbol("o").specifier == Type::int64);
    check_if(st.GetSymbol("p").specifier == Type::uint64);
    check_if(st.GetSymbol("q").specifier == Type::int64);
    check_if(st.GetSymbol("r").specifier == Type::int64);
    check_if(st.GetSymbol("s").specifier == Type::uint64);
    check_if(st.GetSymbol("t").specifier == Type::int64);
    check_if(st.GetSymbol("u").specifier == Type::int64);
    check_if(st.GetSymbol("v").specifier == Type::uint64);
    check_if(st.GetSymbol("w").specifier == Type::int64);
    check_if(st.GetSymbol("x").specifier == Type::int64);
    check_if(st.GetSymbol("y").specifier == Type::uint64);
    check_if(st.GetSymbol("z").specifier == Type::int64);
    
    check_if(st.GetSymbol("aa").specifier == Type::float32);
    check_if(st.GetSymbol("ab").specifier == Type::float64);
    check_if(st.GetSymbol("ac").specifier == Type::float64);
    
    return true;
}

bool test_speclist(const SymbolTable& st)
{
    check_if(st["a"].specifier == Type::int32);
    check_if(st["b"].specifier == Type::int32);
    check_if(st["c"].specifier == Type::int32);

    check_if(st["d"].specifier == Type::int8);
    check_if(st["e"].specifier == Type::int8);
    check_if(st["f"].specifier == Type::int8);
    check_if(st["g"].specifier == Type::int8);

    check_if(st["m"].specifier == Type::int64);
    check_if(st["n"].specifier == Type::int64);
    check_if(st["o"].specifier == Type::int64);
    check_if(st["p"].specifier == Type::int64);
    check_if(st["q"].specifier == Type::int64);

    check_if(st["r"].specifier == Type::int16);
    check_if(st["s"].specifier == Type::int16);
    check_if(st["t"].specifier == Type::int16);
    check_if(st["u"].specifier == Type::int16); 
    check_if(st["v"].specifier == Type::int16);

    check_if(st["w"].specifier == Type::float64);
    check_if(st["x"].specifier == Type::float64);
    check_if(st["y"].specifier == Type::float64);
    check_if(st["z"].specifier == Type::float64);

    check_if(st["aa"].specifier == Type::float32); 
    check_if(st["ab"].specifier == Type::float32);
    check_if(st["ac"].specifier == Type::float32); 
    check_if(st["ad"].specifier == Type::float32);
    
    return true;
}

bool test_qualspec(const SymbolTable& st)
{
    check_if(st["a"].quailfier == 1);
    check_if(st["b"].quailfier == 1);
    check_if(st["c"].quailfier == 5);
    check_if(st["d"].quailfier == 5);
    check_if(st["e"].quailfier == 4);
    check_if(st["f"].quailfier == 12);
    check_if(st["g"].quailfier == 9);

    check_if(st["a"].specifier == Type::int32);
    check_if(st["b"].specifier == Type::int32);
    check_if(st["c"].specifier == Type::int32);
    check_if(st["d"].specifier == Type::int32);
    check_if(st["e"].specifier == Type::int32);
    check_if(st["f"].specifier == Type::int32);
    check_if(st["g"].specifier == Type::int32);
    
    return true;
}
