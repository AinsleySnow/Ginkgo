#include "visitast/IRGen.h"
#include "visitast/Evaluator.h"
#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include <algorithm>


BasicBlock* IRGen::CurrentEnv::GetBasicBlock(const std::string& name)
{
    if (env_.index() == 0)
        return BasicBlock::CreateBasicBlock(std::get<0>(env_), name);
    else return BasicBlock::CreateBasicBlock(std::get<1>(env_), name);
}

BasicBlock* IRGen::CurrentEnv::GetBasicBlock()
{
    std::string name = '%' + std::to_string(index_++);
    return GetBasicBlock(name);
}

Instr* IRGen::CurrentEnv::GetLastInstr()
{
    if (env_.index() == 0)
    {
        auto func = std::get<0>(env_);
        for (int i = -1; ; --i)
        {
            auto bb = func->GetBasicBlock(i);
            if (!bb->Empty()) return bb->GetLastInstr();
        }
    }
    else return std::get<1>(env_)->GetBasicBlock()->GetLastInstr();
}

void IRGen::CurrentEnv::Epilog(BasicBlock* bb)
{
    Backpatch(ret_, bb);
    for (auto gotopair : gotomap_)
        FillNullBlk(gotopair.first, labelmap_[gotopair.second]);
}


const Register* IRGen::AllocaObject(const CType* raw, const std::string& name)
{
    if (scopestack_.Top().GetScopeType() == Scope::ScopeType::file)
    {
        auto ty = raw->ToIRType(transunit_.get());
        auto regname = '@' + name;
        auto var = GlobalVar::CreateGlobalVar(transunit_.get(), regname, ty);

        env_ = CurrentEnv(var);
        builder_.InsertPoint() = env_.GetBasicBlock("");
        auto reg = Register::CreateRegister(
            builder_.InsertPoint(), regname,
            PtrType::GetPtrType(builder_.InsertPoint(), ty));
        scopestack_.Top().AddObject(name, raw, reg);
        return reg;
    }
    else
    {
        auto reg = builder_.InsertAllocaInstr(
            env_.GetRegName(), raw->ToIRType(builder_.InsertPoint()));
        scopestack_.Top().AddObject(name, raw, reg);
        return reg;
    }
}

const IROperand* IRGen::LoadVal(Expr* expr)
{
    if (expr->IsIdentifier())
    {
        auto ident = expr->ToIdentifier();
        ident->Val() = builder_.InsertLoadInstr(env_.GetRegName(), ident->Addr());
        return ident->Val();
    }
    else return expr->Val();
}

const Register* IRGen::LoadAddr(Expr* expr)
{
    if (expr->IsIdentifier())
        return expr->ToIdentifier()->Addr();
    return static_cast<const Register*>(expr->Val());
}

void IRGen::InsertBrIfNoBranchAhead(BasicBlock* bb)
{
    if (builder_.InsertPoint()->Empty() ||
        builder_.InsertPoint()->GetLastInstr()->IsControlInstr())
        builder_.InsertBrInstr(bb);
}

void IRGen::InsertBrIfNoBranchAhead(const IROperand* cond, BasicBlock* tbb, BasicBlock* fbb)
{
    if (builder_.InsertPoint()->Empty() ||
        builder_.InsertPoint()->GetLastInstr()->IsControlInstr())
        builder_.InsertBrInstr(cond, tbb, fbb);
}


void IRGen::FillNullBlk(BrInstr* br, BasicBlock* blk)
{
    if (!br->GetTrueBlk()) br->SetTrueBlk(blk);
    if (!br->GetFalseBlk()) br->SetFalseBlk(blk);
}

void IRGen::Backpatch(
    std::list<BrInstr*>& list, BasicBlock* dest)
{
    for (auto instr : list)
        FillNullBlk(instr, dest);
    list.clear();
}

void IRGen::Merge(
    const std::list<BrInstr*>& src, std::list<BrInstr*>& dest)
{
    dest.insert(dest.end(), src.begin(), src.end());
}


void IRGen::VisitDeclSpec(DeclSpec* spec)
{
    auto tag = spec->TypeSpec();
    if (tag == TypeTag::_void)
        spec->type_ = std::make_unique<CVoidType>();
    else if (tag != TypeTag::customed)
    {
        spec->type_ = std::make_unique<CArithmType>(tag);
        spec->type_->Qual() = spec->Qual();
        spec->type_->Storage() = spec->Storage();
    }
    else
    {
        // TODO
    }
}


