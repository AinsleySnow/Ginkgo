#include "visitast/IRGen.h"
#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "messages/Error.h"
#include <algorithm>
#include <utility>

#define to_int(op) static_cast<const IntConst*>(op)
#define to_float(op) static_cast<const FloatConst*>(op)

const IROperand* IRGen::EvalBinary(
    Tag op, const IROperand* lhs, const IROperand* rhs)
{
#define both_float lhs->IsFloatConst() && rhs->IsFloatConst()
#define both_int lhs->IsIntConst() && rhs->IsIntConst()
#define int_float lhs->IsIntConst() && rhs->IsFloatConst()
#define float_int lhs->IsFloatConst() && rhs->IsIntConst()

#define do_int_calc(sym)                                    \
    if (both_int)                                           \
    {                                                       \
        auto lval = to_int(lhs)->Val();                     \
        auto rval = to_int(rhs)->Val();                     \
        auto ans = lval sym rval;                           \
        return IntConst::CreateIntConst(curfunc_, ans);     \
    }


#define do_calc(sym)                                            \
    if (both_float)                                             \
    {                                                           \
        auto lval = to_float(lhs)->Val();                       \
        auto rval = to_float(rhs)->Val();                       \
        auto ans = lval sym rval;                               \
        return FloatConst::CreateFloatConst(curfunc_, ans);     \
    }                                                           \
    else if (both_int)                                          \
    {                                                           \
        auto lval = to_int(lhs)->Val();                         \
        auto rval = to_int(rhs)->Val();                         \
        auto ans = lval sym rval;                               \
        return IntConst::CreateIntConst(curfunc_, ans);                                          \
    }                                                           \
    else if (int_float)                                         \
    {                                                           \
        auto lval = to_int(lhs)->Val();                         \
        auto rval = to_float(rhs)->Val();                       \
        auto ans = lval sym rval;                               \
        return FloatConst::CreateFloatConst(curfunc_, ans);     \
    }                                                           \
    else if (float_int)                                         \
    {                                                           \
        auto lval = to_float(lhs)->Val();                       \
        auto rval = to_int(rhs)->Val();                         \
        auto ans = lval sym rval;                               \
        return FloatConst::CreateFloatConst(curfunc_, ans);     \
    }

    switch (op)
    {
    case Tag::plus: do_calc(+); break;
    case Tag::minus: do_calc(-); break;
    case Tag::asterisk: do_calc(*); break;
    case Tag::slash: do_calc(/); break;
    case Tag::_and: do_int_calc(&); break;
    case Tag::incl_or: do_int_calc(|); break;
    case Tag::logical_and: do_calc(&&); break;
    case Tag::logical_or: do_calc(||); break;
    case Tag::lshift: do_int_calc(<<); break;
    case Tag::rshift: do_int_calc(>>); break;
    case Tag::lessthan: do_calc(<); break;
    case Tag::greathan: do_calc(>); break;
    case Tag::lessequal: do_calc(<=); break;
    case Tag::greatequal: do_calc(>=); break;
    case Tag::cap: do_int_calc(^); break;
    case Tag::equal: do_calc(==); break;
    case Tag::notequal: do_calc(!=); break;
    default: return nullptr;
    }

    return nullptr;

#undef both_int
#undef both_float
#undef int_float
#undef float_int
#undef do_int_calc
#undef do_calc
}


const IROperand* IRGen::EvalUnary(Tag op, const IROperand* num)
{
    switch (op)
    {
    case Tag::plus: return num;
    case Tag::minus:
        if (num->IsIntConst())
            return IntConst::CreateIntConst(curfunc_, to_int(num)->Val());
        else return FloatConst::CreateFloatConst(curfunc_, to_float(num)->Val());
    case Tag::exclamation:
        if (num->IsIntConst())
            return IntConst::CreateIntConst(curfunc_, !(to_int(num)->Val()));
        else return IntConst::CreateIntConst(curfunc_, !(to_float(num)->Val()));
    case Tag::tilde:
        return IntConst::CreateIntConst(curfunc_, ~(to_int(num)->Val()));
    default: return nullptr;
    }

    return nullptr;
}
#undef to_int
#undef to_float


BasicBlock* IRGen::GetBasicBlock()
{
    std::string name = '%' + std::to_string(index_);
    return curfunc_->AddBasicBlock(name);
}

