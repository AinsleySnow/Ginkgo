%{
#include "yacc.hh"
int yylex(yy::parser::value_type* yylval);
%}


%code requires
{
    #include <cstdio>
    #include <memory>
    #include <string>
    #include <vector>

    #include "ast/CType.h"
    #include "ast/Declaration.h"
    #include "ast/Expression.h"
    #include "ast/Statement.h"
    #include "ast/Tag.h"
    #include "messages/Error.h"
}

%require "3.2"
%language "c++"

%define api.value.type variant
// %define parser.error custom
%define parse.trace

%parse-param { std::vector<std::unique_ptr<DeclStmt>>& transunits }

%token	<std::string> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL
FUNC_NAME SIZEOF TYPEDEF_NAME
%token	<Tag> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<Tag> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<Tag> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<Tag> XOR_ASSIGN OR_ASSIGN
%token	ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	<Tag> CONST RESTRICT VOLATILE ATOMIC
%token  <Tag> BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%type <std::unique_ptr<ExprList>> argument_expression_list
%type <std::unique_ptr<Expr>> primary_expression postfix_expression  
unary_expression cast_expression multiplicative_expression
additive_expression shift_expression relational_expression
equality_expression and_expression exclusive_or_expression
inclusive_or_expression logical_and_expression logical_or_expression
conditional_expression assignment_expression expression constant_expression
constant initializer initializer_list

%type <std::unique_ptr<DeclList>> init_declarator_list
%type <std::unique_ptr<InitDecl>> init_declarator
%type <std::unique_ptr<ParamList>> parameter_type_list parameter_list

%type <std::string> enumerator_list string
%type <Tag> unary_operator assignment_operator
%type <std::unique_ptr<TypeSpec>> type_specifier
%type <Tag> type_qualifier storage_class_specifier
%type <std::unique_ptr<DeclSpec>> declaration_specifiers
%type <std::unique_ptr<Ptr>> pointer
%type <std::unique_ptr<Declaration>> declarator direct_declarator
function_definition
parameter_declaration translation_unit type_name

%type <std::unique_ptr<DeclStmt>> declaration external_declaration
%type <std::unique_ptr<ExprStmt>> expression_statement
%type <std::unique_ptr<CompoundStmt>> compound_statement
%type <std::unique_ptr<Statement>> block_item statement
selection_statement iteration_statement jump_statement labeled_statement
%type <QualType> type_qualifier_list

%precedence LOWER_THAN_ELSE
%precedence ELSE

%start translation_unit
%%
primary_expression
	: IDENTIFIER            { $$ = std::make_unique<IdentExpr>($1); }
	| constant              { $$ = std::move($1); }
	| string                { $$ = std::make_unique<StrExpr>($1); }
	| '(' expression ')'    { $$ = std::move($2); }
	| generic_selection     // TODO
	;

constant
	: I_CONSTANT 
    {
        $$ = std::make_unique<ConstExpr>(
            static_cast<uint64_t>(std::stoll($1.c_str()))
        );
    }
    /* includes character_constant */
	| F_CONSTANT
    { $$ = std::make_unique<ConstExpr>(std::stod($1)); }
	| ENUMERATION_CONSTANT	/* after it has been defined as such */
	;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER
	;

string
	: STRING_LITERAL // fall through
	| FUNC_NAME      // fall through
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
	| postfix_expression '[' expression ']' { $$ = nullptr; }
	| postfix_expression '(' ')'
    { $$ = std::make_unique<CallExpr>(std::move($1)); }
	| postfix_expression '(' argument_expression_list ')'
    { $$ = std::make_unique<CallExpr>(std::move($1), std::move($3)); }
	| postfix_expression '.' IDENTIFIER { $$ = nullptr; }
	| postfix_expression PTR_OP IDENTIFIER { $$ = nullptr; }
	| postfix_expression INC_OP { $$ = nullptr; }
	| postfix_expression DEC_OP { $$ = nullptr; }
	| '(' type_name ')' '{' initializer_list '}' { $$ = nullptr; }
	| '(' type_name ')' '{' initializer_list ',' '}' { $$ = nullptr; }
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
        dynamic_cast<ExprList*>($1.get())->Append(std::move($3));
        $$ = std::move($1);
    }
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
    {
        if ($1 == Tag::_and || $1 == Tag::asterisk)
        {
            if (!$2->IsLVal())
            {
                Error(ErrorId::needlval);
                YYERROR;
            }
            $$ = std::make_unique<UnaryExpr>($1, std::move($2));
        }
        else
            $$ = std::make_unique<UnaryExpr>($1, std::move($2));
    }
	| SIZEOF unary_expression
    {
        if (!$2->Type()->IsComplete())
        {
            Error(ErrorId::uncompletetype);
            YYERROR;
        }
        $$ = std::make_unique<ConstExpr>((uint64_t)8);
    }
	| SIZEOF '(' type_name ')' // TODO
	| ALIGNOF '(' type_name ')' // TODO
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
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::cap, std::move($3)); }
	;

