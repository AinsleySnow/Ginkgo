#ifndef _TYPE_BUILDER_H_
#define _TYPE_BUILDER_H_

#include "visitast/ASTVisitor.h"
#include "ast/CType.h"
#include <memory>

class EnumSpec;
class ScopeStack;


class TypeBuilder : public ASTVisitor
{
public:
    TypeBuilder(ASTVisitor& v, ScopeStack& s) : visitor_(v), scopestack_(s) {}

    void VisitDeclSpec(DeclSpec*) override;
    void VisitFuncDef(FuncDef*) override;
    void VisitObjDef(ObjDef*) override;
    void VisitPtrDef(PtrDef*) override;
    void VisitArrayDef(ArrayDef*) override;


private:
    std::unique_ptr<CEnumType> EnumHelper(const EnumSpec*);

    ASTVisitor& visitor_;
    ScopeStack& scopestack_;
};

#endif // _TYPE_BUILDER_H_
