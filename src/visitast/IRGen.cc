#include "visitast/IRGen.h"
#include "visitast/Evaluator.h"
#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <utf8.h>


void IRGen::CurrentEnv::Epilogue(BasicBlock* bb)
{
    Backpatch(ret_, bb);
    for (auto gotopair : gotomap_)
        FillNullBlk(gotopair.first, labelmap_[gotopair.second]);

    for (auto blk : *GetFunction())
        if (!blk->LastInstr()->IsControlInstr())
            blk->Append(std::make_unique<BrInstr>(bb));
}


const Register* IRGen::AllocaObject(const CType* raw, const std::string& name)
{
    if (scopestack_.Top().GetScopeType() == Scope::ScopeType::file)
    {
        auto ty = raw->ToIRType(transunit_.get());
        auto regname = '@' + name;
        auto var = GlobalVar::CreateGlobalVar(transunit_.get(), regname, ty);

        env_ = CurrentEnv(var);
        auto reg = Register::CreateRegister(
            env_.GetGlobalVar(), regname,
            PtrType::GetPtrType(env_.GetGlobalVar(), ty));

        var->Addr() = reg;
        scopestack_.Top().AddObject(name, raw, reg);
        return reg;
    }
    else
    {
        auto reg = ibud_.InsertAllocaInstr(
            env_.GetRegName(), raw->ToIRType(ibud_.Container()));
        scopestack_.Top().AddObject(name, raw, reg);
        return reg;
    }
}

Function* IRGen::AllocaFunc(const CFuncType* raw, const std::string& name)
{
    Function* pfunc = nullptr;
    if (scopestack_.SearchFunc(name))
        pfunc = transunit_->GetFunction('@' + name);
    else
    {
        auto functy = raw->ToIRType(transunit_.get());
        auto irname = '@' + name;

        pfunc = transunit_->AddFunc(irname, functy);
        pfunc->Inline() = raw->Inline();
        pfunc->Noreturn() = raw->Noreturn();
        pfunc->Addr() = Register::CreateRegister(
            transunit_.get(), irname,
            PtrType::GetPtrType(transunit_.get(),
            PtrType::GetPtrType(transunit_.get(), functy)));

        scopestack_.File().AddFunc(name, raw, pfunc->Addr());
    }

    return pfunc;
}


void IRGen::TypeInferenceHelper(Declaration* decl, Expr* init)
{
    // scenario one point five: int func(void); auto f = func;
    // 'func' has CFuncType, and f should be a pointer to function.
    // We treate it as a special case.
    if (decl->Child()->IsDeclSpec() && init->Type()->Is<CFuncType>())
        decl->Type() = std::make_unique<CPtrType>(init->Type()->Clone());

    // scenario one: auto a = 3; (no TypeSpec at all)
    else if (decl->Child()->IsDeclSpec())
        decl->Type() = init->Type()->Clone();

    // scenario two: int a = 3; auto* b = &a;
    // (no TypeSpec as well, but have declared pointer or array)
    // Note that CodeChk will consisder it as an error if the levels
    // of pointers on either side of the equal sign do not match.
    // Initializers are ignored for the time being, therefore only
    // inference via expression is consisdered.
    else
    {
        auto clone = init->Type()->InnerMost()->Clone();
        decl->Type()->InnerMost()->SetChild(std::move(clone));
    }
}


static const FuncType* GetAssertProto()
{
    const static PtrType ptr{ IntType::GetInt8(true) };
    const static FuncType assert{
        VoidType::GetVoidType(), {
            IntType::GetInt8(true), &ptr, &ptr,
            IntType::GetInt32(false), &ptr
        },
        false
    };
    return &assert;
}

static const PtrType* GetVoidPtr()
{
    const static PtrType vptr{ VoidType::GetVoidType() };
    return &vptr;
}

static const PtrType* GetVaElemPtr()
{
    const static StructType vaelem{
        "__Ginkgo_va_elem", 24, 8, {
            std::make_pair(IntType::GetInt8(true), 0),
            std::make_pair(IntType::GetInt8(true), 4),
            std::make_pair(GetVoidPtr(), 8),
            std::make_pair(GetVoidPtr(), 16)
        }
    };
    const static PtrType ptr{ &vaelem };
    return &ptr;
}

static const FuncType* GetVaStartProto()
{
    const static FuncType vastart{
        VoidType::GetVoidType(), { GetVaElemPtr() }, true
    };
    return &vastart;
}

static const FuncType* GetVaArgProto()
{
    const static FuncType vaarg{
        GetVoidPtr(), {
            GetVaElemPtr(),
            IntType::GetInt32(true),
            IntType::GetInt32(true)
        },
        false
    };
    return &vaarg;
}

static const FuncType* GetVaEndProto()
{
    const static FuncType vaend{
        GetVoidPtr(), { GetVaElemPtr() },
        false
    };
    return &vaend;
}


const FuncType* IRGen::HandleBuiltins(CallExpr* call)
{
    if (!call->Postfix()->IsIdentifier())
        return nullptr;

    auto name = call->Postfix()->ToIdentifier()->Name();
    if (name == "__Ginkgo_assert")
    {
        auto ptr = GetAssertProto();
        ibud_.InsertCallInstr("", ptr, '@' + name);
        return ptr;
    }
    else if (name == "__Ginkgo_va_end")
    {
        auto ptr = GetVaEndProto();
        ibud_.InsertCallInstr("", ptr, '@' + name);
        return ptr;
    }

    return nullptr;
}

void IRGen::HandleVaStart(CallExpr* call)
{
    // Only evaluate the first argument of va_start.
    auto first = call->ArgvList()->begin();
    (*first)->Accept(this);

    // Directly generate the instruction and
    // append the processed arguments.
    auto casted = ibud_.InsertBitcastInstr(
        env_.GetRegName(), GetVaElemPtr(), (*first)->Val()->As<Register>());
    ibud_.InsertCallInstr("", GetVaStartProto(), "@__Ginkgo_va_start");

    auto callinstr = ibud_.LastInstr()->As<CallInstr>();
    callinstr->AddArgv(casted);

    // What if __Ginkgo_va_start takes more than two arguments?
    auto second = std::next(first);
    if (second == call->ArgvList()->end())
    // We are done if no more arguments
        return;

    auto& paramlist = env_.GetParamList()->GetParamList();
    for (auto i = paramlist.crbegin(); i != paramlist.crend(); ++i)
    {
        // (Will be) Guaranteed by CodeChk. If i is not the
        // last argument with known address, CodeChk (will)
        // should have reported it as a warning.
        if ((*i)->ToObjDef()->Name() == (*second)->ToIdentifier()->Name())
        {
            auto offset = std::distance(paramlist.begin(), i.base()) - 1;
            (*second)->Val() = env_.GetFunction()->Params()[offset];
            break;
        }
    }
    callinstr->AddArgv((*second)->Val());
}

