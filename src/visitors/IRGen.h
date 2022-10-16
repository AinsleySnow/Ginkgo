#ifndef _IR_GENERATOR_H_
#define _IR_GENERATOR_H_

#include "Visitor.h"
#include "utilities/IR.h"
#include <memory>

class IRGenerator : public Visitor
{
private:
    std::unique_ptr<IR> ir_{};

public:
    
};

#endif // _IR_GENERATOR_H_
