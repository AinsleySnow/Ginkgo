#ifndef _IR_GEN_H_
#define _IR_GEN_H_

#include "IR/IRBuilder.h"
#include "IR/Value.h"
#include "visitast/Scope.h"
#include "visitast/Visitor.h"
#include <list>
#include <memory>
#include <stack>

class Declaration;
class IROperand;
class IRType;
class Statement;


class IRGen : public Visitor
{
public:
    IRGen() {}
    IRGen(std::string name) :
        transunit_(std::make_unique<Module>(name)) {}

    void VisitDeclSpec(DeclSpec*) override;
    void VisitDeclList(DeclList*) override;
    void VisitFuncDef(FuncDef*) override;
    void VisitObjDef(ObjDef*) override;
    void VisitParamList(ParamList*) override;
    void VisitTransUnit(DeclStmt*) override;

    void VisitAssignExpr(AssignExpr*) override;
    void VisitBinaryExpr(BinaryExpr*) override;
    void VisitCallExpr(CallExpr*) override;
    void VisitCastExpr(CastExpr*) override;
    void VisitCondExpr(CondExpr*) override;
    void VisitConstant(ConstExpr*) override;
    void VisitExprList(ExprList*) override;
    void VisitIdentExpr(IdentExpr*) override;
    void VisitLogicalExpr(LogicalExpr*) override;
    void VisitUnaryExpr(UnaryExpr*) override;

    void VisitBreakStmt(BreakStmt*) override;
    void VisitCaseStmt(CaseStmt*) override;
    void VisitCompoundStmt(CompoundStmt*) override;
    void VisitContinueStmt(ContinueStmt*) override;
    void VisitDoWhileStmt(DoWhileStmt*) override;
    void VisitExprStmt(ExprStmt*) override;
    void VisitForStmt(ForStmt*) override;
    void VisitGotoStmt(GotoStmt*) override;
    void VisitIfStmt(IfStmt*) override;
    void VisitLabelStmt(LabelStmt*) override;
    void VisitRetStmt(RetStmt*) override;
    void VisitSwitchStmt(SwitchStmt*) override;
    void VisitWhileStmt(WhileStmt*) override;

    auto GetModule() { return std::move(transunit_); }


private:
    const IROperand* EvalBinary(Tag t, const IROperand*, const IROperand*);
    const IROperand* EvalUnary(Tag op, const IROperand*);
    BasicBlock* GetBasicBlock();
    Instr* GetLastInstr();
    std::string GetRegName() { return '%' + std::to_string(index_++); }

    const Register* AllocaObject(const CType*, const std::string&);

    void FillNullBlk(BrInstr*, BasicBlock*);
    void Backpatch(const std::list<BrInstr*>&, BasicBlock*);
    std::list<BrInstr*> Merge(std::list<BrInstr*>&, std::list<BrInstr*>&);

    ScopeStack scopestack_{};

    // store pointers to statement nodes to make
    // br instructions generated by break and
    // continue statements to find their way to go
    std::stack<Statement*> brkcntn_{};
    // To which basic block does a label map?
    std::unordered_map<std::string, BasicBlock*> labelmap_{};
    // Where will this BrInstr go to?
    std::unordered_map<BrInstr*, std::string> gotomap_{};
    // store BrInstr* generated by RetInstr
    std::list<BrInstr*> ret_{};
    IRBuilder builder_{};
    size_t index_{};

    std::unique_ptr<Module> transunit_{};
    Function* curfunc_{};
};

#endif // _IR_GEN_H_