Instr* IRGen::GetLastInstr()
{
    return curfunc_->GetBasicBlock(-1)->GetLastInstr();
}


const Register* IRGen::AllocaObject(const CType* raw, const std::string& name)
{
    auto ptrty = PtrType::GetPointer(raw->ToIRType());
    auto reg = builder_.InsertAllocaInstr(GetRegName(), ptrty);
    scopestack_.Top().AddObject(name, raw, reg);
    return reg;
}


void IRGen::FillNullBlk(BrInstr* br, BasicBlock* blk)
{
    if (!br->GetTrueBlk()) br->SetTrueBlk(blk);
    if (!br->GetFalseBlk()) br->SetFalseBlk(blk);
}

void IRGen::Backpatch(
    const std::list<BrInstr*>& list, BasicBlock* dest)
{
    for (auto instr : list)
        FillNullBlk(instr, dest);
}

std::list<BrInstr*> IRGen::Merge(
    std::list<BrInstr*>& first, std::list<BrInstr*>& second)
{
    std::list<BrInstr*> newlist{};
    std::merge(first.begin(), first.end(), second.begin(),
        second.end(), newlist.begin());
    return newlist;
}


void IRGen::VisitDeclSpec(DeclSpec* spec)
{
    auto tag = spec->TypeSpec();
    if (tag != TypeTag::customed)
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
        if (initdecl->initalizer_)
            initdecl->initalizer_->Accept(this);
        initdecl->base_ = AllocaObject(
            initdecl->declarator_->RawType(),
            initdecl->declarator_->Name());
    }
}


void IRGen::VisitFuncDef(FuncDef* def)
{
    def->declspec_->Accept(this);
    def->paramlist_->Accept(this);
    auto spec = def->GetDeclSpec();
    const Ptr* ptr = def->GetRawPtr();

    auto retcty = std::move(spec->Type());
    if (ptr) retcty = retcty->AttachPtr(ptr);
    def->return_ = std::move(retcty);

    auto funccty = std::make_unique<CFuncType>(
        std::move(retcty), def->GetParamList().size());
    for (auto param : def->GetParamType())
    {
        auto arthmic = static_cast<const CArithmType*>(param);
        funccty->AddParam(std::make_unique<CArithmType>(*arthmic));
    }
    scopestack_.Top().AddFunc(def->Name(), funccty.get());

    auto pfunc = transunit_->AddFunc(
        def->Name(), funccty->ToIRType()->ToFunction());

    if (!def->compound_)
        return;


    index_ = 0;
    curfunc_ = pfunc;
    builder_.InsertPoint() = GetBasicBlock();
    scopestack_.PushNewScope(Scope::ScopeType::block);

    auto rety = curfunc_->ReturnType();
    std::string retreg = "";
    if (rety)
    {
        retreg = GetRegName();
        curfunc_->ReturnValue() =
            builder_.InsertAllocaInstr(retreg, rety);
    }

    for (auto& param : def->GetParamList())
        if (!param->Name().empty())
            AllocaObject(param->RawType(), param->Name());

    def->compound_->Accept(this);
    if (!builder_.InsertPoint()->Empty())
        builder_.InsertPoint() = GetBasicBlock();

    if (rety)
    {
        auto retvalue = builder_.InsertLoadInstr(
            GetRegName(), curfunc_->ReturnValue());
        builder_.InsertRetInstr(retvalue);
    }
    else
        builder_.InsertRetInstr();

    Backpatch(ret_, builder_.InsertPoint());
    for (auto gotopair : gotomap_)
        FillNullBlk(gotopair.first, labelmap_[gotopair.second]);

    ret_.clear();
    labelmap_.clear();
    gotomap_.clear();
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


void IRGen::VisitTransUnit(DeclStmt* decl)
{
    decl->Accept(this);
}


void IRGen::VisitAssignExpr(AssignExpr* assign)
{
    assign->left_->Accept(this);
    assign->right_->Accept(this);

    auto lhs = static_cast<
        const Register*>(assign->left_->Val());
    auto rhs = assign->right_->Val();
    auto lhsty = assign->left_->Val()->Type();
    auto rhsty = assign->right_->Val()->Type();

    if (assign->op_ == Tag::assign)
    {
        builder_.InsertStoreInstr(rhs, lhs, false);
        assign->Val() = lhs;
        return;
    }

    auto regname = GetRegName();
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


    builder_.InsertStoreInstr(result, lhs, false);
    assign->Val() = result;
}


void IRGen::VisitBinaryExpr(BinaryExpr* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);

    if (bin->left_->IsConstant() && bin->right_->IsConstant())
    {
        bin->Val() = EvalBinary(
            bin->op_, bin->left_->Val(), bin->right_->Val());
        return;
    }

    auto lhs = bin->left_->Val(), rhs = bin->right_->Val();
    auto lhsty = lhs->Type(), rhsty = rhs->Type();
    bool hasfloat = lhsty->IsFloat() || rhsty->IsFloat();

    std::string regname = GetRegName();
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
    else if (bin->op_ == Tag::incl_or)
        result = builder_.InsertXorInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::_or)
        result = builder_.InsertOrInstr(regname, lhs, rhs);

    bin->Val() = result;
}


