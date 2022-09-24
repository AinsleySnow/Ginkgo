%{
#include <cstdio>
#include <memory>
#include <string>

#include "Node.h"
#include "../utilities/SymbolTable.h"
#include "../utilities/Constant.h"
#include "declaration/Declarator.h"
#include "declaration/Declaration.h"
#include "declaration/DeclSpec.h"
#include "declaration/DirDecl.h"
#include "declaration/Init.h"
#include "declaration/InitDecl.h"
#include "declaration/InitDeclList.h"
#include "declaration/TypeName.h"

#include "expression/AddExpr.h"
#include "expression/AndExpr.h"
#include "expression/ArgvExprList.h"
#include "expression/AssignExpr.h"
#include "expression/CastExpr.h"
#include "expression/CondExpr.h"
#include "expression/EqualExpr.h"
#include "expression/Expression.h"
#include "expression/InclOrExpr.h"
#include "expression/LogicalAndExpr.h"
#include "expression/LogicalOrExpr.h"
#include "expression/MultiExpr.h"
#include "expression/PostfixExpr.h"
#include "expression/PrimaryExpr.h"
#include "expression/RelationExpr.h"
#include "expression/ShiftExpr.h"
#include "expression/UnaryExpr.h"
#include "expression/XorExpr.h"

int yylex(void);
void yyerror(SymbolTable& globalSymbols, const char *s);

%}

%parse-param { SymbolTable& globalSymbols }

%union
{
	std::string* literal;
	Constant* constant;
    Node* node;
    Tag tag;
}

%token	<literal> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	<tag> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<tag> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<tag> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<tag> XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	<tag> CONST RESTRICT VOLATILE
%token  <tag> BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%type <node> declarator declaration declaration_specifiers direct_declarator 
%type <node> initializer init_declarator init_declarator_list 
%type <node> type_name initializer_list

%type <tag> unary_operator assignment_operator
%type <node> primary_expression postfix_expression argument_expression_list 
unary_expression cast_expression multiplicative_expression
additive_expression shift_expression relational_expression
equality_expression and_expression exclusive_or_expression
inclusive_or_expression logical_and_expression logical_or_expression
conditional_expression assignment_expression expression constant_expression

%type <constant> constant
%type <literal> enumerator_list string
%type <tag> type_specifier type_qualifier

%start translation_unit
%%
primary_expression
	: IDENTIFIER 
    { $$ = new PrimaryExpr(std::unique_ptr<std::string>($1)); }
	| constant
    {
        $$ = new PrimaryExpr(*$1);
        delete $1;
    }
	| string
    { $$ = new PrimaryExpr(std::unique_ptr<std::string>($1)); }
	| '(' expression ')'
    { $$ = new PrimaryExpr(std::unique_ptr<Expression>(dynamic_cast<Expression*>($2))); }
	| generic_selection
	;

constant
	: I_CONSTANT 
    {
        $$ = new Constant;
        $$->data = static_cast<uint64_t>(std::stoll(*$1));
        $$->type = Type::int32;
        delete $1;
    }
    /* includes character_constant */
	| F_CONSTANT
    {
        $$ = new Constant;
        $$->data = std::stod(*$1);
        $$->type = Type::float64;
        delete $1;
    }
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
	: primary_expression
    { $$ = new PostfixExpr(std::unique_ptr<PrimaryExpr>(dynamic_cast<PrimaryExpr*>($1))); }
	| postfix_expression '[' expression ']'
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            std::unique_ptr<Expression>(dynamic_cast<Expression*>($3))
        );
    }
	| postfix_expression '(' ')'
    { $$ = new PostfixExpr(std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1))); }
	| postfix_expression '(' argument_expression_list ')'
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            std::unique_ptr<ArgvExprList>(dynamic_cast<ArgvExprList*>($3))
        ); 
    }
	| postfix_expression '.' IDENTIFIER
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            Tag::dot,
            std::unique_ptr<std::string>($3)
        );
    }
	| postfix_expression PTR_OP IDENTIFIER
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            Tag::arrow,
            std::unique_ptr<std::string>($3)
        );
    }
	| postfix_expression INC_OP
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            Tag::inc,
            nullptr
        );
    }
	| postfix_expression DEC_OP
    {
        $$ = new PostfixExpr(
            std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1)),
            Tag::dec,
            nullptr
        );
    }
	| '(' type_name ')' '{' initializer_list '}'
    {
        $$ = new PostfixExpr(
            std::unique_ptr<TypeName>(dynamic_cast<TypeName*>($2)),
            std::unique_ptr<InitList>(dynamic_cast<InitList*>($5))
        );
    }
	| '(' type_name ')' '{' initializer_list ',' '}'
    {
        $$ = new PostfixExpr(
            std::unique_ptr<TypeName>(dynamic_cast<TypeName*>($2)),
            std::unique_ptr<InitList>(dynamic_cast<InitList*>($5))
        );
    }
	;

