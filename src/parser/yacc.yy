%{
#include "yacc.hh"
int yylex(yy::parser::value_type* yylval, CheckType& checktype);
%}


%code requires
{
#include <algorithm>
#include <cstdio>
#include <cctype>
#include <map>
#include <memory>
#include <string>
#include <set>
#include <vector>

#include "ast/CType.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "ast/Tag.h"
#include "messages/Error.h"


class CheckType
{
public:
    CheckType() { EnterScope(); }

    bool& WithinScope() { return within_; }
    bool WithinScope() const { return within_; }

    void EnterScope();
    void LeaveScope();
    void AddIdentifier(const std::string& str, int type);
    int operator()(const std::string&);

private:
    bool within_{};
    std::vector<std::map<std::string, int>> scopes_{};
};


}

%require "3.2"
%language "c++"

%define api.value.type variant
%define parse.error detailed
%define parse.trace
%define lr.type ielr

%parse-param { TransUnit& transunit }
// don't want to inherit from yy::parser.
// just create CheckType in the Driver.
%parse-param { CheckType checktype }
%lex-param { CheckType& checktype }

%token	<std::string> IDENTIFIER I_CONSTANT F_CONSTANT
STRING_LITERAL FUNC_NAME TYPEDEF_NAME ENUMERATION_CONSTANT
%token  <Tag> TRUE FALSE NULLPTR
%token	<Tag> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<Tag> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<Tag> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<Tag> XOR_ASSIGN OR_ASSIGN

%token	<Tag> TYPEDEF EXTERN STATIC AUTO REGISTER CONSTEXPR
%token	<Tag> CONST RESTRICT VOLATILE ATOMIC
%token  <Tag> INLINE NORETURN
%token  <Tag> BOOL BITINT CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	<Tag> COMPLEX IMAGINARY DECIMAL32 DECIMAL64 DECIMAL128
%token  <Tag> TYPEOF TYPEOF_UNQUAL
%token	<Tag> STRUCT UNION ENUM ELLIPSIS

%token	<Tag> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	<Tag> ALIGNAS ALIGNOF SIZEOF GENERIC STATIC_ASSERT THREAD_LOCAL
%token  GKVAARG


%type <Tag> unary_operator assignment_operator
%type <std::unique_ptr<EnumConst>> enumerator
%type <std::unique_ptr<EnumList>> enumerator_list
%type <std::unique_ptr<ExprList>> argument_expression_list
%type <std::unique_ptr<Expr>> primary_expression postfix_expression  
unary_expression cast_expression multiplicative_expression
additive_expression shift_expression relational_expression
equality_expression and_expression exclusive_or_expression
inclusive_or_expression logical_and_expression logical_or_expression
conditional_expression assignment_expression expression constant_expression
constant initializer initializer_list braced_initializer

%type <std::unique_ptr<DeclList>> init_declarator_list
%type <std::unique_ptr<HeterList>> member_declarator_list
%type <HeterFields> member_declaration_list
%type <std::unique_ptr<InitDecl>> init_declarator
%type <std::unique_ptr<ParamList>> parameter_type_list parameter_list

%type <std::string> enumeration_constant string
%type <Tag> type_qualifier storage_class_specifier function_specifier struct_or_union
%type <AlignSpec> alignment_specifier
%type <QualType> type_qualifier_list
%type <std::unique_ptr<TypeSpec>> type_specifier enum_specifier
%type <std::unique_ptr<TypeofSpec>> typeof_specifier
%type <std::unique_ptr<HeterSpec>> struct_or_union_specifier
%type <std::unique_ptr<DeclSpec>> declaration_specifiers specifier_qualifier_list type_specifier_qualifier declaration_specifier
%type <std::unique_ptr<PtrDef>> pointer
%type <std::unique_ptr<Declaration>> declarator direct_declarator function_definition
parameter_declaration type_name enum_type_specifier member_declarator member_declaration
direct_abstract_declarator abstract_declarator array_declarator function_declarator
array_abstract_declarator function_abstract_declarator
%type <std::unique_ptr<TransUnit>> translation_unit

%type <std::unique_ptr<DeclStmt>> declaration external_declaration
%type <std::unique_ptr<ExprStmt>> expression_statement
%type <std::unique_ptr<CompoundStmt>> compound_statement block_item_list
%type <std::unique_ptr<Statement>> block_item statement
selection_statement iteration_statement jump_statement labeled_statement
unlabeled_statement primary_block secondary_block label


// much more precedences are added, in order to avoid
// shift/reduce conflicts caused by new enum grammar
// introduced in C23.
// According to C23(6.7.2.2[5]), doing shift when conflict
// is the right choice.
%precedence IDENTIFIER
%precedence ':'

%precedence '{'
%precedence ENUMERATION_CONSTANT
%precedence LOWER_THAN_SPEC
%precedence VOID %precedence BOOL %precedence CHAR
%precedence SHORT %precedence INT %precedence LONG
%precedence SIGNED %precedence UNSIGNED %precedence FLOAT
%precedence DOUBLE %precedence COMPLEX %precedence IMAGINARY
%precedence ENUM %precedence STRUCT %precedence UNION
%precedence CONST %precedence RESTRICT %precedence VOLATILE
%precedence ATOMIC %precedence ALIGNAS %precedence TYPEDEF_NAME 
%precedence BITINT %precedence DECIMAL32 %precedence DECIMAL64
%precedence DECIMAL128 %precedence TYPEOF %precedence TYPEOF_UNQUAL
%precedence '('
%precedence PREC_ATTR_SPEC
%precedence '['

%precedence LOWER_THAN_ELSE
%precedence ELSE

%start translation_unit
%%
primary_expression
	: IDENTIFIER            { $$ = std::make_unique<IdentExpr>($1); }
	| constant              { $$ = std::move($1); }
	| string                { $$ = std::make_unique<StrExpr>($1); }
	| '(' expression ')'    { $$ = std::move($2); }
	| generic_selection     { $$ = nullptr; } // TODO
	;