void IRGen::VisitDeclList(DeclList* list)
{
    for (auto& initdecl : *list)
    {
        initdecl->declarator_->Accept(this);
            
        if (!dynamic_cast<FuncDef*>(initdecl->declarator_.get()))
        {
            initdecl->base_ = AllocaObject(
            initdecl->declarator_->RawType(),
            initdecl->declarator_->Name());

            if (initdecl->initalizer_)
            {
                initdecl->initalizer_->Accept(this);
                builder_.InsertStoreInstr(
                    initdecl->initalizer_->Val(),
                    initdecl->base_, false);
            }   
        }
    }
}


void IRGen::VisitFuncDef(FuncDef* def)
{
    def->declspec_->Accept(this);
    if (def->paramlist_)
        def->paramlist_->Accept(this);

    auto spec = def->GetDeclSpec();
    const Ptr* ptr = def->GetRawPtr();

    auto retcty = std::move(spec->Type());
    if (ptr) retcty = retcty->AttachPtr(ptr);
    def->return_ = std::move(retcty);

    auto funccty = std::make_unique<CFuncType>(
        std::move(def->return_), def->GetParamList().size());
    for (auto param : def->GetParamType())
        funccty->AddParam(std::move(param));

    Function* pfunc = nullptr;
    if (scopestack_.SearchFunc(def->Name()))
        pfunc = transunit_->GetFunction('@' + def->Name());
    else
    {
        scopestack_.File().AddFunc(def->Name(), funccty.get());
        pfunc = transunit_->AddFunc(
            '@' + def->Name(), funccty->ToIRType(transunit_.get()));
        pfunc->Inline() = spec->Func().IsInline();
        pfunc->Noreturn() = spec->Func().IsNoreturn();
    }

    if (!def->compound_)
        return;

    env_ = CurrentEnv(pfunc);
    builder_.InsertPoint() = env_.GetBasicBlock();
    scopestack_.PushNewScope(Scope::ScopeType::block);

    auto rety = env_.GetFunction()->ReturnType();
    if (!rety->ToVoid())
    {
        env_.GetFunction()->ReturnValue() =
            builder_.InsertAllocaInstr(env_.GetRegName(), rety);
    }

    for (auto& param : def->GetParamList())
    {
        if (dynamic_cast<const CVoidType*>(param->RawType()))
            continue;
        auto ctype = param->RawType();
        auto paramreg = Register::CreateRegister(
            builder_.InsertPoint(), env_.GetRegName(),
            ctype->ToIRType(builder_.InsertPoint()));

        env_.GetFunction()->AddParam(paramreg);
        if (!param->Name().empty())
        {
            auto addr = AllocaObject(ctype, param->Name());
            builder_.InsertStoreInstr(paramreg, addr, false);
        }
    }

    def->compound_->Accept(this);
    if (!builder_.InsertPoint()->Empty())
        builder_.InsertPoint() = env_.GetBasicBlock();

    if (!rety->ToVoid())
    {
        auto retvalue = builder_.InsertLoadInstr(
            env_.GetRegName(), env_.GetFunction()->ReturnValue());
        builder_.InsertRetInstr(retvalue);
    }
    else builder_.InsertRetInstr();

    env_.Epilog(builder_.InsertPoint());
    scopestack_.PopScope();
}


void IRGen::VisitObjDef(ObjDef* def)
{
    def->declspec_->Accept(this);

    auto spec = def->GetDeclSpec();
    const Ptr* ptr = def->GetRawPtr();

    auto objcty = std::move(spec->type_);
    if (ptr) objcty = objcty->AttachPtr(ptr);

    def->type_ = std::move(objcty);
}


void IRGen::VisitParamList(ParamList* list)
{
    for (auto& param : list->GetParamList())
    {
        param->Accept(this);
        list->AppendType(param->RawType());
    }
}


