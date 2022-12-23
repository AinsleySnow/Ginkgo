#ifndef _FUNC_TYPE_H_
#define _FUNC_TYPE_H_

#include "CType.h"
#include <memory>
#include <vector>


class CFuncType : public CType
{
public:
    const CType* GetReturnType() const { return return_.get(); }
    size_t GetParaCount() const { return paramlist_.size(); }
    void SetReturnType(std::shared_ptr<CType> t) { return_ = t; }
    void AddParam(std::shared_ptr<CType> t) { paramlist_.push_back(t); }
    void SetVariadic(bool v) { variadic_ = v; }

    const auto cbegin() const { return paramlist_.cbegin(); }
    const auto cend() const { return paramlist_.cend(); }

    bool IsDerived() const override { return true; }
    bool IsComplete() const override { return true; }

    CFuncType* ToFunc() override { return this; }


private:
    bool variadic_{};
    std::vector<std::shared_ptr<CType>> paramlist_{};
    std::shared_ptr<CType> return_{};
};

#endif // _FUNC_TYPE_H_
