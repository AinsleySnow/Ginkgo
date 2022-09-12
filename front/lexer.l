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
#include <string>
#include "Node.h"
#include "../utilities/SymbolTable.h"
#include "parser.hh"

void yyerror(SymbolTable&, const char *);  /* prints grammar violation message */
int yylex(void);

extern int sym_type(const char *);  /* returns type from symbol table */

#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */

static void comment(void);
static int check_type(void);
%}

%%
"/*"                    { comment(); }
"//".*                  { /* consume //-comment */ }

"auto"					{ return(AUTO); }
"break"					{ return(BREAK); }
"case"					{ return(CASE); }
"char"					{ yylval.tag = Tag::_char; return(CHAR); }
"const"					{ yylval.tag = Tag::_const; return(CONST); }
"continue"				{ return(CONTINUE); }
"default"				{ return(DEFAULT); }
"do"					{ return(DO); }
"double"				{ yylval.tag = Tag::_double; return(DOUBLE); }
"else"					{ return(ELSE); }
"enum"					{ return(ENUM); }
"extern"				{ return(EXTERN); }
"float"					{ yylval.tag = Tag::_float; return(FLOAT); }
"for"					{ return(FOR); }
"goto"					{ return(GOTO); }
"if"					{ return(IF); }
"inline"				{ return(INLINE); }
"int"					{ yylval.tag = Tag::_int; return(INT); }
"long"					{ yylval.tag = Tag::_long; return(LONG); }
"register"				{ return(REGISTER); }
"restrict"				{ yylval.tag = Tag::_restrict; return(RESTRICT); }
"return"				{ return(RETURN); }
"short"					{ yylval.tag = Tag::_short; return(SHORT); }
"signed"				{ yylval.tag = Tag::_signed; return(SIGNED); }
"sizeof"				{ return(SIZEOF); }
"static"				{ return(STATIC); }
"struct"				{ return(STRUCT); }
"switch"				{ return(SWITCH); }
"typedef"				{ return(TYPEDEF); }
"union"					{ return(UNION); }
"unsigned"				{ yylval.tag = Tag::_unsigned; return(UNSIGNED); }
"void"					{ return(VOID); }
"volatile"				{ yylval.tag = Tag::_volatile; return(VOLATILE); }
"while"					{ return(WHILE); }
"_Alignas"              { return ALIGNAS; }
"_Alignof"              { return ALIGNOF; }
"_Atomic"               { yylval.tag = Tag::_atomic; return ATOMIC; }
"_Bool"                 { yylval.tag = Tag::_bool; return BOOL; }
"_Complex"              { return COMPLEX; }
"_Generic"              { return GENERIC; }
"_Imaginary"            { return IMAGINARY; }
"_Noreturn"             { return NORETURN; }
"_Static_assert"        { return STATIC_ASSERT; }
"_Thread_local"         { return THREAD_LOCAL; }
"__func__"              { return FUNC_NAME; }

{L}{A}*					{ 
    int type = check_type(); 
    if (type == IDENTIFIER)
    {
        yylval.literal = new std::string(yytext);
        return IDENTIFIER;
    }  
}

{HP}{H}+{IS}?				        { return I_CONSTANT; }
{NZ}{D}*{IS}?				        { return I_CONSTANT; }
"0"{O}*{IS}?				        { return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"		    { return I_CONSTANT; }

{D}+{E}{FS}?				        { return F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?			    { return F_CONSTANT; }
{D}+"."{E}?{FS}?			        { return F_CONSTANT; }
{HP}{H}+{P}{FS}?			        { return F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?			    { return F_CONSTANT; }
{HP}{H}+"."{P}{FS}?			        { return F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ return STRING_LITERAL; }

"..."					{ return ELLIPSIS; }
">>="					{ yylval.tag = Tag::right_assign; return RIGHT_ASSIGN; }
"<<="					{ yylval.tag = Tag::left_assign; return LEFT_ASSIGN; }
"+="					{ yylval.tag = Tag::add_assign; return ADD_ASSIGN; }
"-="					{ yylval.tag = Tag::sub_assign; return SUB_ASSIGN; }
"*="					{ yylval.tag = Tag::mul_assign; return MUL_ASSIGN; }
"/="					{ yylval.tag = Tag::div_assign; return DIV_ASSIGN; }
"%="					{ yylval.tag = Tag::mod_assign; return MOD_ASSIGN; }
"&="					{ yylval.tag = Tag::and_assign; return AND_ASSIGN; }
"^="					{ yylval.tag = Tag::xor_assign; return XOR_ASSIGN; }
"|="					{ yylval.tag = Tag::or_assign; return OR_ASSIGN; }
">>"					{ yylval.tag = Tag::rshift; return RIGHT_OP; }
"<<"					{ yylval.tag = Tag::lshift; return LEFT_OP; }
"++"					{ yylval.tag = Tag::inc; return INC_OP; }
"--"					{ yylval.tag = Tag::dec; return DEC_OP; }
"->"					{ yylval.tag = Tag::arrow; return PTR_OP; }
"&&"					{ yylval.tag = Tag::_and; return AND_OP; }
"||"					{ yylval.tag = Tag::_or; return OR_OP; }
"<="					{ yylval.tag = Tag::lessequal; return LE_OP; }
">="					{ yylval.tag = Tag::greatequal; return GE_OP; }
"=="					{ yylval.tag = Tag::equal; return EQ_OP; }
"!="					{ yylval.tag = Tag::notequal; return NE_OP; }
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
    SymbolTable temp{};
    yyerror(temp, "unterminated comment");
}

static int check_type(void)
{
    switch (sym_type(yytext))
    {
    case TYPEDEF_NAME:                /* previously defined */
        return TYPEDEF_NAME;
    case ENUMERATION_CONSTANT:        /* previously defined */
        return ENUMERATION_CONSTANT;
    default:                          /* includes undefined */
        return IDENTIFIER;
    }
}
