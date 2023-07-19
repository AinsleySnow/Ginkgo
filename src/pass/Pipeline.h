#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "pass/Pass.h"
#include <cassert>
#include <map>
#include <memory>

class Module;


class Pipeline
{
public:
    Pipeline(Module* m) : module_(m) {}

    template <class PASS, typename... IPASS,
    typename = std::enable_if_t<std::conjunction_v<std::is_integral<IPASS>...>>>
    void AddPass(int i, IPASS&&... dep)
    {
        auto index2pass = [&] (auto&& i) {
            assert(passes_.count(i));
            return passes_.at(i).get();
        };
        passes_[i] = std::make_unique<PASS>(module_, index2pass(std::forward<IPASS>(dep))...);
    }

    template <class PASS = Pass>
    PASS* GetPass(int i)
    {
        assert(passes_.at(i));
        auto p = dynamic_cast<PASS*>(passes_.at(i).get());
        assert(p);
        return p;
    }

    void ExecuteAll()
    {
        for (auto& pass : passes_)
            pass.second->Execute();
    }

private:
    Module* module_{};
    std::map<int, std::unique_ptr<Pass>> passes_{};
};

#endif // _PIPELINE_H_
