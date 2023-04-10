#ifndef _EVALUATOR_H_
#define _EVALUATOR_H_

#include "ast/Tag.h"
#include "IR/IROperand.h"
#include "utils/Pool.h"


class Evaluator
{
public:
    static const IROperand* EvalBinary(Pool<IROperand>*, Tag, const IROperand*, const IROperand*);
    static const IROperand* EvalUnary(Pool<IROperand>*, Tag, const IROperand*);

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