void IRGen::VisitCallExpr(CallExpr* call)
{
    call->postfix_->Accept(this);
    call->argvlist_->Accept(this);

    auto pfunc = static_cast<const Register*>(call->postfix_->Val());

    IdentExpr* ident = dynamic_cast<IdentExpr*>(call->postfix_.get());
    if (ident)
    {
        const FuncType* functy =
            pfunc->Type()->ToPointer()->Point2()->ToFunction();        
        call->Val() = builder_.InsertCallInstr(GetRegName(), functy, ident->name_);
    }
    else // if a function is called through a pointer
    {
        auto func = builder_.InsertLoadInstr(GetRegName(), pfunc);
        call->Val() = builder_.InsertCallInstr(GetRegName(), func);
    }

    auto callinstr = static_cast<CallInstr*>(GetLastInstr());
    for (auto& argv : *call->argvlist_)
        callinstr->AddArgv(argv->Val());
}


void IRGen::VisitCastExpr(CastExpr* cast)
{
    cast->typename_->Accept(this);
    cast->expr_->Accept(this);

    auto& ty = cast->typename_->Type();
    auto expreg = cast->expr_->Val();

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
    if (cond->IsConstant())
    {
        cond->cond_->Accept(this);
        auto sel = static_cast<const Constant*>(cond->cond_->Val());
        if (sel->IsZero())
        {
            cond->false_->Accept(this);
            cond->Val() = cond->false_->Val();
        }
        else
        {
            cond->true_->Accept(this);
            cond->Val() = cond->true_->Val();
        }
        return;
    }

    cond->cond_->Accept(this);
    auto trueblk = GetBasicBlock(),
        falseblk = GetBasicBlock(),
        endblk = GetBasicBlock();
    builder_.InsertBrInstr(cond->cond_->Val(), trueblk, falseblk);

    builder_.InsertPoint() = trueblk;
    cond->true_->Accept(this);
    builder_.InsertBrInstr(endblk);

    builder_.InsertPoint() = falseblk;
    cond->false_->Accept(this);
    builder_.InsertBrInstr(endblk);

    builder_.InsertPoint() = endblk;
    auto sel = cond->cond_->Val();
    auto tval = cond->true_->Val();
    auto fval = cond->false_->Val();

    cond->Val() =
        builder_.InsertPhiInstr(GetRegName(), tval->Type());
    auto phi = static_cast<PhiInstr*>(GetLastInstr());
    phi->AddBlockValPair(trueblk, tval);
    phi->AddBlockValPair(falseblk, fval);
}


void IRGen::VisitConstant(ConstExpr* constant)
{
    auto ctype = constant->RawType();
    if (ctype->IsInteger())
        constant->Val() = IntConst::CreateIntConst(
            curfunc_, constant->GetInt(), ctype->ToIRType()->ToInteger());
    else
        constant->Val() = FloatConst::CreateFloatConst(
            curfunc_, constant->GetFloat(), ctype->ToIRType()->ToFloatPoint());
}


void IRGen::VisitExprList(ExprList* list)
{
    for (auto& expr : list->exprlist_)
        expr->Accept(this);
}


void IRGen::VisitIdentExpr(IdentExpr* ident)
{
    auto object = scopestack_.SearchObject(ident->name_);
    ident->Val() = builder_.InsertLoadInstr(
        GetRegName(), object->GetAddr());
}