void IRGen::VisitAssignExpr(AssignExpr* assign)
{
    assign->left_->Accept(this);
    assign->right_->Accept(this);

    auto rhs = LoadVal(assign->right_.get());
    const Register* addr = LoadAddr(assign->left_.get());

    if (assign->op_ == Tag::assign)
    {
        builder_.InsertStoreInstr(rhs, addr, false);
        assign->Val() = rhs;
        return;
    }

    auto lhs = LoadVal(assign->left_.get());
    auto lhsty = lhs->Type();
    auto rhsty = rhs->Type();

    auto regname = env_.GetRegName();
    const Register* result = nullptr;

    if (assign->op_ == Tag::add_assign && lhsty->IsInt())
        result = builder_.InsertAddInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::add_assign && lhsty->IsFloat())
        result = builder_.InsertFaddInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::sub_assign && lhsty->IsInt())
        result = builder_.InsertSubInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::sub_assign && lhsty->IsFloat())
        result = builder_.InsertFsubInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::mul_assign && lhsty->IsInt())
        result = builder_.InsertMulInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::mul_assign && lhsty->IsFloat())
        result = builder_.InsertFmulInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::div_assign && lhsty->IsInt())
        result = builder_.InsertDivInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::div_assign && lhsty->IsFloat())
        result = builder_.InsertFdivInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::mod_assign && lhsty->IsInt())
        result = builder_.InsertModInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::left_assign)
        result = builder_.InsertShlInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::right_assign && lhsty->ToInteger()->IsSigned())
        result = builder_.InsertAshrInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::right_assign)
        result = builder_.InsertLshrInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::and_assign)
        result = builder_.InsertAndInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::or_assign)
        result = builder_.InsertOrInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::xor_assign)
        result = builder_.InsertXorInstr(regname, lhs, rhs);


    builder_.InsertStoreInstr(result, addr, false);
    assign->Val() = result;
}


void IRGen::VisitBinaryExpr(BinaryExpr* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);

    if (bin->left_->IsConstant() && bin->right_->IsConstant())
    {
        bin->Val() = Evaluator::EvalBinary(
            builder_.InsertPoint(), bin->op_, bin->left_->Val(), bin->right_->Val());
        return;
    }

    auto lhs = LoadVal(bin->left_.get()), rhs = LoadVal(bin->right_.get());
    auto lhsty = lhs->Type(), rhsty = rhs->Type();
    bool hasfloat = lhsty->IsFloat() || rhsty->IsFloat();

    std::string regname = env_.GetRegName();
    const Register* result = nullptr;

    if (bin->op_ == Tag::plus && !hasfloat)
        result = builder_.InsertAddInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::plus)
        result = builder_.InsertFaddInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::minus && !hasfloat)
        result = builder_.InsertSubInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::minus)
        result = builder_.InsertFsubInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::asterisk && !hasfloat)
        result = builder_.InsertMulInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::asterisk)
        result = builder_.InsertFmulInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::slash && !hasfloat)
        result = builder_.InsertDivInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::slash)
        result = builder_.InsertFdivInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::percent)
        result = builder_.InsertModInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::lshift)
        result = builder_.InsertShlInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::rshift && lhsty->ToInteger()->IsSigned())
        result = builder_.InsertAshrInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::rshift)
        result = builder_.InsertLshrInstr(regname, lhs, rhs);   

    else if (bin->op_ == Tag::lessthan)
        result = builder_.InsertCmpInstr(regname, Condition::lt, lhs, rhs);
    else if (bin->op_ == Tag::lessequal)
        result = builder_.InsertCmpInstr(regname, Condition::le, lhs, rhs);
    else if (bin->op_ == Tag::greathan)
        result = builder_.InsertCmpInstr(regname, Condition::gt, lhs, rhs);
    else if (bin->op_ == Tag::greatequal)
        result = builder_.InsertCmpInstr(regname, Condition::ge, lhs, rhs);
    else if (bin->op_ == Tag::equal)
        result = builder_.InsertCmpInstr(regname, Condition::eq, lhs, rhs);
    else if (bin->op_ == Tag::notequal)
        result = builder_.InsertCmpInstr(regname, Condition::ne, lhs, rhs);

    else if (bin->op_ == Tag::_and)
        result = builder_.InsertAndInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::_xor)
        result = builder_.InsertXorInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::_or)
        result = builder_.InsertOrInstr(regname, lhs, rhs);

    bin->Val() = result;
}


