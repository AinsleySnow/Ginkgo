%{
#include <cstdio>
#include <memory>
#include <string>

#include "ast/Expr.h"
#include "ast/ExprList.h"
#include "ast/BinaryExpr.h"
#include "ast/CondExpr.h"
#include "ast/Constant.h"
#include "ast/Identifier.h"
#include "ast/Str.h"
#include "ast/UnaryExpr.h"
#include "messages/Error.h"
#include "types/Type.h"
#include "utils/Scope.h"


int yylex(void);
void yyerror(std::shared_ptr<Scope>, const char*);


#define handle_binary_expr(n0, n1, n2, sym, tag)\
if (n1->WhichNode() == Expr::Which::constant && \
    n2->WhichNode() == Expr::Which::constant)   \
{                                               \
    Constant ans;                               \
    bool correct = Constant::DoCalc(            \
    std::dynamic_pointer_cast<Constant>(n1),    \
    tag,                                        \
    std::dynamic_pointer_cast<Constant>(n2),    \
    ans                                         \
    );                                          \
    if (correct)                                \
        n0 = std::make_shared<Constant>(ans);   \
    else                                        \
    {                                           \
        Error(ErrorId::operatormisuse, #sym,    \
            n1->GetType()->ToString().c_str(),  \
            n2->GetType()->ToString().c_str()); \
            YYERROR;                            \
        }                                       \
    }                                           \
    else                                        \
    {                                           \
        n0 = std::make_shared<BinaryExpr>(      \
		    n1, tag, n2                         \
		);                                      \
    }
%}

%require "3.2"
%language "c++"

%define api.value.type variant

%parse-param { std::shared_ptr<Scope> tree }

%token	<std::string> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	<Tag> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<Tag> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<Tag> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<Tag> XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	<Tag> CONST RESTRICT VOLATILE
%token  <Tag> BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL


%type <std::shared_ptr<Expr>> primary_expression postfix_expression argument_expression_list 
unary_expression cast_expression multiplicative_expression
additive_expression shift_expression relational_expression
equality_expression and_expression exclusive_or_expression
inclusive_or_expression logical_and_expression logical_or_expression
conditional_expression assignment_expression expression constant_expression
%type <std::shared_ptr<Expr>> constant
%type <std::shared_ptr<Expr>> initializer init_declarator init_declarator_list initializer_list
%type <std::shared_ptr<Expr>> declaration
%type <std::shared_ptr<Identifier>> declarator direct_declarator 
%type <std::string> enumerator_list string
%type <Tag> unary_operator assignment_operator
%type <Tag> type_specifier type_qualifier storage_class_specifier
%type <std::shared_ptr<Type>> declaration_specifiers


%start translation_unit
%%
primary_expression
	: IDENTIFIER            { $$ = std::make_shared<Identifier>($1); }
	| constant              // fall through
	| string                { $$ = std::make_shared<Str>($1); }
	| '(' expression ')'    { $$ = $2; }
	| generic_selection     // TODO
	;

constant
	: I_CONSTANT 
    {
        $$ = std::make_shared<Constant>(
            static_cast<uint64_t>(std::stoll($1.c_str()))
        );
    }
    /* includes character_constant */
	| F_CONSTANT
    { $$ = std::make_shared<Constant>(std::stod($1)); }
	| ENUMERATION_CONSTANT	/* after it has been defined as such */
	;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER
	;

string
	: STRING_LITERAL
	| FUNC_NAME
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
	: primary_expression                                    // fall through
	| postfix_expression '[' expression ']'                 // TODO
	| postfix_expression '(' ')'                            // TODO
	| postfix_expression '(' argument_expression_list ')'   // TODO
	| postfix_expression '.' IDENTIFIER                     // TODO
	| postfix_expression PTR_OP IDENTIFIER                  // TODO
	| postfix_expression INC_OP                             // TODO
	| postfix_expression DEC_OP                             // TODO
	| '(' type_name ')' '{' initializer_list '}'            // TODO
	| '(' type_name ')' '{' initializer_list ',' '}'        // TODO
	;

argument_expression_list
	: assignment_expression
    {
        auto exprlist = std::make_shared<ExprList>();
        exprlist->Append($1);
        $$ = exprlist;
    }
	| argument_expression_list ',' assignment_expression
    {
        std::dynamic_pointer_cast<ExprList>($1)->Append($3);
        $$ = $1;
    }
	;