inclusive_or_expression
	: exclusive_or_expression{ $$ = std::move($1); }
	| inclusive_or_expression '|' exclusive_or_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::incl_or, std::move($3)); }
	;

logical_and_expression
	: inclusive_or_expression{ $$ = std::move($1); }
	| logical_and_expression AND_OP inclusive_or_expression
    { $$ = std::make_unique<BinaryExpr>(std::move($1), Tag::and_assign, std::move($3)); }
	;

logical_or_expression
	: logical_and_expression{ $$ = std::move($1); }
	| logical_or_expression OR_OP logical_and_expression
    { $$ = std::make_unique<LogicalExpr>(std::move($1), Tag::or_assign, std::move($3)); }
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
	: assignment_expression{ $$ = std::move($1); }
	| expression ',' assignment_expression
    {
        auto assign = dynamic_cast<AssignExpr*>($1.get());
        if (assign)
        {
            auto list = std::make_unique<ExprList>();
            list->Append(std::move($3));
            list->Append(std::move($1));
            $$ = std::move(list);
        }
        else
        {
            auto list = static_cast<ExprList*>($1.get());
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
	: declaration_specifiers ';' { $$ = nullptr; }
	| declaration_specifiers init_declarator_list ';'
    {
        $2->SetDeclSpec(std::move($1));
        $$ = std::make_unique<DeclStmt>(std::move($2));
    }
	| static_assert_declaration { $$ = nullptr; }
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
    {
        $2->SetStorage($1);
        $$ = std::move($2);
    }
	| storage_class_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->SetStorage($1);
    }
	| type_specifier declaration_specifiers
    {
        $2->AddTypeSpec(std::move($1));
        $$ = std::move($2);
    }
	| type_specifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->AddTypeSpec(std::move($1));
    }
	| type_qualifier declaration_specifiers
    {
        $2->SetQual($1);
        $$ = std::move($2);
    }
	| type_qualifier
    {
        $$ = std::make_unique<DeclSpec>();
        $$->SetQual($1);
    }
	| function_specifier declaration_specifiers
	| function_specifier
	| alignment_specifier declaration_specifiers
	| alignment_specifier
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

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID                      { $$ = std::make_unique<TypeSpec>(Tag::_void); }
	| CHAR                      { $$ = std::make_unique<TypeSpec>(Tag::_char); }
	| SHORT                     { $$ = std::make_unique<TypeSpec>(Tag::_short); }
	| INT                       { $$ = std::make_unique<TypeSpec>(Tag::_int); }
	| LONG                      { $$ = std::make_unique<TypeSpec>(Tag::_long); }
	| FLOAT                     { $$ = std::make_unique<TypeSpec>(Tag::_float); }
	| DOUBLE                    { $$ = std::make_unique<TypeSpec>(Tag::_double); }
	| SIGNED                    { $$ = std::make_unique<TypeSpec>(Tag::_signed); }
	| UNSIGNED                  { $$ = std::make_unique<TypeSpec>(Tag::_unsigned); }
	| BOOL                      { $$ = std::make_unique<TypeSpec>(Tag::_bool); }
	| COMPLEX                   { $$ = std::make_unique<TypeSpec>(Tag::_complex); }
	| IMAGINARY                 { $$ = std::make_unique<TypeSpec>(Tag::_imaginary); }
	| atomic_type_specifier     { $$ = std::make_unique<TypeSpec>(Tag::atomictype); }
	| struct_or_union_specifier { $$ = std::make_unique<TypeSpec>(Tag::customedtype); }
	| enum_specifier            { $$ = std::make_unique<TypeSpec>(Tag::enumtype); }
	| TYPEDEF_NAME              { $$ = std::make_unique<TypeSpec>(Tag::typedefedtype); }
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
	: pointer direct_declarator
    {
        $2->GetPtr() = std::move($1);
        $$ = std::move($2);
    }
	| direct_declarator{ $$ = std::move($1); }
	;