void IRGen::VisitCallExpr(CallExpr* call)
{
    if (call->argvlist_)
    {
        call->argvlist_->Accept(this);
        for (auto& argv : *call->argvlist_)
            LoadVal(argv.get());
    }

    if (call->postfix_->IsIdentifier())
    {
        auto pfunc = transunit_->GetFunction(
            '@' + call->postfix_->ToIdentifier()->name_);
        const FuncType* functy = pfunc->Type();
        call->Val() = builder_.InsertCallInstr(
            env_.GetRegName(), functy, call->postfix_->ToIdentifier()->name_);
    }
    else // if a function is called through a pointer
    {
        call->postfix_->Accept(this);
        auto pfunc = static_cast<const Register*>(call->postfix_->Val());
        auto func = builder_.InsertLoadInstr(env_.GetRegName(), pfunc);
        call->Val() = builder_.InsertCallInstr(env_.GetRegName(), func);
    }

    if (call->argvlist_)
    {
        auto callinstr = static_cast<CallInstr*>(builder_.GetLastInstr());
        for (auto& argv : *call->argvlist_)
            callinstr->AddArgv(argv->Val());
    }
}


void IRGen::VisitCastExpr(CastExpr* cast)
{
    cast->typename_->Accept(this);
    cast->expr_->Accept(this);

    auto& ty = cast->typename_->Type();
    auto expreg = LoadVal(cast->expr_.get());

    if (ty->IsInteger())
    {
        // TODO
    }
    else if (ty->IsFloat())
    {
        // TODO
    }

    cast->Val() = expreg;
    auto arithm = static_cast<const CArithmType*>(ty.get());
    cast->Type() = std::make_unique<CArithmType>(*arithm);
}


void IRGen::VisitCondExpr(CondExpr* cond)
{
    cond->cond_->Accept(this);
    if (cond->cond_->IsConstant())
    {
        auto sel = static_cast<const Constant*>(cond->cond_->Val());
        if (sel->IsZero())
        {
            cond->false_->Accept(this);
            cond->Val() = LoadVal(cond->false_.get());
        }
        else
        {
            cond->true_->Accept(this);
            cond->Val() = LoadVal(cond->true_.get());
        }
        return;
    }

    auto initblk = builder_.InsertPoint();

    auto trueblk = std::make_unique<BasicBlock>(env_.GetRegName());
    builder_.InsertPoint() = trueblk.get();
    cond->true_->Accept(this);

    auto falseblk = std::make_unique<BasicBlock>(env_.GetRegName());
    builder_.InsertPoint() = falseblk.get();
    cond->false_->Accept(this);

    if (cond->true_->IsConstant() && cond->false_->IsConstant())
    {
        builder_.InsertPoint() = initblk;
        cond->Val() = builder_.InsertSelectInstr(
            env_.GetRegName(), LoadVal(cond->cond_.get()),
            true, cond->true_->Val(), cond->false_->Val());

        initblk->MergePools(trueblk.get());
        initblk->MergePools(falseblk.get());
        return;
    }

    auto endblk = std::make_unique<BasicBlock>(env_.GetRegName());

    builder_.InsertPoint() = initblk;
    builder_.InsertBrInstr(
        LoadVal(cond->cond_.get()), trueblk.get(), falseblk.get());

    builder_.InsertPoint() = trueblk.get();
    builder_.InsertBrInstr(endblk.get());

    builder_.InsertPoint() = falseblk.get();
    builder_.InsertBrInstr(endblk.get());

    builder_.InsertPoint() = endblk.get();
    auto tval = LoadVal(cond->true_.get());
    auto fval = LoadVal(cond->false_.get());

    cond->Val() = builder_.InsertPhiInstr(
        env_.GetRegName(), tval->Type());
    auto phi = static_cast<PhiInstr*>(endblk->GetLastInstr());
    phi->AddBlockValPair(trueblk.get(), tval);
    phi->AddBlockValPair(falseblk.get(), fval);

    env_.GetFunction()->AddBasicBlock(std::move(trueblk));
    env_.GetFunction()->AddBasicBlock(std::move(falseblk));
    env_.GetFunction()->AddBasicBlock(std::move(endblk));
}


void IRGen::VisitConstant(ConstExpr* constant)
{
    auto ctype = constant->RawType();
    if (ctype->IsInteger())
        constant->Val() = IntConst::CreateIntConst(
            builder_.InsertPoint(), constant->GetInt(),
            ctype->ToIRType(transunit_.get())->ToInteger());
    else
        constant->Val() = FloatConst::CreateFloatConst(
            builder_.InsertPoint(), constant->GetFloat(),
            ctype->ToIRType(transunit_.get())->ToFloatPoint());
}


void IRGen::VisitExprList(ExprList* list)
{
    for (auto& expr : list->exprlist_)
        expr->Accept(this);
}