void IRGen::VisitLogicalExpr(LogicalExpr* logical)
{
    if (logical->left_->IsConstant() && logical->right_->IsConstant())
    {
        logical->left_->Accept(this);
        logical->right_->Accept(this);
        auto lhs = static_cast<const Constant*>(logical->left_->Val());
        auto rhs = static_cast<const Constant*>(logical->right_->Val());
        logical->Val() = EvalBinary(logical->op_, lhs, rhs);
        return;
    }
    else if (logical->left_->IsConstant())
    {
        logical->left_->Accept(this);
        auto lhs = static_cast<const Constant*>(logical->left_->Val());
        if ((lhs->IsZero() && logical->op_ == Tag::logical_and) ||
            (!lhs->IsZero() && logical->op_ == Tag::logical_or))
            logical->Val() = IntConst::CreateIntConst(curfunc_, !lhs->IsZero());
        else
        {
            logical->right_->Accept(this);
            auto zero = IntConst::CreateIntConst(curfunc_, 0);
            auto one = IntConst::CreateIntConst(curfunc_, 1);
            logical->Val() = builder_.InsertSelectInstr(
                GetRegName(), logical->right_->Val(), true, one, zero);
        }
        return;
    }

    auto firstblk = GetBasicBlock(),
        midblk = GetBasicBlock(),
        finalblk = GetBasicBlock();

    builder_.InsertBrInstr(firstblk);
    builder_.InsertPoint() = firstblk;

    logical->left_->Accept(this);
    auto lhs = static_cast<const Register*>(logical->left_->Val());
    if (logical->left_->IsLVal())
        lhs = builder_.InsertLoadInstr(GetRegName(), lhs);

    auto zero = IntConst::CreateIntConst(curfunc_, 0);
    auto cmpans = builder_.InsertCmpInstr(GetRegName(), Condition::ne, lhs, zero);
    
    if (logical->op_ == Tag::logical_and)
        builder_.InsertBrInstr(cmpans, midblk, finalblk);
    else if (logical->op_ == Tag::logical_or)
        builder_.InsertBrInstr(cmpans, finalblk, midblk);

    builder_.InsertPoint() = midblk;

    logical->right_->Accept(this);
    auto rhs = static_cast<const Register*>(logical->right_->Val());
    if (logical->right_->IsLVal())
        rhs = builder_.InsertLoadInstr(GetRegName(), rhs);

    cmpans = builder_.InsertCmpInstr(GetRegName(), Condition::ne, rhs, zero);
    builder_.InsertBrInstr(finalblk);

    builder_.InsertPoint() = finalblk;

    auto result = GetRegName();
    if (logical->op_ == Tag::logical_and)
    {
        logical->Val() = builder_.InsertPhiInstr(result, IntType::GetInt8(true));
        auto phi = static_cast<PhiInstr*>(GetLastInstr());
        phi->AddBlockValPair(firstblk, zero);
        phi->AddBlockValPair(midblk, cmpans);
    }
    else if (logical->op_ == Tag::logical_or)
    {
        logical->Val() = builder_.InsertPhiInstr(result, IntType::GetInt8(true));
        auto phi = static_cast<PhiInstr*>(GetLastInstr());
        phi->AddBlockValPair(
            firstblk,
            IntConst::CreateIntConst(curfunc_, 1));
        phi->AddBlockValPair(midblk, cmpans);
    }
}


void IRGen::VisitUnaryExpr(UnaryExpr* unary)
{
    unary->content_->Accept(this);

    if (unary->content_->IsConstant())
    {
        unary->Val() = EvalUnary(unary->op_, unary->content_->Val());
        return;
    }

    if (unary->op_ == Tag::inc || unary->op_ == Tag::dec)
    {
        auto op = static_cast<const Register*>(unary->content_->Val());
        auto reg = builder_.InsertLoadInstr(GetRegName(), op);
        auto one = IntConst::CreateIntConst(curfunc_, 1);
        if (unary->op_ == Tag::inc)
            builder_.InsertAddInstr(GetRegName(), reg, one);
        else
            builder_.InsertSubInstr(GetRegName(), reg, one);

        builder_.InsertStoreInstr(reg, op, false);
        unary->Val() = reg;
    }
    else if (unary->op_ == Tag::_and || unary->op_ == Tag::plus)
        unary->Val() = unary->content_->Val();
    else if (unary->op_ == Tag::asterisk)
    {
        auto addreg = static_cast<const Register*>(unary->content_->Val());
        unary->Val() = builder_.InsertLoadInstr(GetRegName(), addreg);
    }
    else if (unary->op_ == Tag::minus)
    {
        auto zero = IntConst::CreateIntConst(curfunc_, 0);
        auto rhs = unary->content_->Val();
        unary->Val() = builder_.InsertSubInstr(GetRegName(), zero, rhs);
    }
    else if (unary->op_ == Tag::tilde)
    {
        auto minusone = IntConst::CreateIntConst(curfunc_, ~0);
        auto rhs = unary->content_->Val();
        unary->Val() = builder_.InsertXorInstr(GetRegName(), minusone, rhs);
    }
    else if (unary->op_ == Tag::exclamation)
    {
        auto zero = IntConst::CreateIntConst(curfunc_, 0);
        auto one = IntConst::CreateIntConst(curfunc_, 1);
        unary->Val() = builder_.InsertSelectInstr(
            GetRegName(), unary->content_->Val(), true, zero, one);
    }
}


