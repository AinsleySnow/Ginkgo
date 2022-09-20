#ifndef _IR_H_
#define _IR_H_

#include <list>
#include <string>
#include <variant>
#include <optional>
#include "IROper.h"
#include "Constant.h"

struct Quadruple
{
    IROper op;
    std::variant<Constant, std::string> arg1;
    std::variant<Constant, std::string> arg2;
    std::string arg3;

    Quadruple(IROper o, const std::string& a3) : op(o), arg3(a3) {};
    Quadruple(IROper o, const std::variant<Constant, std::string>& a1, const std::string& a3) : 
        op(o), arg1(a1), arg3(a3) {};
    Quadruple(IROper o, const std::variant<Constant, std::string>& a1, 
        const std::variant<Constant, std::string>& a2, const std::string& a3) : 
            op(o), arg1(a1), arg2(a2), arg3(a3) {};

    std::string ToString();
};

class IR
{
private:
    static unsigned long lableIndex;
    std::list<Quadruple> data{};

public:
    static std::string GetLable();
    
    std::optional<
        std::variant<Constant, std::string>> Identifier;

    void Append(const Quadruple&);
    void AppendLable(const std::string&);
    void Join(IR&);
    std::string GetLastVar() const;
    Constant GetConstant() const;
};

#endif // _IR_H_