void IRGen::VisitIdentExpr(IdentExpr* ident)
{
    auto object = scopestack_.SearchObject(ident->name_);
    ident->Addr() = object->GetAddr();
}


void IRGen::VisitLogicalExpr(LogicalExpr* logical)
{
    logical->left_->Accept(this);
    auto lhs = LoadVal(logical->left_.get());
    const IROperand* rhs = nullptr;

    if (lhs->IsConstant())
    {
        auto lconst = static_cast<const Constant*>(lhs);
        if ((lconst->IsZero() && logical->op_ == Tag::logical_and) ||
            (!lconst->IsZero() && logical->op_ == Tag::logical_or))
        {
            logical->Val() = IntConst::CreateIntConst(
                builder_.InsertPoint(), !lconst->IsZero());
            return;
        }

        logical->right_->Accept(this);
        rhs = LoadVal(logical->right_.get());
        if (rhs->IsConstant())
        {
            auto rconst = static_cast<const Constant*>(rhs);
            logical->Val() = Evaluator::EvalBinary(
                builder_.InsertPoint(), logical->op_, lconst, rconst);
            return;
        }

        auto zero = IntConst::CreateIntConst(builder_.InsertPoint(), 0);
        auto one = IntConst::CreateIntConst(builder_.InsertPoint(), 1);

        logical->Val() = builder_.InsertSelectInstr(
            env_.GetRegName(), rhs, true, one, zero);
        return;
    }

    auto firstblk = builder_.InsertPoint(),
        midblk = env_.GetBasicBlock(),
        finalblk = env_.GetBasicBlock();

    auto zero = IntConst::CreateIntConst(builder_.InsertPoint(), 0);
    auto cmpans = builder_.InsertCmpInstr(
        env_.GetRegName(), Condition::ne, lhs, zero);

    if (logical->op_ == Tag::logical_and)
        builder_.InsertBrInstr(cmpans, midblk, finalblk);
    else if (logical->op_ == Tag::logical_or)
        builder_.InsertBrInstr(cmpans, finalblk, midblk);

    builder_.InsertPoint() = midblk;

    logical->right_->Accept(this);
    rhs = LoadVal(logical->right_.get());

    cmpans = builder_.InsertCmpInstr(
        env_.GetRegName(), Condition::ne, rhs, zero);
    builder_.InsertBrInstr(finalblk);

    builder_.InsertPoint() = finalblk;

    auto result = env_.GetRegName();
    if (logical->op_ == Tag::logical_and)
    {
        logical->Val() = builder_.InsertPhiInstr(
            result, IntType::GetInt8(true));
        auto phi = static_cast<PhiInstr*>(env_.GetLastInstr());
        phi->AddBlockValPair(firstblk, zero);
        phi->AddBlockValPair(midblk, cmpans);
    }
    else if (logical->op_ == Tag::logical_or)
    {
        logical->Val() = builder_.InsertPhiInstr(
            result, IntType::GetInt8(true));
        auto one = IntConst::CreateIntConst(builder_.InsertPoint(), 1);
        auto phi = static_cast<PhiInstr*>(env_.GetLastInstr());
        phi->AddBlockValPair(firstblk, one);
        phi->AddBlockValPair(midblk, cmpans);
    }
}