argument_expression_list
	: assignment_expression
    {
        $$ = new ArgvExprList();
        ArgvExprList* ael = dynamic_cast<ArgvExprList*>($$);
        ael->argvExprList.splice(
            ael->argvExprList.end(), 
            dynamic_cast<ArgvExprList*>($1)->argvExprList
        );
        delete $1;
    }
	| argument_expression_list ',' assignment_expression
    {
        ArgvExprList* ael = dynamic_cast<ArgvExprList*>($1);
        ael->argvExprList.splice(ael->argvExprList.end(), dynamic_cast<ArgvExprList*>($3)->argvExprList);
        $$ = $1;
        delete $3;
    }
	;

unary_expression
	: postfix_expression
    { $$ = new UnaryExpr(std::unique_ptr<PostfixExpr>(dynamic_cast<PostfixExpr*>($1))); }
	| INC_OP unary_expression
    { $$ = new UnaryExpr(Tag::inc, std::unique_ptr<UnaryExpr>(dynamic_cast<UnaryExpr*>($2))); }
	| DEC_OP unary_expression
    { $$ = new UnaryExpr(Tag::dec, std::unique_ptr<UnaryExpr>(dynamic_cast<UnaryExpr*>($2))); }
	| unary_operator cast_expression
    { $$ = new UnaryExpr($1, std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($2))); }
	| SIZEOF unary_expression
    { $$ = new UnaryExpr(Tag::_sizeof, std::unique_ptr<UnaryExpr>(dynamic_cast<UnaryExpr*>($2))); }
	| SIZEOF '(' type_name ')'
    { $$ = new UnaryExpr(Tag::_sizeof, std::unique_ptr<TypeName>(dynamic_cast<TypeName*>($3))); }
	| ALIGNOF '(' type_name ')'
    { $$ = new UnaryExpr(Tag::_alignof, std::unique_ptr<TypeName>(dynamic_cast<TypeName*>($3))); }
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
	: unary_expression
	{ $$ = new CastExpr(std::unique_ptr<UnaryExpr>(dynamic_cast<UnaryExpr*>($1))); }
	| '(' type_name ')' cast_expression
	{ 
		$$ = new CastExpr(std::unique_ptr<TypeName>(dynamic_cast<TypeName*>($2)), 
						  std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($4))); 
	}
	;

multiplicative_expression
	: cast_expression
	{ $$ = new MultiExpr(std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($1))); }
	| multiplicative_expression '*' cast_expression
	{
		$$ = new MultiExpr(
			std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($1)), 
			Tag::star, 
			std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($3))
		); 
	}
	| multiplicative_expression '/' cast_expression
	{
		$$ = new MultiExpr(
			std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($1)), 
			Tag::slash, 
			std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($3))
		); 
	}
	| multiplicative_expression '%' cast_expression
	{
		$$ = new MultiExpr(
			std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($1)), 
			Tag::percent, 
			std::unique_ptr<CastExpr>(dynamic_cast<CastExpr*>($3))
		);
	}
	;

additive_expression
	: multiplicative_expression
	{ $$ = new AddExpr(std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($1))); }
	| additive_expression '+' multiplicative_expression
    {
		$$ = new AddExpr(
			std::unique_ptr<AddExpr>(dynamic_cast<AddExpr*>($1)), 
			Tag::plus, 
			std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($3))
		);
	}
	| additive_expression '-' multiplicative_expression
	{
		$$ = new AddExpr(
			std::unique_ptr<AddExpr>(dynamic_cast<AddExpr*>($1)), 
			Tag::minus, 
			std::unique_ptr<MultiExpr>(dynamic_cast<MultiExpr*>($3))
		);
	}
    ;