void IRGen::VisitBreakStmt(BreakStmt* stmt)
{
    builder_.InsertBrInstr(nullptr);
    brkcntn_.top()->NextList().push_back(
        static_cast<BrInstr*>(GetLastInstr()));
}


void IRGen::VisitCaseStmt(CaseStmt* stmt)
{
    stmt->stmt_->Accept(this);
    stmt->nextlist_ = stmt->stmt_->NextList();
}


void IRGen::VisitCompoundStmt(CompoundStmt* compound)
{
    scopestack_.PushNewScope(Scope::ScopeType::block);
    for (auto& stmt : compound->stmtlist_)
    {
        stmt->Accept(this);
        if (!stmt->NextList().empty())
        {
            auto blk = GetBasicBlock();
            Backpatch(stmt->NextList(), blk);
            builder_.InsertPoint() = blk;
        }
    }
    scopestack_.PopScope();
}


void IRGen::VisitContinueStmt(ContinueStmt* stmt)
{
    IterStmt* iter = static_cast<IterStmt*>(brkcntn_.top());
    builder_.InsertBrInstr(iter->continuepoint_);
}


void IRGen::VisitDoWhileStmt(DoWhileStmt* stmt)
{
    brkcntn_.push(stmt);

    auto loopblk = GetBasicBlock();
    builder_.InsertBrInstr(loopblk);
    stmt->continuepoint_ = loopblk;

    builder_.InsertPoint() = loopblk;
    stmt->stmt_->Accept(this);

    stmt->expr_->Accept(this);
    auto cmpans = builder_.InsertCmpInstr(
        GetRegName(),
        Condition::ne, stmt->expr_->Val(),
        IntConst::CreateIntConst(curfunc_, 0));

    builder_.InsertBrInstr(cmpans, loopblk, nullptr);

    stmt->nextlist_.push_back(
        static_cast<BrInstr*>(GetLastInstr()));
    stmt->nextlist_ = Merge(stmt->nextlist_, stmt->stmt_->NextList());

    brkcntn_.pop();
}


void IRGen::VisitExprStmt(ExprStmt* stmt)
{
    stmt->expr_->Accept(this);
}


void IRGen::VisitForStmt(ForStmt* stmt)
{
    brkcntn_.push(stmt);

    if (stmt->init_)
        stmt->init_->Accept(this);

    BasicBlock* cmpblk = nullptr;
    BasicBlock* loopblk = GetBasicBlock();

    if (stmt->condition_)
    {
        cmpblk = GetBasicBlock();
        stmt->continuepoint_ = cmpblk;

        builder_.InsertBrInstr(cmpblk);
        builder_.InsertPoint() = cmpblk;

        stmt->condition_->Accept(this);
        auto cmpans = builder_.InsertCmpInstr(
            GetRegName(),
            Condition::ne, stmt->condition_->expr_->Val(),
            IntConst::CreateIntConst(curfunc_, 0));

        builder_.InsertBrInstr(cmpans, loopblk, nullptr);
        stmt->nextlist_.push_back(
            static_cast<BrInstr*>(GetLastInstr()));
    }
    else
        stmt->continuepoint_ = loopblk;

    builder_.InsertPoint() = loopblk;
    if (stmt->increment_)
        stmt->increment_->Accept(this);
    stmt->body_->Accept(this);

    if (stmt->condition_)
        builder_.InsertBrInstr(cmpblk);
    else
        builder_.InsertBrInstr(loopblk);

    stmt->nextlist_ = Merge(stmt->nextlist_, stmt->body_->NextList());
    brkcntn_.pop();
}