constant
	: I_CONSTANT
    {
        if ($1[0] == '\'' || $1[0] == 'U' || $1[0] == 'L' ||
            $1.substr(0, 1) == "u" || $1.substr(0, 2) == "u8")
        {
            $$ = std::make_unique<ConstExpr>($1);
            break; // since the block will be copied into a switch body
        }

        $1.erase(std::remove($1.begin(), $1.end(), '\''), $1.end());

        auto index = $1.find_first_of("ulUL");
        std::string suffix = "";
        if (index != std::string::npos)
            suffix = $1.substr(index, $1.length() - index);

        int base = 10;
        if ($1.length() > 1 && $1[0] == '0' && ($1[1] == 'x' || $1[1] == 'X'))
            base = 16;
        else if ($1.length() > 1 && $1[0] == '0' && ($1[1] == 'b' || $1[1] == 'B'))
            base = 2;
        else if ($1.length() > 1 && $1[0] == '0')
            base = 8;

        unsigned long num = 0;
        if (base == 2)
        {
            for (int i = 2; i < $1.length(); i++)
            {
                num <<= 1;
                if ($1[i] == '1')
                    num |= 1;
            }
        }
        else
        {
            num = std::stoull($1.substr(
                0, $1.length() - suffix.length()), nullptr, base);
        }
        $$ = std::make_unique<ConstExpr>(num, base, suffix);
    }
    /* includes character_constant */
	| F_CONSTANT
    {
        if (std::isalpha($1.back()))
            $$ = std::make_unique<ConstExpr>(std::stod($1), $1.back());
        else
            $$ = std::make_unique<ConstExpr>(std::stod($1));
    }
	| ENUMERATION_CONSTANT	/* after it has been defined as such */
    { $$ = std::make_unique<EnumConst>($1); }
	| TRUE
    { $$ = std::make_unique<ConstExpr>(true); }
    | FALSE
    { $$ = std::make_unique<ConstExpr>(false); }
    | NULLPTR // temporary workaround
    { $$ = std::make_unique<ConstExpr>(static_cast<uint64_t>(0)); }
    ;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER
    {
        checktype.AddIdentifier(
            $1, yy::parser::token::ENUMERATION_CONSTANT);
        $$ = $1;
    }
	;

string
	: STRING_LITERAL // fall through
	| FUNC_NAME      { $$ = "__func__"; }
	;

generic_selection
	: GENERIC '(' assignment_expression ',' generic_assoc_list ')'
	;

generic_assoc_list
	: generic_association
	| generic_assoc_list ',' generic_association
	;

generic_association
	: type_name ':' assignment_expression
	| DEFAULT ':' assignment_expression
	;

postfix_expression
	: primary_expression
    { $$ = std::move($1); }
	| postfix_expression '[' expression ']'
    { $$ = std::make_unique<ArrayExpr>(std::move($1), std::move($3)); }
	| postfix_expression '(' ')'
    { $$ = std::make_unique<CallExpr>(std::move($1)); }
	| postfix_expression '(' argument_expression_list ')'
    { $$ = std::make_unique<CallExpr>(std::move($1), std::move($3)); }
    | GKVAARG '(' assignment_expression ',' type_name ')'
    {
        auto list = std::make_unique<ExprList>();
        list->Append(std::move($3));
        $$ = std::make_unique<CallExpr>(
            std::make_unique<IdentExpr>("__Ginkgo_va_arg"),
            std::move(list), std::move($5));
    }

	| postfix_expression '.' IDENTIFIER
    { $$ = std::make_unique<AccessExpr>(std::move($1), Tag::dot, $3); }
	| postfix_expression '.' TYPEDEF_NAME
    { $$ = std::make_unique<AccessExpr>(std::move($1), Tag::dot, $3); }
	| postfix_expression '.' ENUMERATION_CONSTANT
    { $$ = std::make_unique<AccessExpr>(std::move($1), Tag::dot, $3); }

	| postfix_expression PTR_OP IDENTIFIER
    { $$ = std::make_unique<AccessExpr>(std::move($1), $2, $3); }
    | postfix_expression PTR_OP TYPEDEF_NAME
    { $$ = std::make_unique<AccessExpr>(std::move($1), $2, $3); }
    | postfix_expression PTR_OP ENUMERATION_CONSTANT
    { $$ = std::make_unique<AccessExpr>(std::move($1), $2, $3); }

	| postfix_expression INC_OP
    { $$ = std::make_unique<UnaryExpr>(Tag::postfix_inc, std::move($1)); }
	| postfix_expression DEC_OP
    { $$ = std::make_unique<UnaryExpr>(Tag::postfix_dec, std::move($1)); }
    | compound_literal // TODO
    { $$ = nullptr; }
	;

argument_expression_list
	: assignment_expression
    {
        auto exprlist = std::make_unique<ExprList>();
        exprlist->Append(std::move($1));
        $$ = std::move(exprlist);
    }
	| argument_expression_list ',' assignment_expression
    {
        $1->Append(std::move($3));
        $$ = std::move($1);
    }
	;

compound_literal
    : '(' type_name ')' braced_initializer
    | '(' storage_class_specifiers type_name ')' braced_initializer
    ;

storage_class_specifiers
    : storage_class_specifier
    | storage_class_specifiers storage_class_specifier
    ;


