#ifndef _IROPERAND_H_
#define _IROPERAND_H_

#include <memory>
#include <string>
#include <vector>
#include "IR/IRType.h"
#include "utils/DynCast.h"
#include "utils/Pool.h"


class IROperand
{
protected:
    enum class OpId { op, _int, _float, str, reg };
    static bool ClassOf(const IROperand const*) { return true; }
    OpId id_ = OpId::op;

public:
    IROperand(OpId i, const IRType* t) : id_(i), type_(t) {}
    virtual ~IROperand() {}
    virtual std::string ToString() const = 0;

    OpId ID() const { return id_; }

    ENABLE_IS;
    ENABLE_AS;

    auto Type() const { return type_; }
    auto& Type() { return type_; }

protected:
    const IRType* type_{};
};


class Constant : public IROperand
{
protected:
    static bool ClassOf(const Constant const*) { return true; }
    static bool ClassOf(const IROperand const* op)
    { return op->ID() == OpId::_int || op->ID() == OpId::_float; }

public:
    Constant(OpId i, const IRType* t) : IROperand(i, t) {}
    virtual bool IsZero() const = 0;
};

class IntConst : public Constant
{
protected:
    static bool ClassOf(const IntConst const*) { return true; }
    static bool ClassOf(const Constant const* c) { return c->ID() == OpId::_int; }

public:
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long);
    static IntConst* CreateIntConst(Pool<IROperand>*, unsigned long, const IntType*);
    IntConst(unsigned long ul, const IntType* t) :
        num_(ul), Constant(OpId::_int, t) {}

    std::string ToString() const override;
    unsigned long Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }

private:
    unsigned long num_{};
};

class FloatConst : public Constant
{
protected:
    static bool ClassOf(const FloatConst const*) { return true; }
    static bool ClassOf(const Constant const* c) { return c->ID() == OpId::_float; }

public:
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double);
    static FloatConst* CreateFloatConst(Pool<IROperand>*, double, const FloatType*);
    FloatConst(double d, const FloatType* t) :
        num_(d), Constant(OpId::_float, t) {}

    std::string ToString() const override;
    double Val() const { return num_; }

    bool IsZero() const override { return num_ == 0; }

private:
    double num_{};
};

class StrConst : public IROperand
{
protected:
    static bool ClassOf(const StrConst const*) { return true; }
    static bool ClassOf(const IROperand const* op) { return op->ID() == OpId::str; }

public:
    static StrConst* CreateStrConst(Pool<IROperand>*, const std::string&, const IRType*);
    StrConst(const std::string& s, const PtrType* ty) :
        literal_(s), IROperand(OpId::str, ty) {}

    std::string ToString() const override { return literal_; }

private:
    std::string literal_{};
};


class Register : public IROperand
{
protected:
    static bool ClassOf(const Register const*) { return true; }
    static bool ClassOf(const IROperand const* op) { return op->ID() == OpId::reg; }

public:
    static Register* CreateRegister(Pool<IROperand>*, const std::string&, const IRType*);
    Register(const std::string& n, const IRType* t) :
        name_(n), IROperand(OpId::reg, t) {}

    std::string ToString() const override;
    std::string Name() const { return name_; }

private:
    std::string name_{};
};

#endif // _IROPERAND_H_