void IRGen::HandleVaArg(CallExpr* call)
{
    call->TypeName()->Accept(this);
    auto ty = call->TypeName()->RawType();
    std::string name = "";
    if (ty->Is<CArithmType>())
    {
        auto aty = ty->As<CArithmType>();
        if (aty->IsFloat())
            name = "@__Ginkgo_va_arg_fp";
        else
            name = "@__Ginkgo_va_arg_gp";
    }
    else
        name = "@__Ginkgo_va_arg_mem";

    call->ArgvList()->Accept(this);

    auto casted = ibud_.InsertBitcastInstr(env_.GetRegName(),
        GetVaElemPtr(), LoadVal(call->ArgvList()->begin()->get())->As<Register>());
    call->Val() = ibud_.InsertCallInstr(
        env_.GetRegName(), GetVaArgProto(), name);

    auto callinstr = ibud_.LastInstr()->As<CallInstr>();        
    callinstr->AddArgv(casted);
    callinstr->AddArgv(
        IntConst::CreateIntConst(ibud_.Container(), ty->Size()));
    callinstr->AddArgv(
        IntConst::CreateIntConst(ibud_.Container(), ty->Align()));
}


const IROperand* IRGen::LoadVal(Expr* expr)
{
    if (expr->IsIdentifier())
    {
        auto ident = expr->ToIdentifier();
        if (ident->Addr()->Type()->As<PtrType>()->Point2()->Is<ArrayType>())
        // tl;dr: if ident is a pointer to some array
        {
            // Then... it must be a 'real' array
            auto inner = !ident->Type()->As<CArrayType>()->IsParam();
            // inner not set? That means the array has more
            // than one dimension and is passed to a function.
            auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
            ident->Val() = ibud_.InsertGetElePtrInstr(
                env_.GetRegName(), inner, ident->Addr(), zero);
        }
        else
            ident->Val() = ibud_.InsertLoadInstr(env_.GetRegName(), ident->Addr());
        return ident->Val();
    }
    else if (expr->IsSubscript())
    {
        expr->Val() = ibud_.InsertLoadInstr(
            env_.GetRegName(), expr->ToSubscript()->Addr());
        return expr->Val();
    }
    else if (expr->IsUnary() && expr->ToUnary()->Op() == Tag::asterisk)
    {
        expr->Val() = ibud_.InsertLoadInstr(
            env_.GetRegName(), expr->Val()->As<Register>());
        return expr->Val();
    }
    else if (expr->IsStrExpr())
    {
        auto zero = IntConst::CreateIntConst(
            ibud_.Container(), 0, IntType::GetInt32(true));
        expr->Val() = ibud_.InsertGetElePtrInstr(
            env_.GetRegName(), true, expr->Val()->As<Register>(), zero);
        return expr->Val();
    }
    else if (expr->IsAccess())
    {
        // Val of AccessExpr is always an address
        expr->Val() = ibud_.InsertLoadInstr(
            env_.GetRegName(), expr->Val()->As<Register>());
        return expr->Val();
    }
    else if (expr->IsExprList())
    {
        expr->Val() = LoadVal(expr->ToExprList()->Back().get());
        return expr->Val();
    }
    else
        return expr->Val();
}