unary_expression
	: postfix_expression   { $$ = std::move($1); }
	| INC_OP unary_expression
    {
        if (!$2->IsLVal())
        {
            Error(ErrorId::needlval);
            YYERROR;
        }
        $$ = std::make_unique<UnaryExpr>($1, std::move($2));
    }
	| DEC_OP unary_expression
    {
        if (!$2->IsLVal())
        {
            Error(ErrorId::needlval);
            YYERROR;
        }
        $$ = std::make_unique<UnaryExpr>($1, std::move($2));
    }
	| unary_operator cast_expression
    { $$ = std::make_unique<UnaryExpr>($1, std::move($2)); }
	| SIZEOF unary_expression
    { $$ = std::make_unique<SzAlgnExpr>($1, std::move($2)); }
	| SIZEOF '(' type_name ')'
    { $$ = std::make_unique<SzAlgnExpr>($1, std::move($3)); }
    | ALIGNOF unary_expression
    // This is not standard C23 grammar, but I add it for the sake of
    // symmetry. Similar to sizeof expr, the alignof expr here doesn't
    // actually evaluate the expression, but only check the type of it.
    { $$ = std::make_unique<SzAlgnExpr>($1, std::move($2)); }
	| ALIGNOF '(' type_name ')'
    { $$ = std::make_unique<SzAlgnExpr>($1, std::move($3)); }
	;

unary_operator
	: '&'   { $$ = Tag::_and; }
	| '*'   { $$ = Tag::asterisk; }
	| '+'   { $$ = Tag::plus; }
	| '-'   { $$ = Tag::minus; }
	| '~'   { $$ = Tag::tilde; }
	| '!'   { $$ = Tag::exclamation; }
	;

cast_expression
	: unary_expression                 { $$ = std::move($1); }
	| '(' type_name ')' cast_expression
    { $$ = std::make_unique<CastExpr>(std::move($2), std::move($4)); }
	;

multiplicative_expression
	: cast_expression{ $$ = std::move($1); }
	| multiplicative_expression '*' cast_expression
	{ $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::asterisk, std::move($3)); }
	| multiplicative_expression '/' cast_expression
	{ $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::slash, std::move($3)); }
	| multiplicative_expression '%' cast_expression
	{ $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::percent, std::move($3)); }
	;

additive_expression
	: multiplicative_expression{ $$ = std::move($1); }
	| additive_expression '+' multiplicative_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::plus, std::move($3)); }
	| additive_expression '-' multiplicative_expression
	{ $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::minus, std::move($3)); }
    ;

shift_expression
	: additive_expression{ $$ = std::move($1); }
	| shift_expression LEFT_OP additive_expression
	{ $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::lshift, std::move($3)); }
	| shift_expression RIGHT_OP additive_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::rshift, std::move($3)); }
	;

relational_expression
	: shift_expression { $$ = std::move($1); }
	| relational_expression '<' shift_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::lessthan, std::move($3)); }
	| relational_expression '>' shift_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::greathan, std::move($3)); }
	| relational_expression LE_OP shift_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::lessequal, std::move($3)); }
	| relational_expression GE_OP shift_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::greatequal, std::move($3)); }
	;

equality_expression
	: relational_expression   { $$ = std::move($1); }
	| equality_expression EQ_OP relational_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::equal, std::move($3)); }
	| equality_expression NE_OP relational_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::notequal, std::move($3)); }
	;

and_expression
	: equality_expression  { $$ = std::move($1); }
	| and_expression '&' equality_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::_and, std::move($3)); }
	;

exclusive_or_expression
	: and_expression{ $$ = std::move($1); }
	| exclusive_or_expression '^' and_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::_xor, std::move($3)); }
	;

inclusive_or_expression
	: exclusive_or_expression{ $$ = std::move($1); }
	| inclusive_or_expression '|' exclusive_or_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::_or, std::move($3)); }
	;

logical_and_expression
	: inclusive_or_expression{ $$ = std::move($1); }
	| logical_and_expression AND_OP inclusive_or_expression
    { $$ = std::make_unique<LogicalExpr>(std::move($1), Tag::logical_and, std::move($3)); }
	;

logical_or_expression
	: logical_and_expression{ $$ = std::move($1); }
	| logical_or_expression OR_OP logical_and_expression
    { $$ = std::make_unique<LogicalExpr>(std::move($1), Tag::logical_or, std::move($3)); }
	;

conditional_expression
	: logical_or_expression{ $$ = std::move($1); }
	| logical_or_expression '?' expression ':' conditional_expression
    { $$ = std::make_unique<CondExpr>(std::move($1), std::move($3), std::move($5)); }
	;

assignment_expression
	: conditional_expression{ $$ = std::move($1); }
	| unary_expression assignment_operator assignment_expression
    {
        $$ = std::make_unique<AssignExpr>(
            std::move($1), $2, std::move($3)
        );
    }
	;

assignment_operator
	: '=' { $$ = Tag::assign; }
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression { $$ = std::move($1); }
	| expression ',' assignment_expression
    {
        auto list = dynamic_cast<ExprList*>($1.get());
        if (!list)
        {
            auto el = std::make_unique<ExprList>();
            el->Append(std::move($1));
            el->Append(std::move($3));
            $$ = std::move(el);
        }
        else
        {
            list->Append(std::move($3));
            $$ = std::move($1);
        }
    }
	;

constant_expression
	: conditional_expression 
    // with constraints; 
   { $$ = std::move($1); }
	;

declaration
	: declaration_specifiers ';'
    { $$ = std::make_unique<DeclStmt>(std::move($1)); }
	| declaration_specifiers init_declarator_list ';'
    {
        std::shared_ptr<DeclSpec> ds = std::move($1);
        for (auto& initdecl : *$2)
        {
            auto name = initdecl->declarator_->ToObjDef()->Name();
            if (ds->Storage().IsTypedef())
                checktype.AddIdentifier(
                    name, yy::parser::token::TYPEDEF_NAME);
            else
                checktype.AddIdentifier(
                    name, yy::parser::token::IDENTIFIER);
            initdecl->declarator_->InnerMost()->SetChild(ds);
        }
        $$ = std::make_unique<DeclStmt>(std::move($2));
    }
	| static_assert_declaration { $$ = nullptr; }
	| attribute_declaration { $$ = nullptr; }
    ;

declaration_specifiers
    : declaration_specifier attribute_specifier_sequence %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | declaration_specifier                              %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | declaration_specifier declaration_specifiers
    {
        $2->Extend($1);
        $$ = std::move($2);
    }
    ;