void IRGen::VisitUnaryExpr(UnaryExpr* unary)
{
    unary->content_->Accept(this);

    if (unary->content_->IsConstant())
    {
        unary->Val() = Evaluator::EvalUnary(
            builder_.InsertPoint(), unary->op_, unary->content_->Val());
        return;
    }

    if (unary->op_ == Tag::inc || unary->op_ == Tag::dec ||
        unary->op_ == Tag::postfix_inc || unary->op_ == Tag::postfix_dec)
    {
        auto addr = LoadAddr(unary->content_.get());
        auto val = LoadVal(unary->content_.get());
        auto one = IntConst::CreateIntConst(builder_.InsertPoint(), 1);
        const Register* newval = nullptr;

        if (unary->op_ == Tag::inc || unary->op_ == Tag::postfix_inc)
            newval = builder_.InsertAddInstr(env_.GetRegName(), val, one);
        else newval = builder_.InsertSubInstr(env_.GetRegName(), val, one);

        builder_.InsertStoreInstr(newval, addr, false);
        if (unary->op_ == Tag::postfix_inc || unary->op_ == Tag::postfix_dec)
            unary->Val() = val;
        else unary->Val() = newval;
    }
    else if (unary->op_ == Tag::_and || unary->op_ == Tag::plus)
        unary->Val() = LoadVal(unary->content_.get());
    else if (unary->op_ == Tag::asterisk)
    {
        auto addreg = static_cast<const Register*>(LoadVal(unary->content_.get()));
        unary->Val() = builder_.InsertLoadInstr(env_.GetRegName(), addreg);
    }
    else if (unary->op_ == Tag::minus)
    {
        auto zero = IntConst::CreateIntConst(builder_.InsertPoint(), 0);
        auto rhs = LoadVal(unary->content_.get());
        unary->Val() = builder_.InsertSubInstr(env_.GetRegName(), zero, rhs);
    }
    else if (unary->op_ == Tag::tilde)
    {
        auto rhs = LoadVal(unary->content_.get());
        auto minusone = IntConst::CreateIntConst(
            builder_.InsertPoint(), ~0ull, rhs->Type()->ToInteger());
        unary->Val() = builder_.InsertXorInstr(env_.GetRegName(), minusone, rhs);
    }
    else if (unary->op_ == Tag::exclamation)
    {
        auto zero = IntConst::CreateIntConst(builder_.InsertPoint(), 0);
        auto one = IntConst::CreateIntConst(builder_.InsertPoint(), 1);
        unary->Val() = builder_.InsertSelectInstr(
            env_.GetRegName(), LoadVal(unary->content_.get()), true, zero, one);
    }
}


void IRGen::VisitBreakStmt(BreakStmt* stmt)
{
    builder_.InsertBrInstr(nullptr);
    env_.StmtStackTop()->NextList().push_back(
        static_cast<BrInstr*>(env_.GetLastInstr()));
}


void IRGen::VisitCaseStmt(CaseStmt* stmt)
{
    if (stmt->const_)
    {
        stmt->const_->Accept(this);
        if (!builder_.InsertPoint()->Empty())
        {
            auto bb = env_.GetBasicBlock();
            if (!env_.GetLastInstr()->IsControlInstr())
                builder_.InsertBrInstr(bb);
            builder_.InsertPoint() = bb;
        }
        env_.SwitchStackTop()->AddValueBlkPair(
            static_cast<const IntConst*>(stmt->const_->Val()),
            builder_.InsertPoint());
    }
    else // default
    {
        auto bb = env_.GetBasicBlock();
        if (!env_.GetLastInstr()->IsControlInstr())
            builder_.InsertBrInstr(bb);
        builder_.InsertPoint() = bb;
        env_.SwitchStackTop()->SetDefault(bb);
    }

    stmt->stmt_->Accept(this);
    
    // if this is default tag, create a new basic block
    if (!stmt->const_ && !builder_.InsertPoint()->Empty())
    {
        auto bb = env_.GetBasicBlock();
        if (!env_.GetLastInstr()->IsControlInstr())
            builder_.InsertBrInstr(bb);
        builder_.InsertPoint() = bb;
    }
    Merge(stmt->stmt_->NextList(), stmt->nextlist_);
}


void IRGen::VisitCompoundStmt(CompoundStmt* compound)
{
    scopestack_.PushNewScope(Scope::ScopeType::block);
    for (auto& stmt : compound->stmtlist_)
    {
        stmt->Accept(this);
        if (!stmt->NextList().empty())
        {
            if (!builder_.InsertPoint()->Empty())
            {
                auto bb = env_.GetBasicBlock();
                if (!env_.GetLastInstr()->IsControlInstr())
                    builder_.InsertBrInstr(bb);
                builder_.InsertPoint() = bb;
            }
            Backpatch(stmt->NextList(), builder_.InsertPoint());
        }
    }
    scopestack_.PopScope();
}


void IRGen::VisitContinueStmt(ContinueStmt* stmt)
{
    IterStmt* iter = static_cast<IterStmt*>(env_.StmtStackTop());
    builder_.InsertBrInstr(iter->continuepoint_);
}


void IRGen::VisitDeclStmt(DeclStmt* stmt)
{
    stmt->decl_->Accept(this);
}


