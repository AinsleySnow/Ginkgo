%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{
#include "utils/Scope.h"
#include "parser.hh"
#include <string>


void yyerror(Scope*, const char*);  /* prints grammar violation message */
int yylex(void);

extern int sym_type(const char*);  /* returns type from symbol table */

#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */
#define YYTOKEN yy::parser::token

static void comment(void);
static int check_type(void);
%}

%%
"/*"                    { comment(); }
"//".*                  { /* consume //-comment */ }

"auto"					{ return YYTOKEN::AUTO; }
"break"					{ return YYTOKEN::BREAK; }
"case"					{ return YYTOKEN::CASE; }
"char"					{ yylval->emplace(Tag::_char); return YYTOKEN::CHAR; }
"const"					{ yylval->emplace(Tag::_const); return YYTOKEN::CONST; }
"continue"				{ return YYTOKEN::CONTINUE; }
"default"				{ return YYTOKEN::DEFAULT; }
"do"					{ return YYTOKEN::DO; }
"double"				{ yylval->emplace(Tag::_double); return YYTOKEN::DOUBLE; }
"else"					{ return YYTOKEN::ELSE; }
"enum"					{ return YYTOKEN::ENUM; }
"extern"				{ return YYTOKEN::EXTERN; }
"float"					{ yylval->emplace(Tag::_float); return YYTOKEN::FLOAT; }
"for"					{ return YYTOKEN::FOR; }
"goto"					{ return YYTOKEN::GOTO; }
"if"					{ return YYTOKEN::IF; }
"inline"				{ return YYTOKEN::INLINE; }
"int"					{ yylval->emplace(Tag::_int); return YYTOKEN::INT; }
"long"					{ yylval->emplace(Tag::_long); return YYTOKEN::LONG; }
"register"				{ return YYTOKEN::REGISTER; }
"restrict"				{ yylval->emplace(Tag::_restrict); return YYTOKEN::RESTRICT; }
"return"				{ return YYTOKEN::RETURN; }
"short"					{ yylval->emplace(Tag::_short); return YYTOKEN::SHORT; }
"signed"				{ yylval->emplace(Tag::_signed); return YYTOKEN::SIGNED; }
"sizeof"				{ return YYTOKEN::SIZEOF; }
"static"				{ return YYTOKEN::STATIC; }
"struct"				{ return YYTOKEN::STRUCT; }
"switch"				{ return YYTOKEN::SWITCH; }
"typedef"				{ return YYTOKEN::TYPEDEF; }
"union"					{ return YYTOKEN::UNION; }
"unsigned"				{ yylval->emplace(Tag::_unsigned); return YYTOKEN::UNSIGNED; }
"void"					{ return YYTOKEN::VOID; }
"volatile"				{ yylval->emplace(Tag::_volatile); return YYTOKEN::VOLATILE; }
"while"					{ return YYTOKEN::WHILE; }
"_Alignas"              { return YYTOKEN::ALIGNAS; }
"_Alignof"              { return YYTOKEN::ALIGNOF; }
"_Atomic"               { yylval->emplace(Tag::_atomic); return YYTOKEN::ATOMIC; }
"_Bool"                 { yylval->emplace(Tag::_bool); return YYTOKEN::BOOL; }
"_Complex"              { return YYTOKEN::COMPLEX; }
"_Generic"              { return YYTOKEN::GENERIC; }
"_Imaginary"            { return YYTOKEN::IMAGINARY; }
"_Noreturn"             { return YYTOKEN::NORETURN; }
"_Static_assert"        { return YYTOKEN::STATIC_ASSERT; }
"_Thread_local"         { return YYTOKEN::THREAD_LOCAL; }
"__func__"              { return YYTOKEN::FUNC_NAME; }

{L}{A}*					{ 
    int type = check_type(); 
    if (type == YYTOKEN::IDENTIFIER)
    {
        yylval->emplace(yytext);
        return YYTOKEN::IDENTIFIER;
    }  
}

{HP}{H}+{IS}?				  { yylval->emplace(yytext); return YYTOKEN::I_CONSTANT; }
{NZ}{D}*{IS}?				  { yylval->emplace(yytext); return YYTOKEN::I_CONSTANT; }
"0"{O}*{IS}?				  { yylval->emplace(yytext); return YYTOKEN::I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"	  { yylval->emplace(yytext); return YYTOKEN::I_CONSTANT; }

{D}+{E}{FS}?				  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?		  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }
{D}+"."{E}?{FS}?			  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }
{HP}{H}+{P}{FS}?			  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?		  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }
{HP}{H}+"."{P}{FS}?			  { yylval->emplace(yytext); return YYTOKEN::F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ return YYTOKEN::STRING_LITERAL; }