declaration_specifier
	: storage_class_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->SetStorage($1);
    }
	| type_specifier_qualifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->Extend($1);
    }
	| function_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->SetFuncSpec($1);
    }
	;

init_declarator_list
	: init_declarator 
    {
        auto declist = std::make_unique<DeclList>();
        declist->Append(std::move($1));
        $$ = std::move(declist);
    }
	| init_declarator_list ',' init_declarator
    {
        $1->Append(std::move($3));
        $$ = std::move($1);
    }
	;

init_declarator
	: declarator '=' initializer
    {
        auto decl = std::make_unique<InitDecl>();
        decl->declarator_ = std::move($1);
        decl->initalizer_ = std::move($3);
        $$ = std::move(decl);
    }
	| declarator
    {
        auto decl = std::make_unique<InitDecl>();
        decl->declarator_ = std::move($1);
        $$ = std::move(decl);
    }
	;

attribute_declaration
    : attribute_specifier_sequence ';'
    ;

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
    | CONSTEXPR
	;

type_specifier
	: VOID
    { $$ = std::make_unique<TypeSpec>(Tag::_void); }
	| CHAR
    { $$ = std::make_unique<TypeSpec>(Tag::_char); }
	| SHORT
    { $$ = std::make_unique<TypeSpec>(Tag::_short); }
	| INT
    { $$ = std::make_unique<TypeSpec>(Tag::_int); }
	| LONG
    { $$ = std::make_unique<TypeSpec>(Tag::_long); }
	| FLOAT
    { $$ = std::make_unique<TypeSpec>(Tag::_float); }
	| DOUBLE
    { $$ = std::make_unique<TypeSpec>(Tag::_double); }
	| SIGNED
    { $$ = std::make_unique<TypeSpec>(Tag::_signed); }
	| UNSIGNED
    { $$ = std::make_unique<TypeSpec>(Tag::_unsigned); }
	| BITINT '(' constant_expression ')' // TODO
    { $$ = nullptr; }
    | BOOL
    { $$ = std::make_unique<TypeSpec>(Tag::_bool); }
	| COMPLEX
    { $$ = std::make_unique<TypeSpec>(Tag::_complex); }
	| IMAGINARY
    { $$ = std::make_unique<TypeSpec>(Tag::_imaginary); }
	| DECIMAL32 // No plan to support
    { $$ = nullptr; }
    | DECIMAL64 // No plan to support
    { $$ = nullptr; }
    | DECIMAL128 // No plan to support
    { $$ = nullptr; }
    | atomic_type_specifier
    { $$ = std::make_unique<TypeSpec>(Tag::_atomic); }
	| struct_or_union_specifier
    { $$ = std::move($1); }
	| enum_specifier
    { $$ = std::move($1); }
	| TYPEDEF_NAME
    { $$ = std::make_unique<TypedefSpec>(std::move($1)); }
	| typeof_specifier
    { $$ = std::move($1); }
    ;

struct_or_union_specifier
	: struct_or_union '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1);
        $$->LoadHeterFields(std::move($3));
    }
    | struct_or_union attribute_specifier_sequence '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1);
        $$->LoadHeterFields(std::move($4));
    }

	| struct_or_union IDENTIFIER '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $2);
        $$->LoadHeterFields(std::move($4));
    }
	| struct_or_union TYPEDEF_NAME '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $2);
        $$->LoadHeterFields(std::move($4));
    }
    | struct_or_union ENUMERATION_CONSTANT '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $2);
        $$->LoadHeterFields(std::move($4));
    }
	| struct_or_union attribute_specifier_sequence IDENTIFIER '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $3);
        $$->LoadHeterFields(std::move($5));
    }
	| struct_or_union attribute_specifier_sequence TYPEDEF_NAME '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $3);
        $$->LoadHeterFields(std::move($5));
    }
    | struct_or_union attribute_specifier_sequence ENUMERATION_CONSTANT '{' member_declaration_list '}'
    {
        $$ = std::make_unique<HeterSpec>($1, $3);
        $$->LoadHeterFields(std::move($5));
    }

	| struct_or_union IDENTIFIER
    { $$ = std::make_unique<HeterSpec>($1, $2); }
    | struct_or_union TYPEDEF_NAME
    { $$ = std::make_unique<HeterSpec>($1, $2); }
    | struct_or_union ENUMERATION_CONSTANT
    { $$ = std::make_unique<HeterSpec>($1, $2); }
    | struct_or_union attribute_specifier_sequence IDENTIFIER
    { $$ = std::make_unique<HeterSpec>($1, $3); }
    | struct_or_union attribute_specifier_sequence TYPEDEF_NAME
    { $$ = std::make_unique<HeterSpec>($1, $3); }
    | struct_or_union attribute_specifier_sequence ENUMERATION_CONSTANT
    { $$ = std::make_unique<HeterSpec>($1, $3); }
	;

struct_or_union
	: STRUCT    // fall through
	| UNION     // fall through
	;

member_declaration_list
    : member_declaration
    {
        $$ = HeterFields();
        $$.push_back(std::move($1));
    }
    | member_declaration_list member_declaration
    {
        $1.push_back(std::move($2));
        $$ = std::move($1);
    }
    ;

member_declaration
    : specifier_qualifier_list ';' /* for anonymous struct/union */
    { $$ = std::move($1); }
    | specifier_qualifier_list member_declarator_list ';'
    {
        std::shared_ptr<DeclSpec> ds = std::move($1);
        for (auto& decl : *$2)
            decl->InnerMost()->SetChild(ds);
        $$ = std::move($2);
    }
    | attribute_specifier_sequence specifier_qualifier_list ';' /* for anonymous struct/union */
    { $$ = std::move($2); }
    | attribute_specifier_sequence specifier_qualifier_list member_declarator_list ';'
    {
        std::shared_ptr<DeclSpec> ds = std::move($2);
        for (auto& decl : *$3)
            decl->InnerMost()->SetChild(ds);
        $$ = std::move($3);
    }
    | static_assert_declaration { $$ = nullptr; } // TODO
    ;