const Register* IRGen::LoadAddr(Expr* expr)
{
    if (expr->IsIdentifier())
        return expr->ToIdentifier()->Addr();
    else if (expr->IsSubscript())
        return expr->ToSubscript()->Addr();
    else if (expr->IsExprList())
        return LoadAddr(expr->ToExprList()->Back().get());
    // else if (expr->IsUnary() && expr->ToUnary()->Op() == Tag::_and)
    // else if (expr->IsAccess())
    // no Addr method in UnaryExpr and AccessExpr; if address-of is used,
    // the Val() of expr is just the desired address
    return expr->Val()->As<Register>();
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


void IRGen::VisitArrayDef(ArrayDef* def)
{
    tbud_.VisitArrayDef(def);
}


void IRGen::VisitDeclSpec(DeclSpec* spec)
{
    tbud_.VisitDeclSpec(spec);
}


void IRGen::VisitDeclList(DeclList* list)
{
    for (auto& initdecl : *list)
    {
        initdecl->declarator_->Accept(this);

        bool notfunc = !initdecl->declarator_->Child()->IsFuncDef();
        bool tyinf = initdecl->declarator_->
            InnerMost()->ToDeclSpec()->NeedInference();
        if (notfunc && initdecl->initalizer_)
            initdecl->initalizer_->Accept(this);
        if (tyinf)
        {
            TypeInferenceHelper(
                initdecl->declarator_.get(), initdecl->initalizer_.get());
        }

        auto raw = initdecl->declarator_->RawType();
        auto name = initdecl->declarator_->ToObjDef()->Name();
        if (raw->Storage().IsTypedef())
        {
            scopestack_.Top().AddTypedef(name, raw);
            continue;
        }

        if (notfunc)
        {
            initdecl->base_ = AllocaObject(raw, name);

            if (initdecl->initalizer_)
            {
                // only insert store instr when we're translating
                // a function. the expr tree will be directly add
                // to the tree.
                if (env_.InFunction())
                {
                    auto val = LoadVal(initdecl->initalizer_.get());
                    ibud_.InsertStoreInstr(val, initdecl->base_, false);
                }
                else
                    env_.GetGlobalVar()->Dump2Tree();
            }
        }
    }
}


void IRGen::VisitFuncDef(FuncDef* def)
{
    // Note: function body is translated in
    // VisitObjDef below
    tbud_.VisitFuncDef(def);
}


void IRGen::VisitObjDef(ObjDef* def)
{
    tbud_.VisitObjDef(def);

    if (!def->Child()->IsFuncDef())
        return;

    Function* pfunc = AllocaFunc(
        static_cast<const CFuncType*>(def->RawType()),
        def->Name());

    if (!def->compound_) 
        return;

    // a function with function body
    // It seems weird to translate function body
    // in a method called 'VisitObjDef', but...
    // what if functions are regarded as first-class OBJECTS?
    // (Well, the real reason I choose this structure
    // is that it can greatly simplify the code)

    env_ = CurrentEnv(pfunc);
    bbud_.SetInsertPoint(pfunc);

    ibud_.SetInsertPoint(
        bbud_.GetBasicBlock(env_.GetLabelName()));
    scopestack_.PushNewScope(Scope::ScopeType::block);

    auto rety = env_.GetFunction()->ReturnType();
    if (!rety->Is<VoidType>())
    {
        env_.GetFunction()->ReturnValue() =
            ibud_.InsertAllocaInstr(env_.GetRegName(), rety);
    }

    auto funcdef = def->Child()->ToFuncDef();
    env_.SetParamList(funcdef->RawParamList());
    for (auto& param : funcdef->GetParamList())
    {
        if (param->RawType()->As<CVoidType>())
            continue;
        auto ctype = param->RawType();
        auto paramreg = Register::CreateRegister(
            ibud_.Container(), env_.GetRegName(),
            ctype->ToIRType(ibud_.Container()));

        env_.GetFunction()->AddParam(paramreg);

        if (param->IsObjDef())
        {
            auto name = param->ToObjDef()->Name();
            auto addr = AllocaObject(ctype, name);
            ibud_.InsertStoreInstr(paramreg, addr, false);
        } 
    }

    def->compound_->Accept(this);
    if (!ibud_.Container()->Empty())
        ibud_.SetInsertPoint(bbud_.GetBasicBlock(env_.GetLabelName()));

    if (!rety->Is<VoidType>())
    {
        auto retvalue = ibud_.InsertLoadInstr(
            env_.GetRegName(), env_.GetFunction()->ReturnValue());
        ibud_.InsertRetInstr(retvalue);
    }
    else ibud_.InsertRetInstr();

    env_.Epilogue(ibud_.Container());
    scopestack_.PopScope();
}


void IRGen::VisitParamList(ParamList* list)
{
    for (auto& param : list->GetParamList())
    {
        param->Accept(this);
        auto raw = param->RawType();
        if (auto array = raw->As<CArrayType>(); array)
            const_cast<CArrayType*>(array)->SetIsParam();
        list->AppendType(param->RawType());
    }
}


void IRGen::VisitPtrDef(PtrDef* def)
{
    tbud_.VisitPtrDef(def);
}


void IRGen::VisitAccessExpr(AccessExpr* access)
{
    access->Postfix()->Accept(this);
    tbud_.VisitAccessExpr(access);

    int index = 0;
    const Register* addr = nullptr;
    const CHeterType* heterty = nullptr;

    if (access->Op() == Tag::arrow)
    {
        addr = LoadVal(access->Postfix().get())->As<Register>();
        heterty = access->Postfix()->Type()->
            As<CPtrType>()->Point2()->As<CHeterType>();
    }
    else
    {
        addr = LoadAddr(access->Postfix().get());
        heterty = access->Postfix()->Type()->As<CHeterType>();
    }

    while (true)
    {
        index = (*heterty)[access->Field()];
        auto [next, ty, _] = (*heterty)[index];
        addr = ibud_.InsertGetElePtrInstr(env_.GetRegName(), false, addr, index);
        if (!next)
            break;
        // if field is true: yet another heterogeneous field
        heterty = ty->As<CHeterType>();
    }

    access->Val() = addr;
}


void IRGen::VisitArrayExpr(ArrayExpr* array)
{
    array->identifier_->Accept(this);
    array->index_->Accept(this);
    tbud_.VisitArrayExpr(array);

    const Register* addr = LoadAddr(array->identifier_.get());
    if (addr->Type()->As<PtrType>()->Point2()->Is<PtrType>())
    // tl;dr: if array yields a pointer
        addr = ibud_.InsertLoadInstr(env_.GetRegName(), addr);

    auto inner = true;
    if (auto arr = array->identifier_->Type()->As<CArrayType>(); arr)
        inner = !arr->IsParam();
    array->Addr() = ibud_.InsertGetElePtrInstr(
        env_.GetRegName(), inner, addr, LoadVal(array->index_.get()));
}


#define IS_ARITHM(op)   op->Type()->Is<CArithmType>()
#define IS_PTR(op)      op->Type()->Is<CPtrType>()
#define IS_ARRAY(op)    op->Type()->Is<CArrayType>()
#define AS_REG(op)      op->Val()->As<Register>()

void IRGen::VisitAssignExpr(AssignExpr* assign)
{
    assign->left_->Accept(this);
    assign->right_->Accept(this);
    tbud_.VisitAssignExpr(assign);

    auto rhs = LoadVal(assign->right_.get());
    const Register* addr = LoadAddr(assign->left_.get());

    if (assign->op_ == Tag::assign)
    {
        ibud_.InsertStoreInstr(rhs, addr, false);
        assign->Val() = rhs;
        return;
    }

    auto lhs = LoadVal(assign->left_.get());
    auto lhsty = lhs->Type();
    auto rhsty = rhs->Type();

    auto regname = env_.GetRegName();
    const Register* result = nullptr;

    if (IS_PTR(assign->left_) && IS_ARITHM(assign->right_) && assign->op_ == Tag::add_assign)
        result = ibud_.InsertGetElePtrInstr(regname, true, lhs->As<Register>(), rhs);
    else if (IS_PTR(assign->left_) && IS_ARITHM(assign->right_) && assign->op_ == Tag::sub_assign)
    {
        auto size = lhsty->As<PtrType>()->Point2()->Size();
        const IROperand* newrhs = nullptr;
        if (rhs->Is<IntConst>())
        {
            newrhs = IntConst::CreateIntConst(ibud_.Container(),
                rhs->As<IntConst>()->Val() * size, rhs->Type()->As<IntType>());
        }
        else
        {
            auto sizeconst = IntConst::CreateIntConst(
                ibud_.Container(), size, rhs->Type()->As<IntType>());
            newrhs = ibud_.InsertMulInstr(env_.GetRegName(), rhs, sizeconst);
        }
        result = ibud_.InsertSubInstr(regname, lhs->As<Register>(), newrhs);
    }

    else if (assign->op_ == Tag::add_assign && lhsty->Is<IntType>())
        result = ibud_.InsertAddInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::add_assign && lhsty->Is<FloatType>())
        result = ibud_.InsertFaddInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::sub_assign && lhsty->Is<IntType>())
        result = ibud_.InsertSubInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::sub_assign && lhsty->Is<FloatType>())
        result = ibud_.InsertFsubInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::mul_assign && lhsty->Is<IntType>())
        result = ibud_.InsertMulInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::mul_assign && lhsty->Is<FloatType>())
        result = ibud_.InsertFmulInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::div_assign && lhsty->Is<IntType>())
        result = ibud_.InsertDivInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::div_assign && lhsty->Is<FloatType>())
        result = ibud_.InsertFdivInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::mod_assign && lhsty->Is<IntType>())
        result = ibud_.InsertModInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::left_assign)
        result = ibud_.InsertShlInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::right_assign && lhsty->As<IntType>()->IsSigned())
        result = ibud_.InsertAshrInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::right_assign)
        result = ibud_.InsertLshrInstr(regname, lhs, rhs);

    else if (assign->op_ == Tag::and_assign)
        result = ibud_.InsertAndInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::or_assign)
        result = ibud_.InsertOrInstr(regname, lhs, rhs);
    else if (assign->op_ == Tag::xor_assign)
        result = ibud_.InsertXorInstr(regname, lhs, rhs);


    ibud_.InsertStoreInstr(result, addr, false);
    assign->Val() = result;
}