direct_declarator
	: IDENTIFIER
    { $$ = std::make_unique<ObjDef>($1); }
	| '(' declarator ')' { $$ = nullptr; }
	| direct_declarator '[' ']' { $$ = nullptr; }
	| direct_declarator '[' '*' ']' { $$ = nullptr; }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']' { $$ = nullptr; }
	| direct_declarator '[' STATIC assignment_expression ']' { $$ = nullptr; }
	| direct_declarator '[' type_qualifier_list '*' ']' { $$ = nullptr; }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']' { $$ = nullptr; }
	| direct_declarator '[' type_qualifier_list assignment_expression ']' { $$ = nullptr; }
	| direct_declarator '[' type_qualifier_list ']' { $$ = nullptr; }
	| direct_declarator '[' assignment_expression ']' { $$ = nullptr; }
	| direct_declarator '(' parameter_type_list ')'
    { $$ = std::make_unique<FuncDef>($1->Name(), std::move($3)); }
	| direct_declarator '(' ')'
    { $$ = std::make_unique<FuncDef>($1->Name()); }
	| direct_declarator '(' identifier_list ')' { $$ = nullptr; }
	;

pointer
	: '*' type_qualifier_list pointer
	{ $$ = std::make_unique<Ptr>($2, std::move($3)); }
	| '*' type_qualifier_list
	{ $$ = std::make_unique<Ptr>($2); }
	| '*' pointer { $$ = std::make_unique<Ptr>(std::move($2)); }
	| '*' { $$ = std::make_unique<Ptr>(); }
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
    { $2->SetDeclSpec(std::move($1)); $$ = std::move($2); }
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
    { 
        $$ = std::make_unique<ObjDef>();
        $$->SetDeclSpec(std::move($1));
    }
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
	| assignment_expression{ $$ = std::move($1); }
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
	;

statement
	: labeled_statement{ $$ = std::move($1); }
	| compound_statement{ $$ = std::move($1); }
	| expression_statement{ $$ = std::move($1); }
	| selection_statement{ $$ = std::move($1); }
	| iteration_statement{ $$ = std::move($1); }
	| jump_statement{ $$ = std::move($1); }
	;

labeled_statement
	: IDENTIFIER ':' statement
    { $$ = std::make_unique<LabelStmt>($1, std::move($3)); }
	| CASE constant_expression ':' statement
    { $$ = std::make_unique<CaseStmt>(std::move($2), std::move($4)); }
	| DEFAULT ':' statement
    { $$ = std::make_unique<CaseStmt>(nullptr, std::move($3)); }
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
	: declaration { $$ = std::move($1); }
	| statement { $$ = std::move($1); }
	;

expression_statement
	: ';' { $$ = nullptr; }
	| expression ';' { $$ = std::make_unique<ExprStmt>(std::move($1)); }
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement %prec ELSE
    { $$ = std::make_unique<IfStmt>(std::move($3), std::move($5), std::move($7)); }
	| IF '(' expression ')' statement                %prec LOWER_THAN_ELSE
    { $$ = std::make_unique<IfStmt>(std::move($3), std::move($5)); }
	| SWITCH '(' expression ')' statement
    { $$ = std::make_unique<SwitchStmt>(std::move($3), std::move($5)); }
	;

iteration_statement
	: WHILE '(' expression ')' statement
    { $$ = std::make_unique<WhileStmt>(std::move($3), std::move($5)); }
	| DO statement WHILE '(' expression ')' ';'
    { $$ = std::make_unique<DoWhileStmt>(std::move($5), std::move($2)); }
	| FOR '(' expression_statement expression_statement ')' statement
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), std::move($6)); }
    | FOR '(' expression_statement expression_statement expression ')' statement
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), std::move($5), std::move($7)); }
    | FOR '(' declaration expression_statement ')' statement
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), std::move($6)); }
    | FOR '(' declaration expression_statement expression ')' statement
	{ $$ = std::make_unique<ForStmt>(std::move($3), std::move($4), std::move($5), std::move($7)); }
    ;

jump_statement
	: GOTO IDENTIFIER ';'
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
    { transunits.push_back(std::move($1)); }
	| translation_unit external_declaration
    { transunits.push_back(std::move($2)); }
	;

external_declaration 
    : function_definition { $$ = std::make_unique<DeclStmt>(std::move($1)); }
	| declaration         { $$ = std::move($1); }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement { $$ = nullptr; }
	| declaration_specifiers declarator compound_statement
    {
        $2->SetDeclSpec(std::move($1));
        auto funcdef = static_cast<FuncDef*>($2.get());
        funcdef->SetCompound(std::move($3));
        $$ = std::move($2);
    }
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%

void yy::parser::error(const std::string& msg)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", msg.c_str());
}