unary_expression
	: postfix_expression    // fall through
	| INC_OP unary_expression
    {
        if (!$2->IsLVal())
        {
            Error(ErrorId::needlval);
            YYERROR;
        }
        $$ = std::make_shared<UnaryExpr>($1, $2);
    }
	| DEC_OP unary_expression
    {
        if (!$2->IsLVal())
        {
            Error(ErrorId::needlval);
            YYERROR;
        }
        $$ = std::make_shared<UnaryExpr>($1, $2);
    }
	| unary_operator cast_expression
    {
        if (!$2->IsLVal())
        {
            Error(ErrorId::needlval);
            YYERROR;
        }
        $$ = std::make_shared<UnaryExpr>($1, $2);
    }
	| SIZEOF unary_expression
    {
        if (!$2->GetType()->IsComplete())
        {
            Error(ErrorId::uncompletetype);
            YYERROR;
        }
        $$ = std::make_shared<Constant>($2->GetType()->ToArithm()->GetSize());
    }
	| SIZEOF '(' type_name ')' // TODO
	| ALIGNOF '(' type_name ')' // TODO
    | error ';'
	;

unary_operator
	: '&'   { $$ = Tag::_and; }
	| '*'   { $$ = Tag::star; }
	| '+'   { $$ = Tag::plus; }
	| '-'   { $$ = Tag::minus; }
	| '~'   { $$ = Tag::tilde; }
	| '!'   { $$ = Tag::exclamation; }
	;

cast_expression
	: unary_expression                  // fall through
	| '(' type_name ')' cast_expression // TODO
	;

multiplicative_expression
	: cast_expression // fall through
	| multiplicative_expression '*' cast_expression
	{ handle_binary_expr($$, $1, $3, *, Tag::star); }
	| multiplicative_expression '/' cast_expression
	{ handle_binary_expr($$, $1, $3, /, Tag::divide); }
	| multiplicative_expression '%' cast_expression
	{ handle_binary_expr($$, $1, $3, %, Tag::percent); }
    | error ';'
	;

additive_expression
	: multiplicative_expression // fall through
	| additive_expression '+' multiplicative_expression
    { handle_binary_expr($$, $1, $3, +, Tag::plus); }
	| additive_expression '-' multiplicative_expression
	{ handle_binary_expr($$, $1, $3, -, Tag::minus); }
    | error ';'
    ;

shift_expression
	: additive_expression // fall through
	| shift_expression LEFT_OP additive_expression
	{ handle_binary_expr($$, $1, $3, <<, $2); }
	| shift_expression RIGHT_OP additive_expression
    { handle_binary_expr($$, $1, $3, >>, $3); }
    | error ';'
	;

relational_expression
	: shift_expression  // fall through
	| relational_expression '<' shift_expression
    { handle_binary_expr($$, $1, $3, <, Tag::lessthan); }
	| relational_expression '>' shift_expression
    { handle_binary_expr($$, $1, $3, >, Tag::greathan); }
	| relational_expression LE_OP shift_expression
    { handle_binary_expr($$, $1, $3, <=, $2); }
	| relational_expression GE_OP shift_expression
    { handle_binary_expr($$, $1, $3, >=, $2); }
    | error ';'
	;

equality_expression
	: relational_expression    // fall through
	| equality_expression EQ_OP relational_expression
    { handle_binary_expr($$, $1, $3, ==, $2); }
	| equality_expression NE_OP relational_expression
    { handle_binary_expr($$, $1, $3, !=, $2); }
    | error ';'
	;

and_expression
	: equality_expression   // fall through
	| and_expression '&' equality_expression
    { handle_binary_expr($$, $1, $3, &, Tag::_and); }
    | error ';'
	;

exclusive_or_expression
	: and_expression // fall through
	| exclusive_or_expression '^' and_expression
    { handle_binary_expr($$, $1, $3, ^, Tag::_xor); }
    | error ';'
	;

inclusive_or_expression
	: exclusive_or_expression // fall through
	| inclusive_or_expression '|' exclusive_or_expression
    { handle_binary_expr($$, $1, $3, |, Tag::star); }
    | error ';'
	;

logical_and_expression
	: inclusive_or_expression // fall through
	| logical_and_expression AND_OP inclusive_or_expression
    { handle_binary_expr($$, $1, $3, &&, $2); }
    | error ';'
	;

logical_or_expression
	: logical_and_expression // fall through
	| logical_or_expression OR_OP logical_and_expression
    { handle_binary_expr($$, $1, $3, ||, $2); }
    | error ';'
	;

conditional_expression
	: logical_or_expression // fall through
	| logical_or_expression '?' expression ':' conditional_expression
    | error ';'
	;

assignment_expression
	: conditional_expression // fall through
	| unary_expression assignment_operator assignment_expression
    {
        $$ = new AssignExpr(
            std::shared_ptr<Expr>($1),
            $2,
            std::shared_ptr<Expr>($3)
        );
    }
    | error ';'
	;

assignment_operator
	: '=' { $$ = Tag::equal; }
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
	: assignment_expression
    {
        auto exprlist = std::make_shared<ExprList>();
        exprlist->Append($1);
        $$ = exprlist;
    }
	| expression ',' assignment_expression
    {
        std::dynamic_pointer_cast<ExprList>($1)->Append($3);
        $$ = $1;
    }
    | error ';'
	;