shift_expression
	: additive_expression
    { $$ = new ShiftExpr(std::unique_ptr<AddExpr>(dynamic_cast<AddExpr*>($1))); }
	| shift_expression LEFT_OP additive_expression
	{
		$$ = new ShiftExpr(
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($1)), 
			Tag::lshift, 
			std::unique_ptr<AddExpr>(dynamic_cast<AddExpr*>($3))
		);
	}
	| shift_expression RIGHT_OP additive_expression
    {
		$$ = new ShiftExpr(
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($1)), 
			Tag::rshift, 
			std::unique_ptr<AddExpr>(dynamic_cast<AddExpr*>($3))
		);
	}
	;

relational_expression
	: shift_expression
    { $$ = new RelationExpr(std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($1))); }
	| relational_expression '<' shift_expression
    {
		$$ = new RelationExpr(
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($1)), 
			Tag::lessthan, 
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($3))
		);
	}
	| relational_expression '>' shift_expression
    {
		$$ = new RelationExpr(
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($1)), 
			Tag::greathan, 
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($3))
		);
	}
	| relational_expression LE_OP shift_expression
    {
		$$ = new RelationExpr(
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($1)), 
			Tag::lessequal, 
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($3))
		);
	}
	| relational_expression GE_OP shift_expression
    {
		$$ = new RelationExpr(
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($1)), 
			Tag::greatequal, 
			std::unique_ptr<ShiftExpr>(dynamic_cast<ShiftExpr*>($3))
		);
	}
	;

equality_expression
	: relational_expression
    { $$ = new EqualExpr(std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($1))); }
	| equality_expression EQ_OP relational_expression
    {
		$$ = new EqualExpr(
			std::unique_ptr<EqualExpr>(dynamic_cast<EqualExpr*>($1)), 
			Tag::equal, 
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($3))
		);
	}
	| equality_expression NE_OP relational_expression
    {
		$$ = new EqualExpr(
			std::unique_ptr<EqualExpr>(dynamic_cast<EqualExpr*>($1)), 
			Tag::notequal, 
			std::unique_ptr<RelationExpr>(dynamic_cast<RelationExpr*>($3))
		);
	}
	;

and_expression
	: equality_expression
    { $$ = new AndExpr(std::unique_ptr<EqualExpr>(dynamic_cast<EqualExpr*>($1))); }
	| and_expression '&' equality_expression
    {
		$$ = new AndExpr(
			std::unique_ptr<AndExpr>(dynamic_cast<AndExpr*>($1)), 
			std::unique_ptr<EqualExpr>(dynamic_cast<EqualExpr*>($3))
		);
	}
	;

exclusive_or_expression
	: and_expression
    { $$ = new XorExpr(std::unique_ptr<AndExpr>(dynamic_cast<AndExpr*>($1))); }
	| exclusive_or_expression '^' and_expression
    {
		$$ = new XorExpr(
			std::unique_ptr<XorExpr>(dynamic_cast<XorExpr*>($1)),
			std::unique_ptr<AndExpr>(dynamic_cast<AndExpr*>($3))
		);
	}
	;

inclusive_or_expression
	: exclusive_or_expression
    { $$ = new InclOrExpr(std::unique_ptr<XorExpr>(dynamic_cast<XorExpr*>($1))); }
	| inclusive_or_expression '|' exclusive_or_expression
    {
		$$ = new InclOrExpr(
			std::unique_ptr<InclOrExpr>(dynamic_cast<InclOrExpr*>($1)),
			std::unique_ptr<XorExpr>(dynamic_cast<XorExpr*>($3))
		);
	}
	;

logical_and_expression
	: inclusive_or_expression
    { $$ = new LogicalAndExpr(std::unique_ptr<InclOrExpr>(dynamic_cast<InclOrExpr*>($1))); }
	| logical_and_expression AND_OP inclusive_or_expression
    {
		$$ = new LogicalAndExpr(
			std::unique_ptr<LogicalAndExpr>(dynamic_cast<LogicalAndExpr*>($1)),
			std::unique_ptr<InclOrExpr>(dynamic_cast<InclOrExpr*>($3))
		);
	}
	;

logical_or_expression
	: logical_and_expression
    { $$ = new LogicalOrExpr(std::unique_ptr<LogicalAndExpr>(dynamic_cast<LogicalAndExpr*>($1))); }
	| logical_or_expression OR_OP logical_and_expression
    {
		$$ = new LogicalOrExpr(
			std::unique_ptr<LogicalOrExpr>(dynamic_cast<LogicalOrExpr*>($1)),
			std::unique_ptr<LogicalAndExpr>(dynamic_cast<LogicalAndExpr*>($3))
		);
	}
	;

