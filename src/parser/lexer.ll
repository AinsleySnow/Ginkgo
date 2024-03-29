%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117
%s  inpreprocess

B   [01]
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
BP  (0[bB])
HP  (0[xX])
BS  ({B}({B}|('{B}))*)
OS  ({O}({O}|('{O}))*)
DS  ({D}({D}|('{D}))*)
HS  ({H}({H}|('{H}))*)
E   ([Ee][+-]?{DS})
P   ([Pp][+-]?{DS})
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\f]

/* U8 - expect for [\x00-\x7f] - this one will be handled seperately */
U8  [\xc2-\xdf][\x80-\xbf]|\xe0[\xa0-\xbf][\x80-\xbf]|[\xe1-\xec][\x80-\xbf][\x80-\xbf]|\xed[\x80-\x9f][\x80-\xbf]|[\xee\xef][\x80-\xbf][\x80-\xbf]|\xf0[\x90-\xbf][\x80-\xbf][\x80-\xbf]|[\xf1-\xf3][\x80-\xbf][\x80-\xbf][\x80-\xbf]|\xf4[\x80-\x8f][\x80-\xbf][\x80-\xbf]

%option nounput


%{
#include <cstdlib>
#include <string>
#include <memory>
#include "ast/Tag.h"
#include "yacc.hh"

#undef YY_DECL
#define YY_DECL int yylex(yy::parser::value_type* yylval, CheckType& checktype)
#define YYTOKEN yy::parser::token

static void comment(void);
%}

%%
"/*"                    { comment(); }
"//".*                  { /* consume //-comment */ }

"auto"					{ yylval->emplace<Tag>() = Tag::_auto; return YYTOKEN::AUTO; }
"break"					{ yylval->emplace<Tag>() = Tag::_break; return YYTOKEN::BREAK; }
"case"					{ yylval->emplace<Tag>() = Tag::_case; return YYTOKEN::CASE; }
"char"					{ yylval->emplace<Tag>() = Tag::_char; return YYTOKEN::CHAR; }
"const"					{ yylval->emplace<Tag>() = Tag::_const; return YYTOKEN::CONST; }
"constexpr"             { yylval->emplace<Tag>() = Tag::_constexpr; return YYTOKEN::CONSTEXPR; }
"continue"				{ yylval->emplace<Tag>() = Tag::_continue; return YYTOKEN::CONTINUE; }
"default"				{ yylval->emplace<Tag>() = Tag::_default; return YYTOKEN::DEFAULT; }
"do"					{ yylval->emplace<Tag>() = Tag::_do; return YYTOKEN::DO; }
"double"				{ yylval->emplace<Tag>() = Tag::_double; return YYTOKEN::DOUBLE; }
"else"					{ yylval->emplace<Tag>() = Tag::_else; return YYTOKEN::ELSE; }
"enum"                  { yylval->emplace<Tag>() = Tag::_enum; return YYTOKEN::ENUM; }
"extern"				{ yylval->emplace<Tag>() = Tag::_extern; return YYTOKEN::EXTERN; }
"false"                 { yylval->emplace<Tag>() = Tag::_false; return YYTOKEN::FALSE; }
"float"					{ yylval->emplace<Tag>() = Tag::_float; return YYTOKEN::FLOAT; }
"for"					{ yylval->emplace<Tag>() = Tag::_for; return YYTOKEN::FOR; }
"goto"					{ yylval->emplace<Tag>() = Tag::_goto; return YYTOKEN::GOTO; }
"if"					{ yylval->emplace<Tag>() = Tag::_if; return YYTOKEN::IF; }
"inline"				{ yylval->emplace<Tag>() = Tag::_inline; return YYTOKEN::INLINE; }
"int"					{ yylval->emplace<Tag>() = Tag::_int; return YYTOKEN::INT; }
"long"					{ yylval->emplace<Tag>() = Tag::_long; return YYTOKEN::LONG; }
"nullptr"               { yylval->emplace<Tag>() = Tag::_nullptr; return YYTOKEN::NULLPTR; }
"register"				{ yylval->emplace<Tag>() = Tag::_register; return YYTOKEN::REGISTER; }
"restrict"				{ yylval->emplace<Tag>() = Tag::_restrict; return YYTOKEN::RESTRICT; }
"return"				{ yylval->emplace<Tag>() = Tag::_return; return YYTOKEN::RETURN; }
"short"					{ yylval->emplace<Tag>() = Tag::_short; return YYTOKEN::SHORT; }
"signed"				{ yylval->emplace<Tag>() = Tag::_signed; return YYTOKEN::SIGNED; }
"sizeof"				{ yylval->emplace<Tag>() = Tag::_sizeof; return YYTOKEN::SIZEOF; }
"static"				{ yylval->emplace<Tag>() = Tag::_static; return YYTOKEN::STATIC; }
"struct"				{ yylval->emplace<Tag>() = Tag::_struct; return YYTOKEN::STRUCT; }
"switch"				{ return YYTOKEN::SWITCH; }
"true"                  { yylval->emplace<Tag>() = Tag::_true; return YYTOKEN::TRUE; }
"typedef"				{ yylval->emplace<Tag>() = Tag::_typedef; return YYTOKEN::TYPEDEF; }
"typeof"                { yylval->emplace<Tag>() = Tag::_typeof; return YYTOKEN::TYPEOF; }
"typeof_unqual"         { yylval->emplace<Tag>() = Tag::_typeof_unqual; return YYTOKEN::TYPEOF_UNQUAL; }
"union"                 { yylval->emplace<Tag>() = Tag::_union; return YYTOKEN::UNION; }
"unsigned"				{ yylval->emplace<Tag>() = Tag::_unsigned; return YYTOKEN::UNSIGNED; }
"void"					{ yylval->emplace<Tag>() = Tag::_void; return YYTOKEN::VOID; }
"volatile"				{ yylval->emplace<Tag>() = Tag::_volatile; return YYTOKEN::VOLATILE; }
"while"					{ yylval->emplace<Tag>() = Tag::_while; return YYTOKEN::WHILE; }
"_Alignas"              { yylval->emplace<Tag>() = Tag::_alignas; return YYTOKEN::ALIGNAS; }
"alignas"               { yylval->emplace<Tag>() = Tag::_alignas; return YYTOKEN::ALIGNAS; }
"_Alignof"              { yylval->emplace<Tag>() = Tag::_alignof; return YYTOKEN::ALIGNOF; }
"alignof"               { yylval->emplace<Tag>() = Tag::_alignof; return YYTOKEN::ALIGNOF; }
"_Atomic"               { yylval->emplace<Tag>() = Tag::_atomic; return YYTOKEN::ATOMIC; }
"_Bool"                 { yylval->emplace<Tag>() = Tag::_bool; return YYTOKEN::BOOL; }
"bool"                  { yylval->emplace<Tag>() = Tag::_bool; return YYTOKEN::BOOL; }
"_BitInt"               { return YYTOKEN::BITINT; }
"_Complex"              { return YYTOKEN::COMPLEX; }
"_Decimal32"            { yylval->emplace<Tag>() = Tag::_decimal32; return YYTOKEN::DECIMAL32; }
"_Decimal64"            { yylval->emplace<Tag>() = Tag::_decimal64; return YYTOKEN::DECIMAL64; }
"_Decimal128"           { yylval->emplace<Tag>() = Tag::_decimal128; return YYTOKEN::DECIMAL128; }
"_Generic"              { return YYTOKEN::GENERIC; }
"_Imaginary"            { return YYTOKEN::IMAGINARY; }
"_Noreturn"             { yylval->emplace<Tag>() = Tag::_noreturn; return YYTOKEN::NORETURN; }
"static_assert"         { return YYTOKEN::STATIC_ASSERT; }
"_Thread_local"         { yylval->emplace<Tag>() = Tag::_thread_local; return YYTOKEN::THREAD_LOCAL; }
"thread_local"          { yylval->emplace<Tag>() = Tag::_thread_local; return YYTOKEN::THREAD_LOCAL; }
"__func__"              { return YYTOKEN::FUNC_NAME; }
"__Ginkgo_va_arg"       { return YYTOKEN::GKVAARG; }


({L}|{U8})({A}|{U8})*		{
    yylval->emplace<std::string>() = yytext;
    return checktype(yytext);
}

{BP}{BS}{IS}?                       { yylval->emplace<std::string>() = yytext; return YYTOKEN::I_CONSTANT; }
{HP}{HS}{IS}?                       { yylval->emplace<std::string>() = yytext; return YYTOKEN::I_CONSTANT; }
{NZ}{DS}?{IS}?                      { yylval->emplace<std::string>() = yytext; return YYTOKEN::I_CONSTANT; }
"0"{OS}*{IS}?                       { yylval->emplace<std::string>() = yytext; return YYTOKEN::I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES}|{U8})+"'"    { yylval->emplace<std::string>() = yytext; return YYTOKEN::I_CONSTANT; }

{DS}{E}{FS}?				  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }
{DS}?"."{D}+{E}?{FS}?		  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }
{DS}"."{E}?{FS}?			  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }
{HP}{HS}{P}{FS}?			  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }
{HP}{HS}?"."{HS}{P}{FS}?	  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }
{HP}{HS}"."{P}{FS}?			  { yylval->emplace<std::string>() = yytext; return YYTOKEN::F_CONSTANT; }

<inpreprocess>(\"([^"\\\n]|{ES}|{U8})*\")       {
    yylval->emplace<std::string>() = yytext;
    return YYTOKEN::STRING_LITERAL;
}

<INITIAL>({SP}?\"([^"\\\n]|{ES}|{U8})*\"[ \t\v\f\n]*)+	{
    yylval->emplace<std::string>() = yytext;
    return YYTOKEN::STRING_LITERAL;
}

"..."					{ return YYTOKEN::ELLIPSIS; }
">>="					{ yylval->emplace<Tag>() = Tag::right_assign; return YYTOKEN::RIGHT_ASSIGN; }
"<<="					{ yylval->emplace<Tag>() = Tag::left_assign; return YYTOKEN::LEFT_ASSIGN; }
"+="					{ yylval->emplace<Tag>() = Tag::add_assign; return YYTOKEN::ADD_ASSIGN; }
"-="					{ yylval->emplace<Tag>() = Tag::sub_assign; return YYTOKEN::SUB_ASSIGN; }
"*="					{ yylval->emplace<Tag>() = Tag::mul_assign; return YYTOKEN::MUL_ASSIGN; }
"/="					{ yylval->emplace<Tag>() = Tag::div_assign; return YYTOKEN::DIV_ASSIGN; }
"%="					{ yylval->emplace<Tag>() = Tag::mod_assign; return YYTOKEN::MOD_ASSIGN; }
"&="					{ yylval->emplace<Tag>() = Tag::and_assign; return YYTOKEN::AND_ASSIGN; }
"^="					{ yylval->emplace<Tag>() = Tag::xor_assign; return YYTOKEN::XOR_ASSIGN; }
"|="					{ yylval->emplace<Tag>() = Tag::or_assign; return YYTOKEN::OR_ASSIGN; }
">>"					{ yylval->emplace<Tag>() = Tag::rshift; return YYTOKEN::RIGHT_OP; }
"<<"					{ yylval->emplace<Tag>() = Tag::lshift; return YYTOKEN::LEFT_OP; }
"++"					{ yylval->emplace<Tag>() = Tag::inc; return YYTOKEN::INC_OP; }
"--"					{ yylval->emplace<Tag>() = Tag::dec; return YYTOKEN::DEC_OP; }
"->"					{ yylval->emplace<Tag>() = Tag::arrow; return YYTOKEN::PTR_OP; }
"&&"					{ yylval->emplace<Tag>() = Tag::logical_and; return YYTOKEN::AND_OP; }
"||"					{ yylval->emplace<Tag>() = Tag::logical_or; return YYTOKEN::OR_OP; }
"<="					{ yylval->emplace<Tag>() = Tag::lessequal; return YYTOKEN::LE_OP; }
">="					{ yylval->emplace<Tag>() = Tag::greatequal; return YYTOKEN::GE_OP; }
"=="					{ yylval->emplace<Tag>() = Tag::equal; return YYTOKEN::EQ_OP; }
"!="					{ yylval->emplace<Tag>() = Tag::notequal; return YYTOKEN::NE_OP; }
","					    { return ','; }
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
":"                     { return ':'; }
";"                     { return ';'; }
"="					    { return '='; }

"#"                     {
    BEGIN(inpreprocess);
    return '#';
}

("{"|"<%")				{
    checktype.EnterScope();
    return '{';
}

("}"|"%>")				{
    checktype.LeaveScope();
    return '}';
}

<inpreprocess>"\n"      {
    BEGIN(INITIAL);
    return '\n';
}
<INITIAL>"\n"           { /* not preprocess instruction, discard it */ }
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
    exit(1);
}