constant_expression
	: conditional_expression 
    // with constraints; 
    // fall through
	;

declaration
	: declaration_specifiers ';' // fall through
	| declaration_specifiers init_declarator_list ';'
    {
        for (auto ident : $2)
            ident->SetType(std::shared_ptr<Type>($1));
        $$ = $2;
    }
	| static_assert_declaration { $$ = nullptr; }
    | error ';'
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
    {
        if ($2->GetStorage().SetToken($1))
            $$ = $2;
        else
        {
            Error(ErrorId::multistorage);
            YYERROR;
        }
    }
	| storage_class_specifier
    {
        $$ = new Type();
        $$->GetStorage().SetToken($1);
    }
	| type_specifier declaration_specifiers
    {
        ArithmType* arithm = $2->ToArithm();
        if (!arithm)
        {
            auto temp = std::make_shared<ArithmType>($2);
            temp->SetToken($1);
            $$ = temp;
        }
        else
        {
            arithm->SetToken($1);
            $$ = $2;
        }
    }
	| type_specifier
    { $$ = std::make_shared<ArithmType>($1); }
	| type_qualifier declaration_specifiers
    {
        $2->GetQual().SetToken($1);
        $$ = $2;
    }
	| type_qualifier
    {
        $$ = std::make_shared<Type>();
        $$->GetQual().SetToken($1);
    }
	| function_specifier declaration_specifiers
	| function_specifier
	| alignment_specifier declaration_specifiers
	| alignment_specifier
    | error ';'
	;

init_declarator_list
	: init_declarator 
    {
        auto exprlist = std::make_shared<ExprList>();
        exprlist->Append($1);
        $$ = exprlist;
    }
	| init_declarator_list ',' init_declarator
    {
        $1->Append($3);
        $$ = $1;
    }
    | error ';'
	;

init_declarator
	: declarator '=' initializer
    {
        if ($1->GetIdentType() == Identifier::IdentType::obj)
        {
            $1->SetValExpr(std::shared_ptr<Expr>($3));
            $1->SetType($3->GetType());
            $$ = $1;
        }
    }
	| declarator // fall through
	;

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID 
	| CHAR 
	| SHORT 
	| INT 
	| LONG 
	| FLOAT 
	| DOUBLE
	| SIGNED 
	| UNSIGNED 
	| BOOL 
	| COMPLEX
	| IMAGINARY	  	/* non-mandated extension */
	| atomic_type_specifier
	| struct_or_union_specifier
	| enum_specifier
	| TYPEDEF_NAME		/* after it has been defined as such */
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list ';'	/* for anonymous struct/union */
	| specifier_qualifier_list struct_declarator_list ';'
	| static_assert_declaration
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: ':' constant_expression
	| declarator ':' constant_expression
	| declarator
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant '=' constant_expression
	| enumeration_constant
	;

atomic_type_specifier
	: ATOMIC '(' type_name ')'
	;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

function_specifier
	: INLINE
	| NORETURN
	;

alignment_specifier
	: ALIGNAS '(' type_name ')'
	| ALIGNAS '(' constant_expression ')'
	;

declarator
	: pointer direct_declarator // TODO
	| direct_declarator
    {
        if ($1->GetIdentType() == Identifier::IdentType::undef)
            $$ = new Object($1);
        else
            $$ = $1;
    }
	;

direct_declarator
	: IDENTIFIER
    { $$ = std::make_shared<Identifier>($1); }
	| '(' declarator ')'
	| direct_declarator '[' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' ')'
	| direct_declarator '(' identifier_list ')'
	;

pointer
	: '*' type_qualifier_list pointer
	| '*' type_qualifier_list
	| '*' pointer
	| '*'
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list ',' ELLIPSIS
	| parameter_list
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list abstract_declarator
	| specifier_qualifier_list
	;

abstract_declarator
	: pointer direct_abstract_declarator
	| pointer
	| direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' '*' ']'
	| '[' STATIC type_qualifier_list assignment_expression ']'
	| '[' STATIC assignment_expression ']'
	| '[' type_qualifier_list STATIC assignment_expression ']'
	| '[' type_qualifier_list assignment_expression ']'
	| '[' type_qualifier_list ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' '*' ']'
	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	| assignment_expression // fall through
	;

initializer_list
	: designation initializer
	| initializer
	| initializer_list ',' designation initializer
	| initializer_list ',' initializer
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
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{'  block_item_list '}'
	;

block_item_list
	: block_item
	| block_item_list block_item
	;

block_item
	: declaration // do nothing here
	| statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement
	| IF '(' expression ')' statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration 
    {
        for (auto ident : *($1->ToExprList()))
            tree->AddIdentifier(ident);
    }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
	| declaration_specifiers declarator compound_statement
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%

void yyerror(SymbolTable& globalSymbols, const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}