specifier_qualifier_list
    : type_specifier_qualifier                                  %prec LOWER_THAN_SPEC
    { $$ = std::move($1); }
    /*
    | type_specifier_qualifier attribute_specifier_sequence     %prec LOWER_THAN_SPEC
    { $$ = std::move($1); }
    */
    | type_specifier_qualifier specifier_qualifier_list         %prec LOWER_THAN_SPEC
    {
        $2->Extend($1);
        $$ = std::move($2);
    }
    ;

type_specifier_qualifier
    : type_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->AddTypeSpec(std::move($1));
    }
    | type_qualifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->SetQual($1);
    }
    | alignment_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->AddAlignSpec(std::move($1));
    }
    ;

member_declarator_list
	: member_declarator
    {
        $$ = std::make_unique<HeterList>();
        $$->Append(std::move($1));
    }
	| member_declarator_list ',' member_declarator
    {
        $1->Append(std::move($3));
        $$ = std::move($1);
    }
	;

member_declarator
	: ':' constant_expression
    { $$ = std::make_unique<BitFieldDef>(std::move($2)); }
	| declarator ':' constant_expression
    { $$ = std::make_unique<BitFieldDef>(std::move($1), std::move($3)); }
	| declarator
    { $$ = std::move($1); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>(std::move($3)); }
	| ENUM '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>(std::move($3)); }
	| ENUM attribute_specifier_sequence '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>(std::move($4)); }
	| ENUM attribute_specifier_sequence '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>(std::move($4)); }

	| ENUM IDENTIFIER '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
	| ENUM attribute_specifier_sequence IDENTIFIER '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }
	| ENUM attribute_specifier_sequence IDENTIFIER '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }

    | ENUM TYPEDEF_NAME '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
	| ENUM TYPEDEF_NAME '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
    | ENUM attribute_specifier_sequence TYPEDEF_NAME '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }
	| ENUM attribute_specifier_sequence TYPEDEF_NAME '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }

    | ENUM ENUMERATION_CONSTANT '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
	| ENUM ENUMERATION_CONSTANT '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($4)); }
    | ENUM attribute_specifier_sequence ENUMERATION_CONSTANT '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }
	| ENUM attribute_specifier_sequence ENUMERATION_CONSTANT '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($5)); }

	| ENUM enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>(std::move($4), std::move($2)); }
	| ENUM enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>(std::move($4), std::move($2)); }
	| ENUM attribute_specifier_sequence enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>(std::move($5), std::move($3)); }
	| ENUM attribute_specifier_sequence enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>(std::move($5), std::move($3)); }

	| ENUM IDENTIFIER enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM IDENTIFIER enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM attribute_specifier_sequence IDENTIFIER enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }
	| ENUM attribute_specifier_sequence IDENTIFIER enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }

    | ENUM IDENTIFIER
    { $$ = std::make_unique<EnumSpec>($2); }
    | ENUM IDENTIFIER enum_type_specifier
    { $$ = std::make_unique<EnumSpec>($2, std::move($3)); }

	| ENUM TYPEDEF_NAME enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM TYPEDEF_NAME enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM attribute_specifier_sequence TYPEDEF_NAME enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }
	| ENUM attribute_specifier_sequence TYPEDEF_NAME enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }

    | ENUM TYPEDEF_NAME
    { $$ = std::make_unique<EnumSpec>($2); }
    | ENUM TYPEDEF_NAME enum_type_specifier
    { $$ = std::make_unique<EnumSpec>($2, std::move($3)); }

	| ENUM ENUMERATION_CONSTANT enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM ENUMERATION_CONSTANT enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($2, std::move($5), std::move($3)); }
	| ENUM attribute_specifier_sequence ENUMERATION_CONSTANT enum_type_specifier '{' enumerator_list '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }
	| ENUM attribute_specifier_sequence ENUMERATION_CONSTANT enum_type_specifier '{' enumerator_list ',' '}'
    { $$ = std::make_unique<EnumSpec>($3, std::move($6), std::move($4)); }

    | ENUM ENUMERATION_CONSTANT
    { $$ = std::make_unique<EnumSpec>($2); }
    | ENUM ENUMERATION_CONSTANT enum_type_specifier
    { $$ = std::make_unique<EnumSpec>($2, std::move($3)); }
	;

enumerator_list
	: enumerator
    {
        $$ = std::make_unique<EnumList>();
        $$->Append(std::move($1));
    }
	| enumerator_list ',' enumerator
    {
        $1->Append(std::move($3));
        $$ = std::move($1);
    }
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant '=' constant_expression
    { $$ = std::make_unique<EnumConst>(std::move($1), std::move($3)); }
	| enumeration_constant attribute_specifier_sequence '=' constant_expression
    { $$ = std::make_unique<EnumConst>(std::move($1), std::move($4)); }

    | enumeration_constant
    { $$ = std::make_unique<EnumConst>(std::move($1)); }
	| enumeration_constant attribute_specifier_sequence
    { $$ = std::make_unique<EnumConst>(std::move($1)); }
	;

enum_type_specifier
    : ':' specifier_qualifier_list { $$ = std::move($2); }
    ;

atomic_type_specifier
	: ATOMIC '(' type_name ')'
	;

typeof_specifier
    : TYPEOF '(' expression ')'
    { $$ = std::make_unique<TypeofSpec>($1, std::move($3)); }
    | TYPEOF '(' type_name ')'
    { $$ = std::make_unique<TypeofSpec>($1, std::move($3)); }
    | TYPEOF_UNQUAL '(' expression ')'
    { $$ = std::make_unique<TypeofSpec>($1, std::move($3)); }
    | TYPEOF_UNQUAL '(' type_name ')'
    { $$ = std::make_unique<TypeofSpec>($1, std::move($3)); }
    ;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