void IRGen::VisitBinaryExpr(BinaryExpr* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);
    tbud_.VisitBinaryExpr(bin);

    if (bin->left_->IsConstant() && bin->right_->IsConstant())
    {
        Pool<IROperand>* container = nullptr;
        // if we're evaluating a global variable
        if (env_.InGlobalVar())
            container = env_.GetGlobalVar();
        else if (ibud_.Container())
            container = ibud_.Container();
        else
            container = transunit_.get();

        bin->Val() = Evaluator::EvalBinary(
            container, bin->op_, bin->left_->Val(), bin->right_->Val());
        if (env_.InGlobalVar())
            env_.GetGlobalVar()->AddOpNode(bin->Val(), 2);
        return;
    }

    // In a global var and the expression has not been folded?
    // Then bin->op_ must be eithor plus or minus
    if (env_.InGlobalVar())
    {
        env_.GetGlobalVar()->MergeNode(
            bin->op_ == Tag::plus ? Instr::InstrId::add : Instr::InstrId::sub);
        return;
    }

    auto lhs = LoadVal(bin->left_.get()), rhs = LoadVal(bin->right_.get());
    auto lhsty = lhs->Type(), rhsty = rhs->Type();
    bool hasfloat = lhsty->Is<FloatType>() || rhsty->Is<FloatType>();

    std::string regname = env_.GetRegName();
    const Register* result = nullptr;

    if (IS_ARITHM(bin->left_) && IS_PTR(bin->right_) && bin->op_ == Tag::plus)
        result = ibud_.InsertGetElePtrInstr(regname, true, AS_REG(bin->right_), bin->left_->Val()); 
    else if (IS_PTR(bin->left_) && IS_ARITHM(bin->right_) && bin->op_ == Tag::plus)
        result = ibud_.InsertGetElePtrInstr(regname, true, AS_REG(bin->left_), bin->right_->Val());
    else if (IS_PTR(bin->left_) && IS_ARITHM(bin->right_) && bin->op_ == Tag::minus)
    {
        // here, rhs mustn't be types other than integer type.
        auto size = lhsty->As<PtrType>()->Point2()->Size();
        const IROperand* newrhs = nullptr;
        if (rhs->Is<IntConst>())
        {
            newrhs = IntConst::CreateIntConst(ibud_.Container(),
                rhs->As<IntConst>()->Val() * size, rhs->Type()->As<IntType>());
        }
        else
        {
            auto sizeconst = IntConst::CreateIntConst(
                ibud_.Container(), size, rhs->Type()->As<IntType>());
            newrhs = ibud_.InsertMulInstr(env_.GetRegName(), rhs, sizeconst);
        }
        result = ibud_.InsertSubInstr(regname, lhs->As<Register>(), newrhs);
    }

    else if (IS_ARITHM(bin->left_) && IS_ARRAY(bin->right_) && bin->op_ == Tag::plus)
    {
        auto inner = bin->right_->Type()->As<CArrayType>()->IsParam();
        result = ibud_.InsertGetElePtrInstr(
            regname, inner, AS_REG(bin->right_), bin->left_->Val());
    }
    else if (IS_ARRAY(bin->left_) && IS_ARITHM(bin->right_) && bin->op_ == Tag::plus)
    {
        auto inner = bin->left_->Type()->As<CArrayType>()->IsParam();
        result = ibud_.InsertGetElePtrInstr(
            regname, inner, AS_REG(bin->left_), bin->right_->Val());
    }

    else if (bin->op_ == Tag::plus && !hasfloat)
        result = ibud_.InsertAddInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::plus)
        result = ibud_.InsertFaddInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::minus && !hasfloat)
    {
        result = ibud_.InsertSubInstr(regname, lhs, rhs);
        if (IS_PTR(bin->left_) && IS_PTR(bin->right_))
        {
            auto sizeptr2 = lhsty->As<PtrType>()->Point2()->Size();
            auto size = IntConst::CreateIntConst(ibud_.Container(), sizeptr2, lhsty->As<PtrType>());
            result = ibud_.InsertDivInstr(env_.GetRegName(), result, size);
        }
    }
    else if (bin->op_ == Tag::minus)
        result = ibud_.InsertFsubInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::asterisk && !hasfloat)
        result = ibud_.InsertMulInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::asterisk)
        result = ibud_.InsertFmulInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::slash && !hasfloat)
        result = ibud_.InsertDivInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::slash)
        result = ibud_.InsertFdivInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::percent)
        result = ibud_.InsertModInstr(regname, lhs, rhs);

    else if (bin->op_ == Tag::lshift)
        result = ibud_.InsertShlInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::rshift && lhsty->As<IntType>()->IsSigned())
        result = ibud_.InsertAshrInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::rshift)
        result = ibud_.InsertLshrInstr(regname, lhs, rhs);   

    else if (bin->op_ == Tag::lessthan)
        result = ibud_.InsertCmpInstr(regname, Condition::lt, lhs, rhs);
    else if (bin->op_ == Tag::lessequal)
        result = ibud_.InsertCmpInstr(regname, Condition::le, lhs, rhs);
    else if (bin->op_ == Tag::greathan)
        result = ibud_.InsertCmpInstr(regname, Condition::gt, lhs, rhs);
    else if (bin->op_ == Tag::greatequal)
        result = ibud_.InsertCmpInstr(regname, Condition::ge, lhs, rhs);
    else if (bin->op_ == Tag::equal)
        result = ibud_.InsertCmpInstr(regname, Condition::eq, lhs, rhs);
    else if (bin->op_ == Tag::notequal)
        result = ibud_.InsertCmpInstr(regname, Condition::ne, lhs, rhs);

    else if (bin->op_ == Tag::_and)
        result = ibud_.InsertAndInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::_xor)
        result = ibud_.InsertXorInstr(regname, lhs, rhs);
    else if (bin->op_ == Tag::_or)
        result = ibud_.InsertOrInstr(regname, lhs, rhs);

    bin->Val() = result;
}

#undef IS_ARITHM
#undef IS_PTR
#undef IS_ARRAY
#undef AS_REG


void IRGen::VisitCallExpr(CallExpr* call)
{
    tbud_.VisitCallExpr(call);

    auto pi = call->Postfix()->ToIdentifier();
    if (pi && pi->Name() == "__Ginkgo_va_start")
    {
        HandleVaStart(call);
        return; // Done; No need of code below
    }
    else if (pi && pi->Name() == "__Ginkgo_va_arg")
    {
        HandleVaArg(call);
        return; // No need of code below as well
    }

    if (call->argvlist_)
    {
        call->argvlist_->Accept(this);
        for (auto& argv : *call->argvlist_)
            LoadVal(argv.get());
    }

    const FuncType* proto = nullptr;

    if (proto = HandleBuiltins(call))
        goto end;

    call->postfix_->Accept(&tbud_);

    if (pi)
    {
        auto name = pi->Name();
        auto func = scopestack_.File().GetFunc(name);

        if (!func)
            goto pointercall;
        proto = transunit_->GetFunction('@' + name)->Type();
        bool isvoid = proto->ReturnType()->Is<VoidType>();
        call->Val() = ibud_.InsertCallInstr(
            isvoid ? "" : env_.GetRegName(), proto, '@' + name);
    }
    else // if a function is called through a pointer
    {
pointercall:
        call->postfix_->Accept(this);
        auto pfunc = LoadVal(call->postfix_.get())->As<Register>();
        proto = pfunc->Type()->As<PtrType>()->Point2()->As<FuncType>();
        bool isvoid = proto->ReturnType()->Is<VoidType>();
        call->Val() = ibud_.InsertCallInstr(
            isvoid ? "" : env_.GetRegName(), pfunc);
    }

end:
    if (call->argvlist_)
    {
        auto callinstr = ibud_.LastInstr()->As<CallInstr>();
        auto argv = call->argvlist_->begin();
        for (auto ty : proto->ParamType())
        {
            auto op = (*argv)->Val();
            if (op->Type()->IsArithm())
                op = ibud_.InsertArithmCastInstr(ty, op);
            callinstr->AddArgv(op);
            ++argv;
        }

        if (proto->Variadic())
            for (; argv != call->ArgvList()->end(); ++argv)
                callinstr->AddArgv((*argv)->Val());
    }
}


