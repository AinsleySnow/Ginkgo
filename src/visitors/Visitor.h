#ifndef _VISITOR_H_
#define _VISITOR_H_

class AssignExpr;
class BinaryExpr;
class CondExpr;
class Constant;
class ExprList;
class Label;
class Object;
class Scope;
class Str;
class UnaryExpr;


class Visitor
{
public:
    virtual ~Visitor() {}
    virtual void VisitAssignExpr(AssignExpr*) {}
    virtual void VisitBinaryExpr(BinaryExpr*) {}
    virtual void VisitCondExpr(CondExpr*) {}
    virtual void VisitConstant(Constant*) {}
    virtual void VisitExprList(ExprList*) {}
    virtual void VisitLabel(Label*) {}
    virtual void VisitObject(Object*) {}
    virtual void VisitScope(Scope*) {}
    virtual void VisitStr(Str*) {}
    virtual void VisitUnaryExpr(UnaryExpr*) {}
};

#endif // _VISITOR_H_
