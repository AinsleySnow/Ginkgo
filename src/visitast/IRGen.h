#ifndef _IR_GEN_H_
#define _IR_GEN_H_

#include "IR/IRBuilder.h"
#include "IR/Value.h"
#include "visitast/Scope.h"
#include "visitast/ASTVisitor.h"
#include "visitast/TypeBuilder.h"
#include <list>
#include <memory>
#include <stack>
#include <variant>

class Declaration;
class EnumSpec;
class HeterSpec;
class IROperand;
class IRType;
class FuncType;
class Statement;


class IRGen : public ASTVisitor
{
public:
    IRGen() {}
    IRGen(std::string name) :
        transunit_(std::make_unique<Module>(name)) {}

    void VisitArrayDef(ArrayDef*) override;
    void VisitDeclSpec(DeclSpec*) override;
    void VisitDeclList(DeclList*) override;
    void VisitFuncDef(FuncDef*) override;
    void VisitObjDef(ObjDef*) override;
    void VisitParamList(ParamList*) override;
    void VisitPtrDef(PtrDef*) override;

    void VisitAccessExpr(AccessExpr*) override;
    void VisitArrayExpr(ArrayExpr*) override;
    void VisitAssignExpr(AssignExpr*) override;
    void VisitBinaryExpr(BinaryExpr*) override;
    void VisitCallExpr(CallExpr*) override;
    void VisitCastExpr(CastExpr*) override;
    void VisitCondExpr(CondExpr*) override;
    void VisitConstant(ConstExpr*) override;
    void VisitSzAlgnExpr(SzAlgnExpr*) override;
    void VisitEnumConst(EnumConst*) override;
    void VisitEnumList(EnumList*) override;
    void VisitExprList(ExprList*) override;
    void VisitIdentExpr(IdentExpr*) override;
    void VisitLogicalExpr(LogicalExpr*) override;
    void VisitStrExpr(StrExpr*) override;
    void VisitUnaryExpr(UnaryExpr*) override;

    void VisitBreakStmt(BreakStmt*) override;
    void VisitCaseStmt(CaseStmt*) override;
    void VisitCompoundStmt(CompoundStmt*) override;
    void VisitContinueStmt(ContinueStmt*) override;
    void VisitDeclStmt(DeclStmt*) override;
    void VisitDoWhileStmt(DoWhileStmt*) override;
    void VisitExprStmt(ExprStmt*) override;
    void VisitForStmt(ForStmt*) override;
    void VisitGotoStmt(GotoStmt*) override;
    void VisitIfStmt(IfStmt*) override;
    void VisitLabelStmt(LabelStmt*) override;
    void VisitRetStmt(RetStmt*) override;
    void VisitSwitchStmt(SwitchStmt*) override;
    void VisitTransUnit(TransUnit*) override;
    void VisitWhileStmt(WhileStmt*) override;

    auto GetModule() { return std::move(transunit_); }


private:
    class CurrentEnv
    {
    public:
        CurrentEnv(bool h) : holdvar_(h)
        {
            if (!holdvar_)
                return;
            opool_ = std::make_unique<Pool<IROperand>>();
            typool_ = std::make_unique<Pool<IRType>>();
        }
        CurrentEnv(Function* v) : env_(v), holdfunc_(true) {}

        bool InFunction() const { return holdfunc_; }
        bool InGlobalVar() const { return holdvar_; }

        Function* GetFunction() { return std::get<0>(env_); }
        void EnterGlobalVar(GlobalVar* v) { env_ = v; }
        GlobalVar* GetGlobalVar() { return std::get<1>(env_); }

        std::string GetRegName() { return '%' + std::to_string(index_++); }
        std::string GetLabelName() { return std::to_string(index_++); }

        auto GetParamList() const { return paramlist_; }
        void SetParamList(const ParamList* pl) { paramlist_ = pl; }

        void PushStmt(Statement* s) { brkcntn_.push(s); }
        void PopStmt() { brkcntn_.pop(); }
        Statement* StmtStackTop() { return brkcntn_.top(); }

        void PushSwitch(SwitchInstr* i) { swtch_.push(i); }
        void PopSwitch() { swtch_.pop(); }
        SwitchInstr* SwitchStackTop() { return swtch_.top(); }

        void AddBrInstr4Ret(BrInstr* br) { ret_.push_back(br); }
        void AddLabelBlkPair(const std::string& s, BasicBlock* bb) { labelmap_.emplace(s, bb); }
        void AddBrLabelPair(BrInstr* br, const std::string& s) { gotomap_.emplace(br, s); }

        void Epilogue(BasicBlock*);

        // an elegant seperator -- methods for building global variable below

        Pool<IRType>* GetTypePool() { return typool_.get(); }
        Pool<IROperand>* GetOpPool() { return opool_.get(); }

        void Dump2Tree(const IRType*);
        std::unique_ptr<Node>&& GetExprTree() { return std::move(tree_); }
        void MergeNode(Instr::InstrId);
        void AddOpNode(const IROperand*, int);
        void AddOpNode(const IROperand*);

    private:
        bool holdfunc_{};
        bool holdvar_{};
        std::variant<Function*, GlobalVar*> env_{};

        // store pointers to statement nodes to make
        // br instructions generated by break and
        // continue statements to find their way to go
        std::stack<Statement*> brkcntn_{};
        // Map case and break in switch.
        std::stack<SwitchInstr*> swtch_{};
        // To which basic block does a label map?
        std::unordered_map<std::string, BasicBlock*> labelmap_{};
        // Where will this BrInstr go to?
        std::unordered_map<BrInstr*, std::string> gotomap_{};
        // store BrInstr* generated by RetInstr
        std::list<BrInstr*> ret_{};
        // pointer to current function's original parameter list
        const ParamList* paramlist_{};

        size_t index_{};

        std::unique_ptr<Node> tree_{};
        std::unique_ptr<Pool<IROperand>> opool_{};
        std::unique_ptr<Pool<IRType>> typool_{};
        static std::stack<std::unique_ptr<Node>> stack_;
    };

    std::string GetStrName() { return "@.str" + std::to_string(strindex_++); }

    const Register* AllocaObject(const CType*, const std::string&, bool = false);
    Function* AllocaFunc(const CFuncType*, const std::string&);

    void TypeInferenceHelper(Declaration*, Expr*);

    const FuncType* HandleBuiltins(CallExpr*);
    void HandleVaStart(CallExpr*);
    void HandleVaArg(CallExpr*);

    const IROperand* LoadVal(Expr*);
    const Register* LoadAddr(Expr*);

    static void FillNullBlk(BrInstr*, BasicBlock*);
    static void Backpatch(std::list<BrInstr*>&, BasicBlock*);
    static void Merge(const std::list<BrInstr*>&, std::list<BrInstr*>&);

    ScopeStack scopestack_{};
    TypeBuilder tbud_{ *this, scopestack_ };
    InstrBuilder ibud_{};
    BlockBuilder bbud_{};
    CurrentEnv env_{ false };
    size_t strindex_{};

    std::unique_ptr<Module> transunit_{};
};

#endif // _IR_GEN_H_
