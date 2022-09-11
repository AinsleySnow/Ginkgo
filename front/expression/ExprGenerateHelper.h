/* 
 * Note: The macro defined below is for avoiding
 * retyping code with same structure in implementations 
 * of expression nodes. Since first and second are both 
 * unique pointers, and that the Generate method has been 
 * marked with const, using template here is not that 
 * appropriate. 
 */

#ifndef _EXPR_GENERATE_HELPER_H_
#define _EXPR_GENERATE_HELPER_H_

#define DeclareHelper(first, second)                \
IR firstGen = first->Generate(st);                  \
IR secondGen = second->Generate(st);                \
                                                    \
if (firstGen.Identifier.has_value() &&              \
    secondGen.Identifier.has_value())               \
{                                                   \
    if (firstGen.Identifier.value().index() == 0 && \
        secondGen.Identifier.value().index() == 0)  \
    {                                               \
        Constant temp1 = std::get<0>(               \
            firstGen.Identifier.value());           \
        Constant temp2 = std::get<0>(               \
            secondGen.Identifier.value())          \


#define OperationHelper(op_sym, irop, irop_value)       \
        if (irop == irop_value)                         \
            firstGen.Identifier = temp1 op_sym temp2;   \


#define ExprGenerateHelper(op, st)                  \
        return firstGen;                            \
    }                                               \
}                                                   \
                                                    \
std::string firstAns = firstGen.GetLastVar();       \
std::string secondAns = secondGen.GetLastVar();     \
                                                    \
Quadruple quad{                                     \
    op,                                             \
    firstAns,                                       \
    secondAns,                                      \
    st.GenerateTempVar(                             \
        std::max(st[firstAns].specifier,            \
                 st[secondAns].specifier))};        \
                                                    \
firstGen.Join(secondGen);                           \
firstGen.Append(quad);                              \
return firstGen

#endif // _EXPR_GENERATE_HELPER_H_
