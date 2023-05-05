#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "pass/Pass.h"
#include <memory>
#include <vector>


class Pipeline
{
public:
    Pipeline(std::vector<std::unique_ptr<Pass>> p) :
        passes_(std::move(p)) {}

    void ExecuteAll()
    {
        for (auto& pass : passes_)
            pass->Execute();
    }

private:
    std::vector<std::unique_ptr<Pass>> passes_{};
};

#endif // _PIPELINE_H_
