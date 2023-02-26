#ifndef _EVALUATOR_H_
#define _EVALUATOR_H_

#include "ast/Tag.h"

class Function;
class IROperand;


class Evaluator
{
public:
    static const IROperand* EvalBinary(Function*, Tag, const IROperand*, const IROperand*);
    static const IROperand* EvalUnary(Function*, Tag, const IROperand*);

private:
    template <typename RET, typename LHS, typename RHS>
    static RET Calc(Tag, LHS, RHS);
    template <typename LHS, typename RHS>
    static double Calc(Tag, LHS, RHS);
    template <typename RET, typename NUM>
    static RET Calc(Tag, NUM);

    static bool IsLogicalTag(Tag);
};

#endif // _EVALUATOR_H_
