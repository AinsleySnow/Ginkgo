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

#define ExprGenerateHelper(op, first, second, st)   \
IR firstGen = first->Generate(st);                  \
IR secondGen = second->Generate(st);                \
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
