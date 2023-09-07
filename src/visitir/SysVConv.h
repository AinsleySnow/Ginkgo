#ifndef _SYSV_CONV_H_
#define _SYSV_CONV_H_

#include "visitir/x64.h"
#include <map>
#include <memory>
#include <vector>

class IROperand;
class IRType;
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

    static bool HasFloat(const IRType*, size_t, size_t, size_t);

    SysVConv(const FuncType*);
    SysVConv(const FuncType*, const std::vector<const IROperand*>*);

    void MapArgv();
    const x64* PlaceOfArgv(int) const;
    long OffsetOfArgv(int) const;
    std::unique_ptr<x64>&& ExtractArgv(int);

    static RegTag Index2IntTag(int);
    static RegTag Index2VecTag(int);
    std::pair<int, int> CountRegs() const;
    auto StackSize() const { return stacksize_; }
    auto Padding() const { return padding_; }


private:
    void AlignStackBy(size_t, size_t);
    void CheckParamClass(const IRType*, bool = false);
    const IRType* GetType(int) const;
    void Emplace(int, const IRType*, RegTag);
    void Emplace(x64Heter*, RegTag);
    RegTag GetIntReg();
    RegTag GetVecReg();

    // number of used integer and vector registers
    int intcnt_{};
    int veccnt_{};

    const FuncType* functype_{};
    const std::vector<const IROperand*>* paramlist_{};
    // class of every eight bytes
    std::vector<ParamClass> wordclass_{};
    // how many bytes of parameters are passed in the stack
    size_t stacksize_{};
    size_t padding_{};

    // to where does the xth parameter map?
    // no map will be created if some parameter
    // is to be placed into somewhere of the memory
    std::map<int, std::unique_ptr<x64>> argvs_{};
    // if an argv is passed on the stack, what's its
    // offset relative to rsp?
    std::map<int, long> memoffset_{};
};

#endif // _SYSV_CONV_H_
