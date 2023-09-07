#ifndef _TYPE_BUILDER_H_
#define _TYPE_BUILDER_H_

#include "visitast/ASTVisitor.h"
#include "ast/CType.h"
#include <memory>

class EnumSpec;
class HeterSpec;
class TypedefSpec;
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
    void VisitHeterList(HeterList*) override;

    void VisitAccessExpr(AccessExpr*) override;
    void VisitArrayExpr(ArrayExpr*) override;
    void VisitAssignExpr(AssignExpr*) override;
    void VisitBinaryExpr(BinaryExpr*) override;
    void VisitCallExpr(CallExpr*) override;
    void VisitCastExpr(CastExpr*) override;
    void VisitCondExpr(CondExpr*) override;
    void VisitSzAlgnExpr(SzAlgnExpr*) override;
    void VisitEnumConst(EnumConst*) override;
    void VisitEnumList(EnumList*) override;
    void VisitExprList(ExprList*) override;
    void VisitIdentExpr(IdentExpr*) override;
    void VisitLogicalExpr(LogicalExpr*) override;
    void VisitStrExpr(StrExpr*) override;
    void VisitUnaryExpr(UnaryExpr*) override;

private:
    std::unique_ptr<CEnumType> EnumHelper(const EnumSpec*, size_t);
    template <class T> std::unique_ptr<T> HeterHelper(const HeterSpec*, size_t);
    std::unique_ptr<CType> TypedefHelper(const TypedefSpec*, size_t);

    std::shared_ptr<CType> EnlargeCType(std::shared_ptr<CType>, int);
    std::shared_ptr<CType> GetCTypeByValue(std::shared_ptr<CType>, std::shared_ptr<CType>, uint64_t);
    std::shared_ptr<CType> GetCTypeByValue(std::shared_ptr<CType>, std::shared_ptr<CType>, double);
    std::shared_ptr<CType> MatchCType(std::shared_ptr<CType>, std::shared_ptr<CType>);

    ASTVisitor& visitor_;
    ScopeStack& scopestack_;
};

#endif // _TYPE_BUILDER_H_