function_specifier
	: INLINE    { $$ = Tag::_inline; }
	| NORETURN  { $$ = Tag::_noreturn; }
	;

alignment_specifier
	: ALIGNAS '(' type_name ')'
    { $$ = std::move($3); }
	| ALIGNAS '(' constant_expression ')'
    { $$ = std::move($3); }
	;

declarator
	: pointer direct_declarator
    {
        $2->InnerMost()->SetChild(std::move($1));
        $$ = std::move($2);
    }
	| direct_declarator { $$ = std::move($1); }
	;

direct_declarator
	: IDENTIFIER
    { $$ = std::make_unique<ObjDef>($1); }
    | ENUMERATION_CONSTANT
    { $$ = std::make_unique<ObjDef>($1); }

    /* If the parser is a recursive descent one,
     * parsing these snippets would be super
     * easy. But, unfortunately it's not.

    | IDENTIFIER attribute_declaration
    { $$ = std::make_unique<ObjDef>($1); }
    | ENUMERATION_CONSTANT attribute_declaration
    { $$ = std::make_unique<ObjDef>($1); }
    */

	| '(' declarator ')'
    { $$ = std::move($2); }

    | array_declarator                                  %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    /*
    | array_declarator attribute_specifier_sequence     %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    */

    | function_declarator                               %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | function_declarator attribute_specifier_sequence  %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    ;

