#ifndef _PASS_H_
#define _PASS_H_

class Module;


class Pass
{
public:
    ~Pass() {}

    virtual void Init(Module*) = 0;
    virtual void Execute() = 0;

    auto& CurModule() { return module_; }
    auto CurModule() const { return module_; }

private:
    Module* module_{};
};

#endif // _PASS_H_