void IRGen::VisitGotoStmt(GotoStmt* stmt)
{
    builder_.InsertBrInstr(nullptr);
    auto br = static_cast<BrInstr*>(GetLastInstr());
    gotomap_.emplace(br, stmt->ident_);

    auto bb = GetBasicBlock();
    builder_.InsertPoint() = bb;
}


void IRGen::VisitIfStmt(IfStmt* stmt)
{
    BasicBlock* trueblk = GetBasicBlock();
    BasicBlock* falseblk = nullptr;

    stmt->expr_->Accept(this);
    auto cmpans = builder_.InsertCmpInstr(
        GetRegName(),
        Condition::ne, stmt->expr_->Val(),
        IntConst::CreateIntConst(curfunc_, 0));

    if (stmt->false_)
    {
        falseblk = GetBasicBlock();
        builder_.InsertBrInstr(cmpans, trueblk, falseblk);
    }
    else
    {
        builder_.InsertBrInstr(cmpans, trueblk, nullptr);
        stmt->nextlist_.push_back(
            static_cast<BrInstr*>(GetLastInstr()));
    }

    builder_.InsertPoint() = trueblk;
    stmt->true_->Accept(this);

    if (stmt->false_)
    {
        builder_.InsertBrInstr(nullptr);
        stmt->nextlist_.push_back(
            static_cast<BrInstr*>(GetLastInstr()));

        builder_.InsertPoint() = falseblk;
        stmt->false_->Accept(this);
    }

    if (!stmt->false_) // if (...) {...}
        stmt->nextlist_ = Merge(stmt->nextlist_, stmt->true_->NextList());
    else // if (...) {...} else ...
        stmt->nextlist_ = Merge(stmt->true_->NextList(), stmt->false_->NextList());
}


void IRGen::VisitLabelStmt(LabelStmt* stmt)
{
    auto bb = GetBasicBlock();
    labelmap_.emplace(stmt->label_, bb);
    builder_.InsertPoint() = bb;
    stmt->stmt_->Accept(this);
}


void IRGen::VisitRetStmt(RetStmt* stmt)
{
    if (stmt->retvalue_)
    {
        stmt->retvalue_->Accept(this);
        auto retreg = curfunc_->ReturnValue();
        builder_.InsertStoreInstr(
            stmt->retvalue_->Val(), retreg, false);
    }

    builder_.InsertBrInstr(nullptr);
    ret_.push_back(static_cast<BrInstr*>(GetLastInstr()));
}


void IRGen::VisitSwitchStmt(SwitchStmt* stmt)
{
    stmt->expr_->Accept(this);
    auto ident = stmt->expr_->Val();

    builder_.InsertSwitchInstr(ident);
    auto switchinstr = static_cast<SwitchInstr*>(GetLastInstr());

    auto compound = dynamic_cast<CompoundStmt*>(stmt->stmt_.get());
    if (!compound) return;
    for (auto& line : compound->stmtlist_)
    {
        auto _case = dynamic_cast<CaseStmt*>(line.get());
        if (!_case) continue;
        brkcntn_.push(stmt);
        line->Accept(this);
        brkcntn_.pop();
        stmt->nextlist_ = Merge(stmt->nextlist_, line->NextList());
    }
}


void IRGen::VisitWhileStmt(WhileStmt* stmt)
{
    brkcntn_.push(stmt);

    auto cmpblk = GetBasicBlock(),
        loopblk = GetBasicBlock();

    stmt->continuepoint_ = cmpblk;
    builder_.InsertBrInstr(cmpblk);
    builder_.InsertPoint() = cmpblk;

    stmt->expr_->Accept(this);
    auto cmpans = builder_.InsertCmpInstr(
        GetRegName(),
        Condition::ne, stmt->expr_->Val(),
        IntConst::CreateIntConst(curfunc_, 0));

    builder_.InsertBrInstr(cmpans, loopblk, nullptr);
    stmt->nextlist_.push_back(
        static_cast<BrInstr*>(GetLastInstr()));

    builder_.InsertPoint() = loopblk;
    stmt->stmt_->Accept(this);
    builder_.InsertBrInstr(cmpblk);

    stmt->nextlist_ = Merge(stmt->nextlist_, stmt->stmt_->NextList());

    brkcntn_.pop();
}