void IRGen::VisitCastExpr(CastExpr* cast)
{
    tbud_.VisitCastExpr(cast);
    cast->expr_->Accept(this);

    auto& ty = cast->Type();
    auto& expr = cast->expr_->Type();
    auto expreg = LoadVal(cast->expr_.get());

    auto createint = [this] (const IRType* ty, const IROperand* op) -> const IntConst* {
        return IntConst::CreateIntConst(
            this->ibud_.Container(), op->As<IntConst>()->Val(), ty->As<IntType>());
    };

    if (ty->Is<CPtrType>() && expr->Is<CPtrType>())
    {
        if (expreg->Is<Constant>())
            expreg = createint(ty->ToIRType(ibud_.Container()), expreg);
        else
            expreg = ibud_.InsertBitcastInstr(
                env_.GetRegName(), ty->ToIRType(ibud_.Container()), expreg->As<Register>());
    }
    else if (ty->Is<CPtrType>() && expr->Is<CArithmType>())
    {
        if (expreg->Is<Constant>())
            expreg = createint(ty->ToIRType(ibud_.Container()), expreg);
        else
            expreg = ibud_.InsertItoPtrInstr(env_.GetRegName(),
                ty->ToIRType(ibud_.Container())->As<PtrType>(), expreg->As<Register>());
    }
    else if (ty->Is<CArithmType>() && expr->Is<CPtrType>())
    {
        if (expreg->Is<Constant>())
            expreg = createint(ty->ToIRType(ibud_.Container()), expreg);
        else
            expreg = ibud_.InsertPtrtoIInstr(env_.GetRegName(),
                ty->ToIRType(ibud_.Container())->As<IntType>(), expreg->As<Register>());
    }
    else
    {
        // cast->typename_ is arithmetic type
        // so does the type of cast->expr_
        expreg = ibud_.InsertArithmCastInstr(
            ty->ToIRType(ibud_.Container()), expreg);
    }

    cast->Val() = expreg;
}