array_declarator
	: direct_declarator '[' ']'
    {
        $1->InnerMost()->SetChild(std::make_unique<ArrayDef>());
        $$ = std::move($1);
    }
	| direct_declarator '[' '*' ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Variable() = true;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($5));
        def->Static() = true;
        def->Qual() = $4;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Static() = true;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' type_qualifier_list '*' ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Variable() = true;
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($5));
        def->Static() = true;
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' type_qualifier_list ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_declarator '[' assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($3));
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
    ;

function_declarator
	: direct_declarator '(' parameter_type_list ')'
    {
        auto func = std::make_unique<FuncDef>(std::move($3));
        $1->InnerMost()->SetChild(std::move(func));
        $$ = std::move($1);
    }
	| direct_declarator '(' ')'
    {
        $1->InnerMost()->SetChild(std::make_unique<FuncDef>());
        $$ = std::move($1);
    }
	;

pointer
	: '*' type_qualifier_list pointer
	{ $$ = std::make_unique<PtrDef>($2, std::move($3)); }
	| '*' type_qualifier_list
	{ $$ = std::make_unique<PtrDef>($2); }
	| '*' pointer
    { $$ = std::make_unique<PtrDef>(std::move($2)); }
	| '*'                                                           %prec PREC_ATTR_SPEC
    { $$ = std::make_unique<PtrDef>(); }

    /*
    | '*' attribute_specifier_sequence type_qualifier_list pointer  %prec PREC_ATTR_SPEC
	{ $$ = std::make_unique<PtrDef>($3, std::move($4)); }
	| '*' attribute_specifier_sequence type_qualifier_list          %prec PREC_ATTR_SPEC
	{ $$ = std::make_unique<PtrDef>($3); }
	| '*' attribute_specifier_sequence pointer                      %prec PREC_ATTR_SPEC
    { $$ = std::make_unique<PtrDef>(std::move($3)); }
	| '*' attribute_specifier_sequence                              %prec PREC_ATTR_SPEC
    { $$ = std::make_unique<PtrDef>(); }
    */
	;

type_qualifier_list
	: type_qualifier 
    { $$ = QualType(); $$.SetToken($1); }
	| type_qualifier_list type_qualifier 
    { $1.SetToken($2); $$ = $1; }
	;


parameter_type_list
	: parameter_list ',' ELLIPSIS
    { $1->Variadic() = true; $$ = std::move($1); }
	| parameter_list { $$ = std::move($1); }
    | ELLIPSIS
    {
        $$ = std::make_unique<ParamList>();
        $$->Variadic() = true;
    }
	;

parameter_list
	: parameter_declaration 
    {
        $$ = std::make_unique<ParamList>();
        $$->Append(std::move($1));
    }
	| parameter_list ',' parameter_declaration
    {
        $1->Append(std::move($3));
        $$ = std::move($1);
    }
	;

parameter_declaration
	: declaration_specifiers declarator
    {
        $2->InnerMost()->SetChild(std::move($1));
        $$ = std::move($2);
    }
	| declaration_specifiers abstract_declarator
    {
        $2->InnerMost()->SetChild(std::move($1));
        $$ = std::move($2);
    }
	| declaration_specifiers
    { $$ = std::move($1); }

    | attribute_specifier_sequence declaration_specifiers declarator
    {
        $3->InnerMost()->SetChild(std::move($2));
        $$ = std::move($3);
    }
	| attribute_specifier_sequence declaration_specifiers abstract_declarator
    {
        $3->InnerMost()->SetChild(std::move($2));
        $$ = std::move($3);
    }
	| attribute_specifier_sequence declaration_specifiers
    { $$ = std::move($2); }
	;

type_name
	: specifier_qualifier_list abstract_declarator
    {
        $2->InnerMost()->SetChild(std::move($1));
        $$ = std::move($2);
    }
	| specifier_qualifier_list
    { $$ = std::move($1); }
	;

abstract_declarator
	: pointer direct_abstract_declarator
    {
        $2->InnerMost()->SetChild(std::move($1));
        $$ = std::move($2);
    }
	| pointer
    { $$ = std::move($1); }
	| direct_abstract_declarator
    { $$ = std::move($1); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
    { $$ = std::move($2); }
    | array_abstract_declarator                                 %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | array_abstract_declarator attribute_specifier_sequence    %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | function_abstract_declarator                              %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    | function_abstract_declarator attribute_specifier_sequence %prec PREC_ATTR_SPEC
    { $$ = std::move($1); }
    ;

array_abstract_declarator
	: '[' ']'
    { $$ = std::make_unique<ArrayDef>(); }
	| '[' '*' ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Variable() = true;
        $$ = std::move(def);
    }
	| '[' STATIC type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Static() = true;
        def->Qual() = $3;
        $$ = std::move(def);
    }
	| '[' STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($3));
        def->Static() = true;
        $$ = std::move(def);
    }
	| '[' type_qualifier_list STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Static() = true;
        def->Qual() = $2;
        $$ = std::move(def);
    }
	| '[' type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($3));
        def->Qual() = $2;
        $$ = std::move(def);
    }
	| '[' type_qualifier_list ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Qual() = $2;
        $$ = std::move(def);
    }
	| '[' assignment_expression ']'
    { $$ = std::make_unique<ArrayDef>(std::move($2)); }
	| direct_abstract_declarator '[' ']'
    {
        auto def = std::make_unique<ArrayDef>();
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' '*' ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Variable() = true;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($5));
        def->Static() = true;
        def->Qual() = $4;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Static() = true;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($4));
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($5));
        def->Static() = true;
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' type_qualifier_list ']'
    {
        auto def = std::make_unique<ArrayDef>();
        def->Qual() = $3;
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
	| direct_abstract_declarator '[' assignment_expression ']'
    {
        auto def = std::make_unique<ArrayDef>(std::move($3));
        $1->InnerMost()->SetChild(std::move(def));
        $$ = std::move($1);
    }
    ;

function_abstract_declarator
	: '(' ')'
    { $$ = std::make_unique<FuncDef>(); }
	| '(' parameter_type_list ')'
    { $$ = std::make_unique<FuncDef>(std::move($2)); }
	| direct_abstract_declarator '(' ')'
    {   
        $1->InnerMost()->SetChild(std::make_unique<FuncDef>());
        $$ = std::move($1);
    }
	| direct_abstract_declarator '(' parameter_type_list ')'
    {
        $1->InnerMost()->SetChild(std::make_unique<FuncDef>(std::move($3)));
        $$ = std::move($1);
    }
	;

braced_initializer
    : '{' '}' { $$ = nullptr; }
	| '{' initializer_list '}' { $$ = nullptr; }
	| '{' initializer_list ',' '}' { $$ = nullptr; }
	;

initializer
    : braced_initializer { $$ = nullptr; }
    | assignment_expression { $$ = std::move($1); }
    ;

initializer_list
	: designation initializer { $$ = nullptr; }
	| initializer { $$ = nullptr; }
	| initializer_list ',' designation initializer { $$ = nullptr; }
	| initializer_list ',' initializer { $$ = nullptr; }
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

static_assert_declaration
	: STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
	| STATIC_ASSERT '(' constant_expression ')' ';'
    ;

attribute_specifier_sequence
    : attribute_specifier_sequence attribute_specifier
    | attribute_specifier
    ;

attribute_specifier
    : '[' '[' attribute_list ']' ']'
    | '[' '[' ']' ']'
    ;

attribute_list
    : attribute
    | attribute_list ',' attribute
    ;

attribute
    : attribute_token attribute_argument_clause
    | attribute_token
    ;

attribute_token
    : IDENTIFIER
    | attirbute_prefixed_token
    ;

attirbute_prefixed_token
    : IDENTIFIER ':' ':' IDENTIFIER
    ;

attribute_argument_clause
    : '(' ')'
    | '(' balanced_token_sequence ')'
    ;

balanced_token_sequence
    : balanced_token
    | balanced_token_sequence balanced_token
    ;

balanced_token
    : '(' balanced_token_sequence ')'
    | '[' balanced_token_sequence ']'
    | '{' balanced_token_sequence '}'
    | '(' ')'
    | '[' ']'
    | '{' '}'
    // | any token other than a parenthesis, a bracket, or a brace
    // TODO; maybe we need recursive descent parser to tackle this
    ;

preprocess_instruction
    : '#' IDENTIFIER I_CONSTANT STRING_LITERAL '\n' // #line 123 "123.c"
    | '#' IDENTIFIER I_CONSTANT '\n'                // #line 123
    ;

statement
	: labeled_statement     { $$ = std::move($1); }
    | unlabeled_statement   { $$ = std::move($1); }
    ;

unlabeled_statement
	: expression_statement  { $$ = std::move($1); }
	| primary_block         { $$ = std::move($1); }
	| jump_statement        { $$ = std::move($1); }
	;

primary_block
    : compound_statement    { $$ = std::move($1); }
    | selection_statement   { $$ = std::move($1); }
    | iteration_statement   { $$ = std::move($1); }
    ;

secondary_block
    : statement             { $$ = std::move($1); }
    ;

// Workaround to get desired behavior.
// Otherwise, complex mechanism will be added to CheckType class.
label
	: IDENTIFIER ':'
    { $$ = std::make_unique<LabelStmt>($1); }
    | TYPEDEF_NAME ':'
    { $$ = std::make_unique<LabelStmt>($1); }
    | ENUMERATION_CONSTANT ':'
    { $$ = std::make_unique<LabelStmt>($1); }
	| CASE constant_expression ':'
    { $$ = std::make_unique<CaseStmt>(std::move($2)); }
	| DEFAULT ':'
    { $$ = std::make_unique<CaseStmt>(nullptr); }
	;

labeled_statement
    : label statement
    {
        auto raw = $1.get();
        if (auto l = dynamic_cast<LabelStmt*>(raw); l)
            l->AddStatement(std::move($2));
        else
        {
            auto c = dynamic_cast<CaseStmt*>(raw);
            c->AddStatement(std::move($2));
        }
        $$ = std::move($1);
    }
    ;

compound_statement
	: '{' '}'
    { $$ = std::make_unique<CompoundStmt>(); }
	| '{'  block_item_list '}'
    { $$ = std::move($2); }
	;

block_item_list
	: block_item
    {
        $$ = std::make_unique<CompoundStmt>();
        $$->Append(std::move($1));
    }
	| block_item_list block_item
    {
        $1->Append(std::move($2));
        $$ = std::move($1);
    }
	;

block_item
	: declaration { $$ = std::move($1); }
	| unlabeled_statement { $$ = std::move($1); }
    | label { $$ = std::move($1); }
    | preprocess_instruction { $$ = std::make_unique<ExprStmt>(nullptr); }
	;

expression_statement
	: ';' { $$ = std::make_unique<ExprStmt>(nullptr); }
	| expression ';' { $$ = std::make_unique<ExprStmt>(std::move($1)); }
	;

selection_statement
	: IF '(' expression ')' secondary_block ELSE secondary_block %prec ELSE
    { $$ = std::make_unique<IfStmt>(std::move($3), std::move($5), std::move($7)); }
	| IF '(' expression ')' secondary_block                %prec LOWER_THAN_ELSE
    { $$ = std::make_unique<IfStmt>(std::move($3), std::move($5)); }
	| SWITCH '(' expression ')' secondary_block
    { $$ = std::make_unique<SwitchStmt>(std::move($3), std::move($5)); }
	;

iteration_statement
	: WHILE '(' expression ')' secondary_block
    { $$ = std::make_unique<WhileStmt>(std::move($3), std::move($5)); }
	| DO secondary_block WHILE '(' expression ')' ';'
    { $$ = std::make_unique<DoWhileStmt>(std::move($5), std::move($2)); }

    | FOR '(' ';' ';' ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::unique_ptr<Expr>(nullptr), nullptr, nullptr, std::move($6)); }
    | FOR '(' expression ';' ';' ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::move($3), nullptr, nullptr, std::move($7)); }
    | FOR '(' ';' expression ';' ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::unique_ptr<Expr>(nullptr), std::move($4), nullptr, std::move($7)); }
    | FOR '(' expression ';' expression ';' ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::move($3), std::move($5), nullptr, std::move($8)); }
    | FOR '(' ';' ';' expression ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::unique_ptr<Expr>(nullptr), nullptr, std::move($5), std::move($7)); }
	| FOR '(' expression ';' ';' expression ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::move($3), nullptr, std::move($6), std::move($8)); }
	| FOR '(' ';' expression ';' expression ')' secondary_block
    { $$ = std::make_unique<ForStmt>(std::unique_ptr<Expr>(nullptr), std::move($4), std::move($6), std::move($8)); }
	| FOR '(' expression ';' expression ';' expression ')' secondary_block
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($5), std::move($7), std::move($9)); }

    | FOR '(' declaration ';' ')' secondary_block
	{ $$ = std::make_unique<ForStmt>(std::move($3), nullptr, nullptr, std::move($6)); }
    | FOR '(' declaration expression ';' ')' secondary_block
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), nullptr, std::move($7)); }
    | FOR '(' declaration ';' expression ')' secondary_block
	{ $$ = std::make_unique<ForStmt>(std::move($3), nullptr, std::move($5), std::move($7)); }
    | FOR '(' declaration expression ';' expression ')' secondary_block
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), std::move($6), std::move($8)); }
    ;