void IRGen::VisitDoWhileStmt(DoWhileStmt* stmt)
{
    env_.PushStmt(stmt);

    auto loopblk = env_.GetBasicBlock();
    builder_.InsertBrInstr(loopblk);
    stmt->continuepoint_ = loopblk;

    builder_.InsertPoint() = loopblk;
    stmt->stmt_->Accept(this);

    stmt->expr_->Accept(this);
    auto cmpans = builder_.InsertCmpInstr(
        env_.GetRegName(),
        Condition::ne, LoadVal(stmt->expr_.get()),
        IntConst::CreateIntConst(builder_.InsertPoint(), 0));

    builder_.InsertBrInstr(cmpans, loopblk, nullptr);

    stmt->nextlist_.push_back(
        static_cast<BrInstr*>(env_.GetLastInstr()));
    Merge(stmt->stmt_->NextList(), stmt->nextlist_);

    env_.PopStmt();
}


void IRGen::VisitExprStmt(ExprStmt* stmt)
{
    if (!stmt->Empty())
        stmt->expr_->Accept(this);
}


void IRGen::VisitForStmt(ForStmt* stmt)
{
    env_.PushStmt(stmt);

    if (stmt->init_)
        stmt->init_->Accept(this);

    BasicBlock* cmpblk = nullptr;
    BasicBlock* loopblk = nullptr;
    BasicBlock* incblk = nullptr;

    if (!stmt->condition_->Empty())
    {
        cmpblk = env_.GetBasicBlock();
        loopblk = env_.GetBasicBlock();

        builder_.InsertBrInstr(cmpblk);
        builder_.InsertPoint() = cmpblk;
        stmt->condition_->Accept(this);
        builder_.InsertBrInstr(
            LoadVal(stmt->condition_->expr_.get()), loopblk, nullptr);
        stmt->PushBrInstr(builder_.GetLastInstr());
    }
    else
    {
        loopblk = env_.GetBasicBlock();
        builder_.InsertBrInstr(loopblk);
    }

    if (stmt->increment_)
    {
        incblk = env_.GetBasicBlock();

        builder_.InsertPoint() = incblk;
        stmt->increment_->Accept(this);

        if (!stmt->condition_->Empty())
            builder_.InsertBrInstr(cmpblk);
        else builder_.InsertBrInstr(loopblk);
    }

    builder_.InsertPoint() = loopblk;
    if (stmt->increment_)
        stmt->continuepoint_ = incblk;
    else if (!stmt->condition_->Empty())
        stmt->continuepoint_ = cmpblk;
    else stmt->continuepoint_ = loopblk;

    stmt->body_->Accept(this);

    if (stmt->increment_)
        builder_.InsertBrInstr(incblk);
    else if (!stmt->condition_->Empty())
        builder_.InsertBrInstr(cmpblk);
    else builder_.InsertBrInstr(loopblk);

    Merge(stmt->body_->NextList(), stmt->nextlist_);
    env_.PopStmt();
}


void IRGen::VisitGotoStmt(GotoStmt* stmt)
{
    builder_.InsertBrInstr(nullptr);
    auto br = static_cast<BrInstr*>(
        builder_.InsertPoint()->GetLastInstr());
    env_.AddBrLabelPair(br, stmt->ident_);
}


void IRGen::VisitIfStmt(IfStmt* stmt)
{
    BasicBlock* initblk = builder_.InsertPoint();
    BasicBlock* trueblk = env_.GetBasicBlock();
    BasicBlock* falseblk = stmt->false_ ? env_.GetBasicBlock() : nullptr;

    stmt->expr_->Accept(this);
    if (stmt->expr_->IsConstant() && 
        (initblk->Empty() || !initblk->GetLastInstr()->IsControlInstr()))
    {
        auto pconst = static_cast<const Constant*>(stmt->expr_->Val());
        if (pconst->IsZero() && falseblk)
            builder_.InsertBrInstr(falseblk);
        else if (pconst->IsZero() && !falseblk)
        {
            builder_.InsertBrInstr(nullptr);
            stmt->nextlist_.push_back(
                static_cast<BrInstr*>(initblk->GetLastInstr()));
        }
        else if (!pconst->IsZero())
            builder_.InsertBrInstr(trueblk);
    }
    else if (!stmt->expr_->IsConstant())
    {
        auto cmpans = LoadVal(stmt->expr_.get());
        if (stmt->false_)
            builder_.InsertBrInstr(cmpans, trueblk, falseblk);
        else
        {
            builder_.InsertBrInstr(cmpans, trueblk, nullptr);
            stmt->nextlist_.push_back(
                static_cast<BrInstr*>(initblk->GetLastInstr()));
        }
    }

    builder_.InsertPoint() = trueblk;
    stmt->true_->Accept(this);

    if (trueblk->Empty() || !trueblk->GetLastInstr()->IsControlInstr())
    {
        builder_.InsertBrInstr(nullptr);
        stmt->nextlist_.push_back(
            static_cast<BrInstr*>(trueblk->GetLastInstr()));
    }

    if (stmt->false_)
    {
        builder_.InsertPoint() = falseblk;
        stmt->false_->Accept(this);
        if (falseblk->Empty() || !falseblk->GetLastInstr()->IsControlInstr())
        {
            builder_.InsertBrInstr(nullptr);
            stmt->nextlist_.push_back(
                static_cast<BrInstr*>(falseblk->GetLastInstr()));
        }
    }

    if (!stmt->false_) // if (...) {...}
        Merge(stmt->true_->NextList(), stmt->nextlist_);
    else // if (...) {...} else ...
    {
        Merge(stmt->true_->NextList(), stmt->nextlist_);
        Merge(stmt->false_->NextList(), stmt->nextlist_);
    }
}