void IRGen::VisitCondExpr(CondExpr* cond)
{
    cond->cond_->Accept(this);
    if (cond->cond_->IsConstant())
    {
        auto sel = cond->cond_->Val()->As<Constant>();
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

    auto initblk = ibud_.Container();

    auto trueblk = bbud_.GetBasicBlock(env_.GetLabelName());
    ibud_.SetInsertPoint(trueblk);
    cond->true_->Accept(this);

    auto falseblk = bbud_.GetBasicBlock(env_.GetLabelName());
    ibud_.SetInsertPoint(falseblk);
    cond->false_->Accept(this);

    if (cond->true_->IsConstant() && cond->false_->IsConstant())
    {
        ibud_.SetInsertPoint(initblk);
        cond->Val() = ibud_.InsertSelectInstr(
            env_.GetRegName(), LoadVal(cond->cond_.get()),
            true, cond->true_->Val(), cond->false_->Val());
        // Remove unused basic blocks
        bbud_.PopBack();
        bbud_.PopBack();
        return;
    }

    auto endblk = bbud_.GetBasicBlock(env_.GetLabelName());

    ibud_.SetInsertPoint(initblk);
    ibud_.InsertBrInstr(
        LoadVal(cond->cond_.get()), trueblk, falseblk);

    ibud_.SetInsertPoint(trueblk);
    ibud_.InsertBrInstr(endblk);

    ibud_.SetInsertPoint(falseblk);
    ibud_.InsertBrInstr(endblk);

    ibud_.SetInsertPoint(endblk);
    auto tval = LoadVal(cond->true_.get());
    auto fval = LoadVal(cond->false_.get());

    cond->Val() = ibud_.InsertPhiInstr(
        env_.GetRegName(), tval->Type());
    auto phi = ibud_.LastInstr()->As<PhiInstr>();
    phi->AddBlockValPair(trueblk, tval);
    phi->AddBlockValPair(falseblk, fval);
}


void IRGen::VisitConstant(ConstExpr* constant)
{
    auto ctype = constant->RawType();
    Pool<IROperand>* container = nullptr;
    if (env_.InGlobalVar())
        container = env_.GetGlobalVar();
    else if (ibud_.Container())
        container = ibud_.Container();
    else
        container = transunit_.get();

    if (ctype->As<CArithmType>()->IsInteger())
        constant->Val() = IntConst::CreateIntConst(
            container, constant->GetInt(),
            ctype->ToIRType(transunit_.get())->As<IntType>());
    else
        constant->Val() = FloatConst::CreateFloatConst(
            container, constant->GetFloat(),
            ctype->ToIRType(transunit_.get())->As<FloatType>());

    if (env_.InGlobalVar())
        env_.GetGlobalVar()->AddOpNode(constant->Val());
}


void IRGen::VisitSzAlgnExpr(SzAlgnExpr* expr)
{
    if (expr->ContentAsDecl())
        expr->ContentAsDecl()->Accept(&tbud_);
    else
        expr->ContentAsExpr()->Accept(&tbud_);

    if (expr->IsSizeof())
    {
        auto data = expr->ContentAsDecl() ?
            expr->ContentAsDecl()->Type()->Size() :
            expr->ContentAsExpr()->Type()->Size();
        expr->Val() = IntConst::CreateIntConst(
            ibud_.Container(), data, IntType::GetInt64(false));
    }
    else // if expr->IsAlignof()
    {
        auto data = expr->ContentAsDecl() ?
            expr->ContentAsDecl()->Type()->Align() :
            expr->ContentAsExpr()->Type()->Align();
        expr->Val() = IntConst::CreateIntConst(
            ibud_.Container(), data, IntType::GetInt64(false));
    }
}


void IRGen::VisitEnumConst(EnumConst* enumconst)
{
    // All members of an enum specifier has been evaluated in
    // VisitEnumList and their value is in the scope stack.
    // So if control reaches to this function, value of the enum
    // member can be safely loaded from the scope stack.
    auto mem = scopestack_.SearchMember(enumconst->Name());
    enumconst->Val() = mem->Value();
}


void IRGen::VisitEnumList(EnumList* list)
{
    // open up a temporary scope and place the
    // enum member in it, before we deduce the
    // underlying type of the enum.
    scopestack_.PushNewScope(Scope::ScopeType::block);

    auto first = list->front();
    if (first->ValueExpr())
    {
        first->ValueExpr()->Accept(this);
        first->Val() = first->ValueExpr()->Val();
        tbud_.VisitEnumConst(first);
    }
    else
    {
        first->Val() = IntConst::CreateIntConst(
            transunit_.get(), 0);
        first->Type() = std::make_shared<CArithmType>(TypeTag::int32);
    }

    for (auto i = list->begin() + 1; i != list->end(); ++i)
    {
        if ((*i)->ValueExpr())
        {
            (*i)->ValueExpr()->Accept(this);
            (*i)->Val() = (*i)->ValueExpr()->Val();
        }
        else
        {
            auto pconst = (*(i - 1))->Val()->As<IntConst>();
            (*i)->Val() = IntConst::CreateIntConst(
                transunit_.get(), pconst->Val() + 1);
        }

        // check type of the enum const after we've evaluated it
        tbud_.VisitEnumConst(i->get());
        scopestack_.Top().AddMember(
            (*i)->Name(), nullptr, (*i)->Val()->As<IntConst>());
    }

    // deduce the underlying type of the enum
    tbud_.VisitEnumList(list);
    scopestack_.PopScope();
}


void IRGen::VisitExprList(ExprList* list)
{
    for (auto& expr : list->exprlist_)
        expr->Accept(this);

    // The left operand of a comma operator is evaluated as a void
    // expression; there is a sequence point between its evaluation
    // and that of the right operand. Then the right operand is
    // evaluated; the result has its type and value. (6.5.17[2])
}


void IRGen::VisitIdentExpr(IdentExpr* ident)
{
    tbud_.VisitIdentExpr(ident);

    auto object = scopestack_.SearchObject(ident->name_);
    if (object)
        ident->Addr() = object->Addr();
    else
    {
        auto func = scopestack_.File().GetFunc(ident->name_);
        ident->Addr() = func->Addr();
    }
}


void IRGen::VisitLogicalExpr(LogicalExpr* logical)
{
    logical->left_->Accept(this);
    auto lhs = LoadVal(logical->left_.get());
    const IROperand* rhs = nullptr;

    if (lhs->Is<Constant>())
    {
        auto lconst = lhs->As<Constant>();
        if ((lconst->IsZero() && logical->op_ == Tag::logical_and) ||
            (!lconst->IsZero() && logical->op_ == Tag::logical_or))
        {
            logical->Val() = IntConst::CreateIntConst(
                ibud_.Container(), !lconst->IsZero());
            return;
        }

        logical->right_->Accept(this);
        rhs = LoadVal(logical->right_.get());
        if (rhs->Is<Constant>())
        {
            auto rconst = rhs->As<Constant>();
            logical->Val() = Evaluator::EvalBinary(
                ibud_.Container(), logical->op_, lconst, rconst);
            return;
        }

        auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
        auto one = IntConst::CreateIntConst(ibud_.Container(), 1);

        logical->Val() = ibud_.InsertSelectInstr(
            env_.GetRegName(), rhs, true, one, zero);
        return;
    }

    auto firstblk = ibud_.Container(),
        midblk = bbud_.GetBasicBlock(env_.GetLabelName()),
        finalblk = bbud_.GetBasicBlock(env_.GetLabelName());

    if (logical->op_ == Tag::logical_and)
        ibud_.InsertBrInstr(lhs, midblk, finalblk);
    else if (logical->op_ == Tag::logical_or)
        ibud_.InsertBrInstr(lhs, finalblk, midblk);

    ibud_.SetInsertPoint(midblk);

    logical->right_->Accept(this);
    rhs = LoadVal(logical->right_.get());

    auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
    const Register* cmpans = ibud_.InsertCmpInstr(
        env_.GetRegName(), Condition::ne, rhs, zero);
    ibud_.InsertBrInstr(finalblk);

    ibud_.SetInsertPoint(finalblk);

    auto result = env_.GetRegName();
    if (logical->op_ == Tag::logical_and)
    {
        logical->Val() = ibud_.InsertPhiInstr(
            result, IntType::GetInt8(true));
        auto phi = ibud_.LastInstr()->As<PhiInstr>();
        phi->AddBlockValPair(firstblk, zero);
        phi->AddBlockValPair(midblk, cmpans);
    }
    else if (logical->op_ == Tag::logical_or)
    {
        logical->Val() = ibud_.InsertPhiInstr(
            result, IntType::GetInt8(true));
        auto one = IntConst::CreateIntConst(ibud_.Container(), 1);
        auto phi = ibud_.LastInstr()->As<PhiInstr>();
        phi->AddBlockValPair(firstblk, one);
        phi->AddBlockValPair(midblk, cmpans);
    }
}


template <typename T>
static std::string WashString(const T& str, size_t& count)
{
    std::string result = "";
    constexpr auto size = sizeof(typename T::value_type);
    constexpr auto mask = ((unsigned int)(-1)) >> (32 - size * 8);

    int start = str.find_first_of('"') + 1;
    int end = str.find_last_of('"');

    for (auto i = str.begin() + start; i != str.begin() + end; ++i)
    {
        if (*i == '"')
        {
            i += 1;
            while (*i != '"')
                i += 1;
            continue;
        }
        else if (*i == '\\')
        {
            i += 1;
            count += 1;
            result += '\\';
            if (*i == 'x' || isdigit(*i))
            {
                for (int j = 0; j < 3; ++j, ++i)
                    result += static_cast<typename T::value_type>(*i);
                i = i - 1;
            }
            else if (*i == 'a')
                result += "007";
            else if (*i == 'v')
                result += "013";
            else
                result += static_cast<typename T::value_type>(*i);
            for (int i = 0; i < size - 1; ++i)
                result += "\\x00";
        }
        else if (isascii(*i))
        {
            result += static_cast<typename T::value_type>(*i);
            count += 1;
            for (int i = 0; i < size - 1; ++i)
                result += "\\x00";
        }
        else
        {
            count += 1;
            auto ch = static_cast<unsigned int>((*i) & mask);
            auto pch = reinterpret_cast<unsigned char*>(&ch);
            for (int i = 0; i < size; ++i)
                result += fmt::format("\\x{:02x}", pch[i]);
        }
    }

    count += 1;
    return result;
}

void IRGen::VisitStrExpr(StrExpr* str)
{
    tbud_.VisitStrExpr(str);

    auto content = str->Content();
    std::string literal = "";
    size_t count = 0;

    if (content == "__func__")
    {
        literal = env_.GetFunction()->Name().substr(1);
        count = literal.size();
    }
    else
    {
        if (str->Width() == 1)
            literal = WashString(content, count);
        else if (str->Width() == 2)
            literal = WashString(utf8::utf8to16(content), count);
        else if (str->Width() == 4)
            literal = WashString(utf8::utf8to32(content), count);
    }

    str->Type()->As<CArrayType>()->SetCount(count);
    auto array = str->Type()->ToIRType(transunit_.get())->As<ArrayType>();
    auto global = GlobalVar::CreateGlobalVar(
        transunit_.get(), GetStrName(), array);
    global->AddOpNode(
        StrConst::CreateStrConst(transunit_.get(), literal, array));
    global->Dump2Tree();

    global->Addr() = Register::CreateRegister(
        global, global->Name(), PtrType::GetPtrType(global, array));
    str->Val() = global->Addr();
}


void IRGen::VisitUnaryExpr(UnaryExpr* unary)
{
    unary->content_->Accept(this);
    tbud_.VisitUnaryExpr(unary);

    if (unary->content_->IsConstant())
    {
        Pool<IROperand>* container = nullptr;
        if (env_.InGlobalVar())
            container = env_.GetGlobalVar();
        else if (!ibud_.Container())
            container = transunit_.get();
        else
            container = ibud_.Container();

        unary->Val() = Evaluator::EvalUnary(
            container, unary->op_, unary->content_->Val());

        if (env_.InGlobalVar())
            env_.GetGlobalVar()->AddOpNode(unary->Val(), 1);
        return;
    }

    if (unary->op_ == Tag::inc || unary->op_ == Tag::dec ||
        unary->op_ == Tag::postfix_inc || unary->op_ == Tag::postfix_dec)
    {
        auto addr = LoadAddr(unary->content_.get());
        auto val = LoadVal(unary->content_.get());
        const Register* newval = nullptr;

        if (val->Type()->Is<FloatType>())
        {
            auto one = FloatConst::CreateFloatConst(ibud_.Container(), 1, val->Type()->As<FloatType>());
            if (unary->op_ == Tag::inc || unary->op_ == Tag::postfix_inc)
                newval = ibud_.InsertFaddInstr(env_.GetRegName(), val, one);
            else
                newval = ibud_.InsertFsubInstr(env_.GetRegName(), val, one);
        }
        else
        {
            auto one = IntConst::CreateIntConst(ibud_.Container(), 1, val->Type()->As<IntType>());
            if (unary->op_ == Tag::inc || unary->op_ == Tag::postfix_inc)
                newval = ibud_.InsertAddInstr(env_.GetRegName(), val, one);
            else
                newval = ibud_.InsertSubInstr(env_.GetRegName(), val, one);
        }

        ibud_.InsertStoreInstr(newval, addr, false);
        if (unary->op_ == Tag::postfix_inc || unary->op_ == Tag::postfix_dec)
            unary->Val() = val;
        else unary->Val() = newval;
    }
    else if (unary->op_ == Tag::plus)
        unary->Val() = LoadVal(unary->content_.get());
    else if (unary->op_ == Tag::_and)
    {
        unary->Val() = LoadAddr(unary->content_.get());
        if (env_.InGlobalVar())
            env_.GetGlobalVar()->MergeNode(Instr::InstrId::geteleptr);
    }
    else if (unary->op_ == Tag::asterisk)
    {
        const IROperand* addreg = LoadAddr(unary->content_.get());
        if (addreg->Type()->As<PtrType>()->Point2()->Is<ArrayType>())
        {
            auto inner = !unary->content_->Type()->As<CArrayType>()->IsParam();
            auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
            addreg = ibud_.InsertGetElePtrInstr(
                env_.GetRegName(), inner, addreg->As<Register>(), zero);
        }
        // Not a pointer? That means we are dereferencing a pointer
        // to arithmetic type or heterogeneous type, and we must store
        // the address of the dereferenced value in unary->Val(), that is,
        // the pointer itself.
        else if (!addreg->Type()->As<PtrType>()->Point2()->Is<PtrType>())
            ;
        else
            addreg = LoadVal(unary->content_.get());
        unary->Val() = addreg;
    }
    else if (unary->op_ == Tag::minus)
    {
        auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
        auto rhs = LoadVal(unary->content_.get());
        unary->Val() = ibud_.InsertSubInstr(env_.GetRegName(), zero, rhs);
    }
    else if (unary->op_ == Tag::tilde)
    {
        auto rhs = LoadVal(unary->content_.get());
        auto minusone = IntConst::CreateIntConst(
            ibud_.Container(), ~0ull, rhs->Type()->As<IntType>());
        unary->Val() = ibud_.InsertXorInstr(env_.GetRegName(), minusone, rhs);
    }
    else if (unary->op_ == Tag::exclamation)
    {
        auto zero = IntConst::CreateIntConst(ibud_.Container(), 0);
        auto one = IntConst::CreateIntConst(ibud_.Container(), 1);
        unary->Val() = ibud_.InsertSelectInstr(
            env_.GetRegName(), LoadVal(unary->content_.get()), true, zero, one);
    }
}


void IRGen::VisitBreakStmt(BreakStmt* stmt)
{
    ibud_.InsertBrInstr(nullptr);
    env_.StmtStackTop()->PushBrInstr(ibud_.LastInstr());
}


void IRGen::VisitCaseStmt(CaseStmt* stmt)
{
    if (stmt->const_)
    {
        stmt->const_->Accept(this);
        if (!ibud_.Container()->Empty())
        {
            auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
            if (!ibud_.LastInstr()->IsControlInstr())
                ibud_.InsertBrInstr(bb);
            ibud_.SetInsertPoint(bb);
        }
        env_.SwitchStackTop()->AddValueBlkPair(
            stmt->const_->Val()->As<IntConst>(), ibud_.Container());
    }
    else // default
    {
        auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
        if (ibud_.Container()->Empty() || !ibud_.LastInstr()->IsControlInstr())
            ibud_.InsertBrInstr(bb);
        ibud_.SetInsertPoint(bb);
        env_.SwitchStackTop()->SetDefault(bb);
    }

    if (stmt->stmt_)
        stmt->stmt_->Accept(this);

    // if this is default tag, create a new basic block
    if (!stmt->const_ && !ibud_.Container()->Empty())
    {
        auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
        if (!ibud_.LastInstr()->IsControlInstr())
            ibud_.InsertBrInstr(bb);
        ibud_.SetInsertPoint(bb);
    }
    if (stmt->stmt_)
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
            if (!ibud_.Container()->Empty())
            {
                auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
                if (!ibud_.LastInstr()->IsControlInstr())
                    ibud_.InsertBrInstr(bb);
                ibud_.SetInsertPoint(bb);
            }
            Backpatch(stmt->NextList(), ibud_.Container());
        }
    }
    scopestack_.PopScope();
}