// Workaround, as above
jump_statement
	: GOTO IDENTIFIER ';'
    { $$ = std::make_unique<GotoStmt>($2); }
    | GOTO TYPEDEF_NAME ';'
    { $$ = std::make_unique<GotoStmt>($2); }
    | GOTO ENUMERATION_CONSTANT ';'
    { $$ = std::make_unique<GotoStmt>($2); }
	| CONTINUE ';'
    { $$ = std::make_unique<ContinueStmt>(); }
	| BREAK ';'
    { $$ = std::make_unique<BreakStmt>(); }
	| RETURN ';'
    { $$ = std::make_unique<RetStmt>(); }
	| RETURN expression ';'
    { $$ = std::make_unique<RetStmt>(std::move($2)); }
	;

translation_unit
	: external_declaration
    { transunit.AddDecl(std::move($1)); }
    | preprocess_instruction
    { $$ = nullptr; }
	| translation_unit external_declaration
    { transunit.AddDecl(std::move($2)); }
    | translation_unit preprocess_instruction
    { $$ = nullptr; }
	;

external_declaration 
    : function_definition { $$ = std::make_unique<DeclStmt>(std::move($1)); }
	| declaration         { $$ = std::move($1); }
	;

function_definition
	: declaration_specifiers declarator
    {
        // Here we already have a new scope, since to
        // reduce to this rule we must have the '{' read.
        auto& paramlist = $2->Child()->ToFuncDef()->GetParamList();
        for (auto& param : paramlist)
            if (auto p = param->ToObjDef())
                checktype.AddIdentifier(p->Name(), yy::parser::token::IDENTIFIER);
    }
    compound_statement
    {
        $2->InnerMost()->SetChild(std::move($1));
        $2->ToObjDef()->SetCompound(std::move($4));
        $$ = std::move($2);
    }
	;


%%

void yy::parser::error(const std::string& msg)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", msg.c_str());
}


void CheckType::EnterScope()
{
    scopes_.push_back({});
}

void CheckType::LeaveScope()
{
    scopes_.pop_back();
}

void CheckType::AddIdentifier(const std::string& name, int type)
{
    // don't add enum const to current scope, since when
    // a enum const is to be added, the topmost scope is
    // brought by the pair of braces of the enum declaration,
    // and it'll be soon popped out.
    if (type == yy::parser::token::ENUMERATION_CONSTANT)
        (scopes_.end() - 2)->emplace(name, type);
    else
        scopes_.back().emplace(name, type);
}

int CheckType::operator()(const std::string& name)
{
    // pick up the type of the first identifier with the given name
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
        auto result = it->find(name);
        if (result != it->end())
            return result->second;
    }

    // return IDENTIFIER if not found
    // No one cares what it really is.
    return yy::parser::token::IDENTIFIER;
}