void IRGen::VisitLabelStmt(LabelStmt* stmt)
{
    if (!builder_.InsertPoint()->Empty())
    {
        auto bb = env_.GetBasicBlock();
        builder_.InsertPoint() = bb;
        if (!env_.GetLastInstr()->IsControlInstr())
            builder_.InsertBrInstr(bb);
    }

    env_.AddLabelBlkPair(stmt->label_, builder_.InsertPoint());
    stmt->stmt_->Accept(this);
    Merge(stmt->stmt_->NextList(), stmt->nextlist_);
}


void IRGen::VisitRetStmt(RetStmt* stmt)
{
    if (stmt->retvalue_)
    {
        stmt->retvalue_->Accept(this);
        auto retreg = env_.GetFunction()->ReturnValue();
        builder_.InsertStoreInstr(
            LoadVal(stmt->retvalue_.get()), retreg, false);
    }

    builder_.InsertBrInstr(nullptr);
    env_.AddBrInstr4Ret(static_cast<BrInstr*>(env_.GetLastInstr()));
}


void IRGen::VisitSwitchStmt(SwitchStmt* stmt)
{
    stmt->expr_->Accept(this);
    auto ident = LoadVal(stmt->expr_.get());

    builder_.InsertSwitchInstr(ident);
    auto switchinstr = static_cast<
        SwitchInstr*>(env_.GetLastInstr());

    env_.PushSwitch(switchinstr);
    env_.PushStmt(stmt);

    stmt->stmt_->Accept(this);
    Merge(stmt->stmt_->NextList(), stmt->nextlist_);

    if (!switchinstr->GetDefault())
    {
        auto bb = env_.GetBasicBlock();
        if (!env_.GetLastInstr()->IsControlInstr())
            builder_.InsertBrInstr(bb);
        builder_.InsertPoint() = bb;
        Backpatch(stmt->nextlist_, bb);
        switchinstr->SetDefault(bb);
    }

    env_.PopStmt();
    env_.PopSwitch();
}


void IRGen::VisitTransUnit(TransUnit* tu)
{
    scopestack_.PushNewScope(Scope::ScopeType::file);
    for (auto& decl : tu->declist_)
        decl->Accept(this);
    scopestack_.PopScope();
}


void IRGen::VisitWhileStmt(WhileStmt* stmt)
{
    env_.PushStmt(stmt);

    auto cmpblk = env_.GetBasicBlock(),
        loopblk = env_.GetBasicBlock();

    stmt->continuepoint_ = cmpblk;
    builder_.InsertBrInstr(cmpblk);
    builder_.InsertPoint() = cmpblk;

    stmt->expr_->Accept(this);
    auto cmpans = builder_.InsertCmpInstr(
        env_.GetRegName(),
        Condition::ne, LoadVal(stmt->expr_.get()),
        IntConst::CreateIntConst(builder_.InsertPoint(), 0));

    builder_.InsertBrInstr(cmpans, loopblk, nullptr);
    stmt->nextlist_.push_back(
        static_cast<BrInstr*>(env_.GetLastInstr()));

    builder_.InsertPoint() = loopblk;
    stmt->stmt_->Accept(this);
    builder_.InsertBrInstr(cmpblk);

    Merge(stmt->stmt_->NextList(), stmt->nextlist_);

    env_.PopStmt();
}