void IRGen::VisitContinueStmt(ContinueStmt* stmt)
{
    IterStmt* iter = static_cast<IterStmt*>(env_.StmtStackTop());
    ibud_.InsertBrInstr(iter->continuepoint_);
}


void IRGen::VisitDeclStmt(DeclStmt* stmt)
{
    stmt->decl_->Accept(this);
}


void IRGen::VisitDoWhileStmt(DoWhileStmt* stmt)
{
    env_.PushStmt(stmt);

    auto loopblk = bbud_.GetBasicBlock(env_.GetLabelName());
    ibud_.InsertBrInstr(loopblk);
    stmt->continuepoint_ = loopblk;

    ibud_.SetInsertPoint(loopblk);
    stmt->stmt_->Accept(this);

    stmt->expr_->Accept(this);
    ibud_.InsertBrInstr(
        LoadVal(stmt->expr_.get()), loopblk, nullptr);

    stmt->PushBrInstr(ibud_.LastInstr());
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
    else if (stmt->decl_)
    {
        scopestack_.PushNewScope(Scope::ScopeType::block);
        stmt->decl_->Accept(this);
    }

    BasicBlock* cmpblk = nullptr;
    BasicBlock* loopblk = nullptr;
    BasicBlock* incblk = nullptr;

    if (stmt->condition_)
    {
        cmpblk = bbud_.GetBasicBlock(env_.GetLabelName());
        loopblk = bbud_.GetBasicBlock(env_.GetLabelName());

        ibud_.InsertBrInstr(cmpblk);
        ibud_.SetInsertPoint(cmpblk);
        stmt->condition_->Accept(this);
        ibud_.InsertBrInstr(
            LoadVal(stmt->condition_.get()), loopblk, nullptr);
        stmt->PushBrInstr(ibud_.LastInstr());
    }
    else
    {
        loopblk = bbud_.GetBasicBlock(env_.GetLabelName());
        ibud_.InsertBrInstr(loopblk);
    }

    if (stmt->increment_)
    {
        incblk = bbud_.GetBasicBlock(env_.GetLabelName());

        ibud_.SetInsertPoint(incblk);
        stmt->increment_->Accept(this);

        if (stmt->condition_)
            ibud_.InsertBrInstr(cmpblk);
        else
            ibud_.InsertBrInstr(loopblk);

        bbud_.SetInsertPoint(incblk);
    }

    ibud_.SetInsertPoint(loopblk);
    if (stmt->increment_)
        stmt->continuepoint_ = incblk;
    else if (stmt->condition_)
        stmt->continuepoint_ = cmpblk;
    else
        stmt->continuepoint_ = loopblk;

    stmt->body_->Accept(this);

    if (stmt->increment_)
    {
        ibud_.InsertBrInstr(incblk);
        bbud_.SetInsertPoint(bbud_.InsertPoint() + 1);
    }
    else if (stmt->condition_)
        ibud_.InsertBrInstr(cmpblk);
    else
        ibud_.InsertBrInstr(loopblk);

    Merge(stmt->body_->NextList(), stmt->nextlist_);

    auto endblk = bbud_.GetBasicBlock(env_.GetLabelName());
    ibud_.SetInsertPoint(endblk);
    Backpatch(stmt->NextList(), endblk);

    if (stmt->decl_)
        scopestack_.PopScope();
    env_.PopStmt();
}


