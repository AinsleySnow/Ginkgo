#include "../utilities/SymbolTable.h"

#define check_if(expr)                                      \
    if(!static_cast<bool>(expr)) {                          \
        printf("check %s failed at line %d, at %s in %s.",  \
            #expr, __LINE__, __func__, __FILE__);           \
        return false;}                                      \


bool test_speccomb(const SymbolTable& st)
{
    check_if(st.GetSymbol("a").specifier == TypeSpec::int8);
    check_if(st.GetSymbol("b").specifier == TypeSpec::uint8);
    check_if(st.GetSymbol("c").specifier == TypeSpec::int8);

    check_if(st.GetSymbol("d").specifier == TypeSpec::int16);
    check_if(st.GetSymbol("e").specifier == TypeSpec::int16);
    check_if(st.GetSymbol("f").specifier == TypeSpec::uint16);
    check_if(st.GetSymbol("g").specifier == TypeSpec::int16);
    check_if(st.GetSymbol("h").specifier == TypeSpec::uint16);
    check_if(st.GetSymbol("i").specifier == TypeSpec::int16);
    
    check_if(st.GetSymbol("j").specifier == TypeSpec::int32);
    check_if(st.GetSymbol("k").specifier == TypeSpec::int32);
    check_if(st.GetSymbol("l").specifier == TypeSpec::uint32);
    check_if(st.GetSymbol("m").specifier == TypeSpec::int32);
    check_if(st.GetSymbol("n").specifier == TypeSpec::uint32);

    check_if(st.GetSymbol("o").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("p").specifier == TypeSpec::uint64);
    check_if(st.GetSymbol("q").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("r").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("s").specifier == TypeSpec::uint64);
    check_if(st.GetSymbol("t").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("u").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("v").specifier == TypeSpec::uint64);
    check_if(st.GetSymbol("w").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("x").specifier == TypeSpec::int64);
    check_if(st.GetSymbol("y").specifier == TypeSpec::uint64);
    check_if(st.GetSymbol("z").specifier == TypeSpec::int64);
    
    check_if(st.GetSymbol("aa").specifier == TypeSpec::float32);
    check_if(st.GetSymbol("ab").specifier == TypeSpec::float64);
    check_if(st.GetSymbol("ac").specifier == TypeSpec::float64);
    
    return true;
}
