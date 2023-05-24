#ifndef _SYSV_CONV_H_
#define _SYSV_CONV_H_

#include "visitir/x64.h"
#include <map>
#include <memory>
#include <vector>

class IROperand;
class FuncType;


class SysVConv
{
public:
    // complex_x87 is commented since Ginkgo
    // doesn't support complex number yet
    enum class ParamClass
    {
        integer, sse, sseup, x87, x87up,
        /*complex_x87,*/ no_class, memory 
    };

    SysVConv(const FuncType*);
    const x64* PlaceOfArgv(int) const;
    auto StackSize() const { return stacksize_; }


private:
    void CheckParamClass(const IRType*);
    void Emplace(int, const IRType*, RegTag);
    RegTag GetIntReg();
    RegTag GetVecReg();

    // number of used integer and vector registers
    int intcnt_{};
    int veccnt_{};

    // class of every eight bytes
    std::vector<ParamClass> wordclass_{};
    // how many bytes of parameters are passed in the stack
    size_t stacksize_{};

    // to where does the xth parameter map?
    // no map will be created if some parameter
    // is to be placed into somewhere of the memory
    std::map<int, std::unique_ptr<x64>> argvs_{};
};

#endif // _SYSV_CONV_H_