conditional_expression
	: logical_or_expression
    { $$ = new CondExpr(std::unique_ptr<LogicalOrExpr>(dynamic_cast<LogicalOrExpr*>($1))); }
	| logical_or_expression '?' expression ':' conditional_expression
    {
		$$ = new CondExpr(
			std::unique_ptr<LogicalOrExpr>(dynamic_cast<LogicalOrExpr*>($1)),
            std::unique_ptr<Expression>(dynamic_cast<Expression*>($3)),
			std::unique_ptr<CondExpr>(dynamic_cast<CondExpr*>($5))
		);
	}
	;

assignment_expression
	: conditional_expression
    { $$ = new AssignExpr(std::unique_ptr<CondExpr>(dynamic_cast<CondExpr*>($1))); }
	| unary_expression assignment_operator assignment_expression
    {
		$$ = new AssignExpr(
			std::unique_ptr<UnaryExpr>(dynamic_cast<UnaryExpr*>($1)),
			$2,
			std::unique_ptr<AssignExpr>(dynamic_cast<AssignExpr*>($3))
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
	: assignment_expression
    {
        $$ = new Expression();
        (dynamic_cast<Expression*>($$)->exprs).push_back(
            *dynamic_cast<AssignExpr*>($1));
        delete $1;
    }
	| expression ',' assignment_expression
    {
        (dynamic_cast<Expression*>($1)->exprs).push_back(
            *dynamic_cast<AssignExpr*>($3));
        delete $3;
        $$ = $1;
    }
	;

constant_expression
	: conditional_expression	/* with constraints */
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
    { 
        $$ = new Declaration(
            std::unique_ptr<DeclSpec>(dynamic_cast<DeclSpec*>($1)),
            std::unique_ptr<InitDeclList>(dynamic_cast<InitDeclList*>($2))
        );
    }
	| static_assert_declaration
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
	| storage_class_specifier
	| type_specifier declaration_specifiers
    {
        DeclSpec* temp = new DeclSpec();
        temp->MarkSpec($1);
        temp->Join(dynamic_cast<DeclSpec*>($2));
        delete $2;
        $$ = temp;
    }
	| type_specifier
    {
        DeclSpec* temp = new DeclSpec();
        temp->MarkSpec($1);
        $$ = temp;
    }
	| type_qualifier declaration_specifiers
    {
        DeclSpec* temp = new DeclSpec();
        temp->MarkQual($1);
        temp->Join(dynamic_cast<DeclSpec*>($2));
        delete $2;
        $$ = temp;
    }
	| type_qualifier
    {
        DeclSpec* temp = new DeclSpec();
        temp->MarkQual($1);
        $$ = temp;
    }
	| function_specifier declaration_specifiers
	| function_specifier
	| alignment_specifier declaration_specifiers
	| alignment_specifier
	;

init_declarator_list
	: init_declarator 
    { 
        InitDeclList* temp = new InitDeclList();
        temp->initList.push_back(*dynamic_cast<InitDecl*>($1));
        $$ = temp;
    }
	| init_declarator_list ',' init_declarator
    {
        InitDeclList* temp = dynamic_cast<InitDeclList*>($1);
        temp->initList.push_back(*dynamic_cast<InitDecl*>($3));
        $$ = temp;
    }
	;

init_declarator
	: declarator '=' initializer 
    {
        $$ = new InitDecl(
            std::unique_ptr<Declarator>(dynamic_cast<Declarator*>($1)),
            std::unique_ptr<Init>(dynamic_cast<Init*>($3))
        );
    }
	| declarator 
    {
        $$ = new InitDecl(
            std::unique_ptr<Declarator>(dynamic_cast<Declarator*>($1))
        );
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
	: pointer direct_declarator
	| direct_declarator 
    { $$ = new Declarator(std::unique_ptr<DirDecl>(dynamic_cast<DirDecl*>($1))); }
	;

direct_declarator
	: IDENTIFIER
    { $$ = new DirDecl(*$1); delete $1;}
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
	| assignment_expression         
    { $$ = new Init(std::unique_ptr<AssignExpr>(dynamic_cast<AssignExpr*>($1))); }
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
	: declaration
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
        globalSymbols.RegisterSymbol(*dynamic_cast<Declaration*>($1));
        delete $1;
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
