#ifndef _IROPERAND_H_
#define _IROPERAND_H_

#include <memory>
#include <string>
#include <vector>
#include "IR/IRType.h"


class IROperand
{
public:
    IROperand(const IRType* t) : type_(t) {}
    virtual ~IROperand() {}
    virtual std::string ToString() const = 0;

    virtual bool IsIntConst() const { return false; }
    virtual bool IsFloatConst() const { return false; }
    virtual bool IsConstant() const { return false; }
    virtual bool IsRegister() const { return false; }

    auto Type() const { return type_; }
    auto& Type() { return type_; }

protected:
    const IRType* type_{};
};


class IOperandPool
{
public:
    virtual ~IOperandPool() {}
    void AddIROperand(std::unique_ptr<IROperand>);

private:
    std::vector<std::unique_ptr<IROperand>> operands_{};
};


class Constant : public IROperand
{
public:
    Constant(const IRType* t) : IROperand(t) {}
    virtual bool IsZero() const = 0;
};

class IntConst : public Constant
{
public:
    static IntConst* CreateIntConst(IOperandPool*, unsigned long);
    static IntConst* CreateIntConst(IOperandPool*, unsigned long, const IntType*);
    IntConst(unsigned long ul, const IntType* t) :
        num_(ul), Constant(t) {}

    std::string ToString() const override;
    unsigned long Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }
    bool IsConstant() const override { return true; }
    bool IsIntConst() const override { return true; }

private:
    unsigned long num_{};
};

class FloatConst : public Constant
{
public:
    static FloatConst* CreateFloatConst(IOperandPool*, double);
    static FloatConst* CreateFloatConst(IOperandPool*, double, const FloatType*);
    FloatConst(double d, const FloatType* t) :
        num_(d), Constant(t) {}

    std::string ToString() const override;
    double Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }
    bool IsConstant() const override { return true; }
    bool IsFloatConst() const override { return true; }

private:
    double num_{};
};

class Register : public IROperand
{
public:
    static Register* CreateRegister(IOperandPool*, const std::string&, const IRType*);
    Register(const std::string& n, const IRType* t) :
        name_(n), IROperand(t) {}

    std::string ToString() const override;
    std::string Name() const { return name_; }

private:
    std::string name_{};
};

#endif // _IROPERAND_H_