void IRGen::VisitGotoStmt(GotoStmt* stmt)
{
    ibud_.InsertBrInstr(nullptr);
    auto br = ibud_.LastInstr()->As<BrInstr>();
    env_.AddBrLabelPair(br, stmt->ident_);
}


void IRGen::VisitIfStmt(IfStmt* stmt)
{
    BasicBlock* trueblk = bbud_.GetBasicBlock(env_.GetLabelName());
    BasicBlock* falseblk = stmt->false_ ?
        bbud_.GetBasicBlock(env_.GetLabelName()) : nullptr;

    stmt->expr_->Accept(this);
    if (falseblk)
        ibud_.InsertBrInstr(
            LoadVal(stmt->expr_.get()), trueblk, falseblk);
    else
    {
        ibud_.InsertBrInstr(
            LoadVal(stmt->expr_.get()), trueblk, nullptr);
        stmt->PushBrInstr(ibud_.LastInstr());
    }

    ibud_.SetInsertPoint(trueblk);
    stmt->true_->Accept(this);

    if (trueblk->Empty() || !trueblk->LastInstr()->IsControlInstr())
    {
        ibud_.InsertBrInstr(nullptr);
        stmt->PushBrInstr(trueblk->LastInstr());
    }

    if (stmt->false_)
    {
        ibud_.SetInsertPoint(falseblk);
        stmt->false_->Accept(this);
        if (falseblk->Empty() || !falseblk->LastInstr()->IsControlInstr())
        {
            ibud_.InsertBrInstr(nullptr);
            stmt->PushBrInstr(falseblk->LastInstr());
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
    if (!ibud_.Container()->Empty())
    {
        auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
        if (!ibud_.LastInstr()->IsControlInstr())
            ibud_.InsertBrInstr(bb);
        ibud_.SetInsertPoint(bb);   
    }

    env_.AddLabelBlkPair(stmt->label_, ibud_.Container());
    if (stmt->stmt_)
    {
        stmt->stmt_->Accept(this);
        Merge(stmt->stmt_->NextList(), stmt->nextlist_);
    }
}


void IRGen::VisitRetStmt(RetStmt* stmt)
{
    if (stmt->retvalue_)
    {
        stmt->retvalue_->Accept(this);
        auto retreg = env_.GetFunction()->ReturnValue();
        ibud_.InsertStoreInstr(
            LoadVal(stmt->retvalue_.get()), retreg, false);
    }

    ibud_.InsertBrInstr(nullptr);
    env_.AddBrInstr4Ret(ibud_.LastInstr()->As<BrInstr>());
}


void IRGen::VisitSwitchStmt(SwitchStmt* stmt)
{
    stmt->expr_->Accept(this);
    auto ident = LoadVal(stmt->expr_.get());

    ibud_.InsertSwitchInstr(ident);
    auto switchinstr = ibud_.LastInstr()->As<SwitchInstr>();

    env_.PushSwitch(switchinstr);
    env_.PushStmt(stmt);

    stmt->stmt_->Accept(this);
    Merge(stmt->stmt_->NextList(), stmt->nextlist_);

    if (!switchinstr->GetDefault())
    {
        auto bb = bbud_.GetBasicBlock(env_.GetLabelName());
        if (!ibud_.LastInstr()->IsControlInstr())
            ibud_.InsertBrInstr(bb);
        ibud_.SetInsertPoint(bb);
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

    auto cmpblk = bbud_.GetBasicBlock(env_.GetLabelName()),
        loopblk = bbud_.GetBasicBlock(env_.GetLabelName());

    stmt->continuepoint_ = cmpblk;
    ibud_.InsertBrInstr(cmpblk);
    ibud_.SetInsertPoint(cmpblk);

    stmt->expr_->Accept(this);
    ibud_.InsertBrInstr(
        LoadVal(stmt->expr_.get()), loopblk, nullptr);
    stmt->PushBrInstr(ibud_.LastInstr());

    ibud_.SetInsertPoint(loopblk);
    stmt->stmt_->Accept(this);
    ibud_.InsertBrInstr(cmpblk);

    Merge(stmt->stmt_->NextList(), stmt->nextlist_);

    env_.PopStmt();
}