"..."					{ return YYTOKEN::ELLIPSIS; }
">>="					{ yylval->emplace(Tag::right_assign); return YYTOKEN::RIGHT_ASSIGN; }
"<<="					{ yylval->emplace(Tag::left_assign); return YYTOKEN::LEFT_ASSIGN; }
"+="					{ yylval->emplace(Tag::add_assign); return YYTOKEN::ADD_ASSIGN; }
"-="					{ yylval->emplace(Tag::sub_assign); return YYTOKEN::SUB_ASSIGN; }
"*="					{ yylval->emplace(Tag::mul_assign); return YYTOKEN::MUL_ASSIGN; }
"/="					{ yylval->emplace(Tag::div_assign); return YYTOKEN::DIV_ASSIGN; }
"%="					{ yylval->emplace(Tag::mod_assign); return YYTOKEN::MOD_ASSIGN; }
"&="					{ yylval->emplace(Tag::and_assign); return YYTOKEN::AND_ASSIGN; }
"^="					{ yylval->emplace(Tag::xor_assign); return YYTOKEN::XOR_ASSIGN; }
"|="					{ yylval->emplace(Tag::or_assign); return YYTOKEN::OR_ASSIGN; }
">>"					{ yylval->emplace(Tag::rshift); return YYTOKEN::RIGHT_OP; }
"<<"					{ yylval->emplace(Tag::lshift); return YYTOKEN::LEFT_OP; }
"++"					{ yylval->emplace(Tag::inc); return YYTOKEN::INC_OP; }
"--"					{ yylval->emplace(Tag::dec); return YYTOKEN::DEC_OP; }
"->"					{ yylval->emplace(Tag::arrow); return YYTOKEN::PTR_OP; }
"&&"					{ yylval->emplace(Tag::_and); return YYTOKEN::AND_OP; }
"||"					{ yylval->emplace(Tag::_or); return YYTOKEN::OR_OP; }
"<="					{ yylval->emplace(Tag::lessequal); return YYTOKEN::LE_OP; }
">="					{ yylval->emplace(Tag::greatequal); return YYTOKEN::GE_OP; }
"=="					{ yylval->emplace(Tag::equal); return YYTOKEN::EQ_OP; }
"!="					{ yylval->emplace(Tag::notequal); return YYTOKEN::NE_OP; }
";"					    { return ';'; }
("{"|"<%")				{ return '{'; }
("}"|"%>")				{ return '}'; }
","					    { return ','; }
":"					    { return ':'; }
"="					    { return '='; }
"("					    { return '('; }
")"					    { return ')'; }
("["|"<:")				{ return '['; }
("]"|":>")				{ return ']'; }
"."					    { return '.'; }
"&"					    { return '&'; }
"!"					    { return '!'; }
"~"					    { return '~'; }
"-"					    { return '-'; }
"+"					    { return '+'; }
"*"					    { return '*'; }
"/"					    { return '/'; }
"%"					    { return '%'; }
"<"					    { return '<'; }
">"					    { return '>'; }
"^"					    { return '^'; }
"|"					    { return '|'; }
"?"					    { return '?'; }

{WS}+					{ /* whitespace separates tokens */ }
.					    { /* discard bad characters */ }

%%

int yywrap(void)        /* called at end of input */
{
    return 1;           /* terminate now */
}

static void comment(void)
{
    int c;

    while ((c = yyinput()) != 0)
    {
        if (c == '*')
        {
            while ((c = yyinput()) == '*')
                ;

            if (c == '/')
                return;

            if (c == 0)
                break;
        }
    }
    yyerror(nullptr, "unterminated comment");
}

static int check_type(void)
{
    switch (sym_type(yytext))
    {
    case TYPEDEF_NAME:                /* previously defined */
        return YYTOKEN::TYPEDEF_NAME;
    case ENUMERATION_CONSTANT:        /* previously defined */
        return YYTOKEN::ENUMERATION_CONSTANT;
    default:                          /* includes undefined */
        return YYTOKEN::IDENTIFIER;
    }
}
