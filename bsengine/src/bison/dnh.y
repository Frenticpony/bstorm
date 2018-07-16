%skeleton "lalr1.cc"
%require "3.0"

%define api.namespace {bstorm}
%define parser_class_name {DnhParser}
%locations
%define api.location.type {bstorm::SourceLoc}
%define parse.error verbose

%parse-param { bstorm::DnhParseContext* ctx }
%lex-param { bstorm::DnhParseContext* ctx }

%code requires
{ // top dnh.tab.hpp
#include <bstorm/source_map.hpp>
#include <bstorm/node.hpp>
#include <bstorm/env.hpp>

namespace bstorm
{
class DnhLexer;
struct DnhParseContext
{
    DnhParseContext(DnhLexer* lexer, bool expandInclude) :
        env(std::make_shared<Env>()),
        lexer(lexer),
        expandInclude(expandInclude){}
    DnhParseContext(const std::shared_ptr<Env>& globalEnv, DnhLexer* lexer, bool expandInclude) :
        env(globalEnv ? globalEnv : std::make_shared<Env>()),
        lexer(lexer),
        expandInclude(expandInclude){}
    std::shared_ptr<Env> env;
    std::shared_ptr<NodeBlock> result;
    std::vector<NodeHeader> headers;
    const bool expandInclude;
    DnhLexer* lexer;
};
}
}

%code
{ // dnh.tab.cpp after #include dnh.tab.hpp
#include "../reflex/dnh_lexer.hpp"

#include <bstorm/logger.hpp>
#include <bstorm/util.hpp>

#include <regex>

using namespace bstorm;

static int yylex(DnhParser::semantic_type *yylval, DnhParser::location_type* yylloc, DnhParseContext* ctx)
{
    auto lexer = ctx->lexer;
    auto tk = lexer->dnhlex();
    switch(tk)
    {
        case DnhParser::token_type::TK_NUM:
        case DnhParser::token_type::TK_IDENT:
            yylval->str = new std::string(lexer->GetString());
            break;
        case DnhParser::token_type::TK_HEADER:
        case DnhParser::token_type::TK_STR:
            yylval->wstr = new std::wstring(lexer->GetWString());
            break;
        case DnhParser::token_type::TK_CHAR:
            yylval->wchar = lexer->GetWChar();
            break;
    }
    yylloc->begin = lexer->GetSourcePos();
    return tk;
}

void DnhParser::error(const DnhParser::location_type& yylloc, const std::string& msg)
{
    throw Log(Log::Level::LV_ERROR)
      .SetMessage(msg)
      .AddSourcePos(std::make_shared<SourcePos>(yylloc.begin));
}

static std::shared_ptr<NodeExp> exp(NodeExp* exp) { return std::shared_ptr<NodeExp>(exp); }
static std::shared_ptr<NodeStmt> stmt(NodeStmt* stmt) { return std::shared_ptr<NodeStmt>(stmt); }
static std::shared_ptr<NodeLeftVal> leftval(NodeLeftVal* leftval) { return std::shared_ptr<NodeLeftVal>(leftval); }
static std::shared_ptr<NodeBlock> block(NodeBlock* block) { return std::shared_ptr<NodeBlock>(block); }

static void FixPos(Node *node, const DnhParser::location_type& yylloc)
{
    node->srcPos = std::make_shared<SourcePos>(yylloc.begin);
}

static void CheckDupDef(DnhParseContext* ctx, const DnhParser::location_type& yylloc, const std::string& name)
{
    if (ctx->env->table.count(name) != 0)
    {
        auto prevDef = ctx->env->table[name];
        auto prevDefLine = std::to_string(prevDef->srcPos->line);
        auto prevDefPath = ToUTF8(*prevDef->srcPos->filename);
        auto msg = "found a duplicate definition of '" + prevDef->name + "' (previous definition was at line " + prevDefLine + " in " + prevDefPath + ").";
        throw Log(Log::Level::LV_ERROR)
          .SetMessage(msg)
          .AddSourcePos(std::make_shared<SourcePos>(yylloc.begin));
    }
}

static void AddDef(DnhParseContext* ctx, NodeDef* def)
{
    ctx->env->table[def->name] = std::shared_ptr<NodeDef>(def);
}
}

%union
{
    std::vector<std::shared_ptr<NodeStmt>> *stmts;
    std::vector<std::shared_ptr<NodeExp>> *exps;
    std::vector<std::shared_ptr<NodeElseIf>> *elsifs;
    std::vector<std::shared_ptr<NodeCase>> *cases;
    std::vector<std::wstring> *wstrs;
    std::vector<std::string> *strs;
    NodeRange *range;
    NodeBlock *block;
    NodeElseIf *elsif;
    NodeCase *case_;
    NodeLeftVal *leftval;
    NodeStmt *stmt;
    NodeExp *exp;
    std::wstring *wstr;
    std::string *str;
    wchar_t wchar;
}

%destructor { delete $$; } <stmts>
%destructor { delete $$; } <exps>
%destructor { delete $$; } <elsifs>
%destructor { delete $$; } <cases>
%destructor { delete $$; } <wstrs>
%destructor { delete $$; } <strs>
%destructor { delete $$; } <range>
%destructor { delete $$; } <block>
%destructor { delete $$; } <elsif>
%destructor { delete $$; } <case_>
%destructor { delete $$; } <leftval>
%destructor { delete $$; } <stmt>
%destructor { delete $$; } <exp>
%destructor { delete $$; } <wstr>
%destructor { delete $$; } <str>

/* keyword */
%token TK_ALTERNATIVE "alternative"
%token TK_ASCENT "ascent"
%token TK_BREAK "break"
%token TK_CASE "case"
%token TK_DESCENT "descent"
%token TK_ELSE "else"
%token TK_FUNCTION "function"
%token TK_IF "if"
%token TK_IN "in"
%token TK_LOCAL "local"
%token TK_LOOP "loop"
%token TK_REAL "real"
%token TK_RETURN "return"
%token TK_LET "let"
%token TK_VAR "var"
%token TK_OTHERS "others"
%token TK_SUB "sub"
%token TK_TASK "task"
%token TK_TIMES "times"
%token TK_WHILE "while"
%token TK_YIELD "yield"

%token TK_ATMARK "@"

/* operator */
%token TK_PLUS "+"
%token TK_MINUS "-"
%token TK_MUL "*"
%token TK_DIV "/"
%token TK_REM "%"
%token TK_POW "^"
%token TK_CAT "~"
%token TK_SUCC "++"
%token TK_PRED "--"
%token TK_DOTDOT ".."
%token TK_ASSIGN "="
%token TK_ADDASSIGN "+="
%token TK_SUBASSIGN "-="
%token TK_MULASSIGN "*="
%token TK_DIVASSIGN "/="
%token TK_REMASSIGN "%="
%token TK_POWASSIGN "^="
%token TK_CATASSIGN "~="

/* logical operator */
%token TK_LT "<"
%token TK_GT ">"
%token TK_GE ">="
%token TK_LE "<="
%token TK_EQ "=="
%token TK_NE "!="
%token TK_AND "&&"
%token TK_OR "||"
%token TK_NOT "!"

/* separator */
%token TK_SEMI ";"
%token TK_COMMA ","
/* paren */
%token TK_LPAREN "("
%token TK_RPAREN ")"
%token TK_LBRACKET "["
%token TK_RBRACKET "]"
%token TK_LBRACE "{"
%token TK_RBRACE "}"
%token TK_LABSPAREN "(|"
%token TK_RABSPAREN "|)"
/* literal */
%token <str> TK_NUM "<number>"
%token <str> TK_IDENT "<identifier>"
%token <wchar> TK_CHAR "<char>"
%token <wstr> TK_STR "<string>"

/* for header */
%token <wstr> TK_HEADER "#<header>"
%token TK_IGNORED_HEADER "##..#<header>"

%token TK_EOF 0 "end of file"

%left TK_OR
%left TK_AND
%left TK_EQ TK_NE TK_LT TK_GT TK_LE TK_GE

%left TK_CAT TK_PLUS TK_MINUS
%left TK_MUL TK_DIV TK_REM
%right UPLUS UMINUS TK_NOT
%right TK_POW

%type <stmts> stmts stmts1 term-by-single term-by-compound
%type <stmt> single-stmt compound-stmt
%type <stmt> call-stmt return yield break var-assign var-init
%type <stmt> loop-stmt local loop times while ascent descent
%type <stmt> if alternative
%type <stmt> header ignored-header
%type <block> block loop-body opt-else opt-others
%type <elsifs> elsifs
%type <elsif> elsif
%type <cases> cases
%type <case_> case
%type <leftval> left-value
%type <range> range
%type <exps> exps exps1 indices indices1
%type <exp> cond
%type <exp> exp primary monoop binop call-exp lit array array-access
%type <strs> params params1 opt-params
%type <str> param loop-param
%type <wstr> header-param
%type <wstrs> header-params

%start program

%%
program            : stmts TK_EOF
                      {
                          ctx->result = std::make_shared<NodeBlock>(ctx->env, std::move(*$1));
                          ctx->result->srcPos = std::make_shared<SourcePos>(SourcePos({1, 1, ctx->lexer->GetCurrentFilePath()}));
                          delete($1);
                      }

stmts              : single-stmt
                       {
                         if ($1)
                         {
                             $$ = new std::vector<std::shared_ptr<NodeStmt>>{stmt($1)};
                         } else {
                             $$ = new std::vector<std::shared_ptr<NodeStmt>>();
                         }
                       }
                   | stmts1 single-stmt { $$ = $1; if ($2) { $1->push_back(stmt($2)); } }

stmts1             : term-by-single TK_SEMI { $$ = $1; }
                   | term-by-compound

term-by-single     : single-stmt
                       {
                         if ($1)
                         {
                             $$ = new std::vector<std::shared_ptr<NodeStmt>>{stmt($1)};
                         } else
                         {
                             $$ = new std::vector<std::shared_ptr<NodeStmt>>();
                         }
                       }
                   | stmts1 single-stmt { $$ = $1; if ($2) { $1->push_back(stmt($2)); } }

term-by-compound   : compound-stmt
                       {
                         if ($1) {
                           $$ = new std::vector<std::shared_ptr<NodeStmt>>{stmt($1)};
                         } else {
                           $$ = new std::vector<std::shared_ptr<NodeStmt>>();
                         }
                       }
                   | stmts1 compound-stmt { $$ = $1; if ($2) { $1->push_back(stmt($2)); } }

single-stmt        : none             { $$ = NULL; }
                   | var-decl         { $$ = NULL; }
                   | var-init
                   | var-assign
                   | call-stmt
                   | return
                   | yield
                   | break
                   | left-value TK_SUCC { $$ = new NodeSucc(leftval($1)); FixPos($$, @2); }
                   | left-value TK_PRED { $$ = new NodePred(leftval($1)); FixPos($$, @2); }

call-stmt          : TK_IDENT { $$ = new NodeCallStmt(*$1, {}); FixPos($$, @1); delete($1); }
                   | TK_IDENT TK_LPAREN exps TK_RPAREN { $$ = new NodeCallStmt(*$1, std::move(*$3)); FixPos($$, @1); delete($1); delete($3); }

yield              : TK_YIELD { $$ = new NodeYield(); FixPos($$, @1); }
break              : TK_BREAK { $$ = new NodeBreak(); FixPos($$, @1); }

new-scope           : { auto newEnv = std::make_shared<Env>(); newEnv->parent = ctx->env; ctx->env = newEnv; }

block              : TK_LBRACE stmts TK_RBRACE
                       {
                           $$ = new NodeBlock(ctx->env, std::move(*$2));
                           FixPos($$, @1);
                           delete($2);
                           ctx->env = ctx->env->parent;
                       }


compound-stmt      : builtin-sub-def { $$ = NULL; }
                   | sub-def         { $$ = NULL; }
                   | func-def        { $$ = NULL; }
                   | task-def        { $$ = NULL; }
                   | local
                   | if
                   | alternative
                   | loop-stmt
                   | header
                   | ignored-header

loop-stmt          : loop
                   | times
                   | while
                   | ascent
                   | descent

builtin-sub-def    : TK_ATMARK TK_IDENT { CheckDupDef(ctx, @2, *$2); } new-scope opt-nullparams block { auto def = new NodeBuiltInSubDef(*$2, block($6)); FixPos(def, @1); AddDef(ctx, def); delete($2); }

sub-def            : TK_SUB TK_IDENT { CheckDupDef(ctx, @2, *$2); } new-scope opt-nullparams block { auto def = new NodeSubDef(*$2, block($6)); FixPos(def, @1); AddDef(ctx, def); delete($2); }

func-def           : TK_FUNCTION TK_IDENT { CheckDupDef(ctx, @2, *$2); } new-scope opt-params
                       {
                         if (ctx->env->table.count("result") == 0)
                         {
                             auto result = new NodeVarDecl("result");
                             FixPos(result, @1);
                             AddDef(ctx, result);
                         }
                       }
                       block
                       {
                           auto def = new NodeFuncDef(*$2, std::move(*$5), block($7));
                           FixPos(def, @1);
                           AddDef(ctx, def);
                           delete($2); delete($5);
                       }

task-def           : TK_TASK TK_IDENT { CheckDupDef(ctx, @2, *$2); } new-scope opt-params block { auto def = new NodeTaskDef(*$2, std::move(*$5), block($6)); FixPos(def, @1); AddDef(ctx, def); delete($2); delete($5); }

params             : params1 opt-comma { $$ = $1; }
                   | none { $$ = new std::vector<std::string>(); }
params1            : params1 TK_COMMA param { $$ = $1; $1->push_back(*$3); delete $3; }
                   | param { $$ = new std::vector<std::string>{*$1}; delete $1; }
param              : opt-declarator TK_IDENT { auto param = new NodeProcParam(*$2); FixPos(param, @2); AddDef(ctx, param); $$ = $2; }

opt-comma          : none | TK_COMMA

opt-declarator     : none
                   | declarator

opt-params         : TK_LPAREN params TK_RPAREN { $$ = $2; }
                   | none { $$ = new std::vector<std::string>(); }

opt-nullparams     : TK_LPAREN TK_RPAREN
                   | none

local              : TK_LOCAL new-scope block { $$ = new NodeLocal(block($3)); FixPos($$, @1); }

if                 : TK_IF cond new-scope block elsifs opt-else { $$ = new NodeIf(exp($2), block($4), std::move(*$5), block($6)); FixPos($$, @1); delete($5); }

elsifs             : none         { $$ = new std::vector<std::shared_ptr<NodeElseIf>>(); }
                   | elsifs elsif { $$ = $1; $$->push_back(std::shared_ptr<NodeElseIf>($2)); }

elsif              : TK_ELSE TK_IF cond new-scope block { $$ = new NodeElseIf(exp($3), block($5)); FixPos($$, @1); }

opt-else           : TK_ELSE new-scope block { $$ = $3; }
                   | none                    { $$ = NULL; }

alternative        : TK_ALTERNATIVE TK_LPAREN exp TK_RPAREN cases opt-others { $$ = new NodeAlternative(exp($3), std::move(*$5), block($6)); FixPos($$, @1); delete($5); }

cases              : none       { $$ = new std::vector<std::shared_ptr<NodeCase>>(); }
                   | cases case { $$ = $1; $$->push_back(std::shared_ptr<NodeCase>($2)); }

case               : TK_CASE TK_LPAREN exps1 TK_RPAREN new-scope block { $$ = new NodeCase(std::move(*$3), block($6)); FixPos($$, @1); delete $3; }

opt-others         : TK_OTHERS new-scope block  { $$ = $3; FixPos($3, @1); }
                   | none                       { $$ = NULL; }

loop               : TK_LOOP new-scope block                         { $$ = new NodeLoop(block($3)); FixPos($$, @1); }
                   | TK_LOOP TK_LPAREN exp TK_RPAREN new-scope block { $$ = new NodeTimes(exp($3), block($6)); FixPos($$, @1); }

times              : TK_TIMES TK_LPAREN exp TK_RPAREN new-scope loop-body { $$ = new NodeTimes(exp($3), block($6)); FixPos($$, @1); }

while              : TK_WHILE cond new-scope loop-body { $$ = new NodeWhile(exp($2), block($4)); FixPos($$, @1); }

ascent             : TK_ASCENT new-scope TK_LPAREN loop-param TK_IN range TK_RPAREN loop-body { $$ = new NodeAscent(*$4, std::shared_ptr<NodeRange>($6), block($8)); FixPos($$, @1); delete $4; }
descent            : TK_DESCENT new-scope TK_LPAREN loop-param TK_IN range TK_RPAREN loop-body { $$ = new NodeDescent(*$4, std::shared_ptr<NodeRange>($6), block($8)); FixPos($$, @1); delete $4; }

loop-body          : TK_LOOP block { $$ = $2; }
                   | block

header             : TK_HEADER TK_LBRACKET header-params TK_RBRACKET
                       {
                           if (ctx->lexer->GetIncludeStackSize() == 1)
                           {
                               auto header = new NodeHeader(*$1, std::move(*$3));
                               FixPos(header, @1);
                               ctx->headers.push_back(*header);
                               $$ = header;
                           } else
                           {
                               $$ = NULL;
                           }
                           delete $1; delete $3;
                       }
                   | TK_HEADER TK_STR
                       {
                           auto name = *$1;
                           auto param = *$2; 
                           if (ctx->expandInclude && name == L"include")
                           {
                               $$ = NULL;
                               ctx->lexer->PushInclude(param);
                           } else
                           {
                               if (ctx->lexer->GetIncludeStackSize() == 1)
                               {
                                   auto header = new NodeHeader(name, {param});
                                   FixPos(header, @1);
                                   ctx->headers.push_back(*header);
                                   $$ = header;
                               } else
                               {
                                   $$ = NULL;
                               }
                           }
                           delete $1; delete $2;
                       }

ignored-header     : TK_IGNORED_HEADER TK_LBRACKET header-params TK_RBRACKET { $$ = NULL; delete $3; }
                   | TK_IGNORED_HEADER TK_STR { $$ = NULL; delete $2; }

header-params      : header-params header-param { $$ = $1; $1->push_back(*$2); delete $2; }
                   | header-params TK_COMMA { $$ = $1;}
                   | none { $$ = new std::vector<std::wstring>(); }
header-param       : TK_STR 
                   | TK_IDENT { $$ = new std::wstring(ToUnicode(*$1)); delete($1); }
                   | TK_NUM { $$ = new std::wstring(ToUnicode(*$1)); delete($1); }

range              : exp TK_DOTDOT exp { $$ = new NodeRange(exp($1), exp($3)); FixPos($$, @2); }

cond               : TK_LPAREN exp TK_RPAREN { $$ = $2; }

loop-param         : opt-declarator TK_IDENT { auto param = new NodeLoopParam(*$2); FixPos(param, @2); AddDef(ctx, param); $$ = $2; }

exps               : exps1 opt-comma { $$ = $1; }
                   | none { $$ = new std::vector<std::shared_ptr<NodeExp>>(); }
exps1              : exps1 TK_COMMA exp { $$ = $1; $1->push_back(exp($3)); }
                   | exp { $$ = new std::vector<std::shared_ptr<NodeExp>>{exp($1)}; }

return             : TK_RETURN exp { $$ = new NodeReturn(exp($2)); FixPos($$, @1); }
                   | TK_RETURN     { $$ = new NodeReturnVoid(); FixPos($$, @1); }

left-value         : TK_IDENT indices { $$ = new NodeLeftVal(*$1, std::move(*$2)); FixPos($$, @1); delete($1); delete($2); }

indices            : indices1
                   | none  { $$ = new std::vector<std::shared_ptr<NodeExp>>(); }
indices1           : TK_LBRACKET exp TK_RBRACKET          { $$ = new std::vector<std::shared_ptr<NodeExp>>{exp($2)}; }
                   | indices1 TK_LBRACKET exp TK_RBRACKET { $$ = $1; $1->push_back(exp($3)); }

declarator         : TK_LET | TK_REAL | TK_VAR
var-decl           : declarator TK_IDENT { CheckDupDef(ctx, @2, *$2); } { auto varDecl = new NodeVarDecl(*$2); FixPos(varDecl, @1); AddDef(ctx, varDecl); delete($2); }

var-init           : declarator TK_IDENT { CheckDupDef(ctx, @2, *$2); } TK_ASSIGN exp { $$ = new NodeVarInit(*$2, exp($5)); auto varDecl = new NodeVarDecl(*$2); FixPos($$, @1); FixPos(varDecl, @1); AddDef(ctx, varDecl); delete $2; }

var-assign         : left-value TK_ASSIGN exp    { $$ = new NodeAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_ADDASSIGN exp { $$ = new NodeAddAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_SUBASSIGN exp { $$ = new NodeSubAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_MULASSIGN exp { $$ = new NodeMulAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_DIVASSIGN exp { $$ = new NodeDivAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_REMASSIGN exp { $$ = new NodeRemAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_POWASSIGN exp { $$ = new NodePowAssign(leftval($1), exp($3)); FixPos($$, @2); }
                   | left-value TK_CATASSIGN exp { $$ = new NodeCatAssign(leftval($1), exp($3)); FixPos($$, @2); }

exp                : primary
                   | monoop
                   | binop

primary            : lit
                   | TK_LPAREN exp TK_RPAREN       { $$ = $2; }
                   | TK_LABSPAREN exp TK_RABSPAREN { $$ = new NodeAbs(exp($2)); FixPos($$, @1); }
                   | call-exp
                   | array-access

monoop             : TK_PLUS  exp %prec UPLUS  { $$ = $2; }
                   | TK_MINUS exp %prec UMINUS { $$ = new NodeNeg(exp($2)); FixPos($$, @1); }
                   | TK_NOT   exp              { $$ = new NodeNot(exp($2)); FixPos($$, @1); }

binop              : exp TK_PLUS exp  { $$ = new NodeAdd(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_MINUS exp { $$ = new NodeSub(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_MUL exp   { $$ = new NodeMul(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_DIV exp   { $$ = new NodeDiv(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_REM exp   { $$ = new NodeRem(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_POW exp   { $$ = new NodePow(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_CAT exp   { $$ = new NodeCat(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_LT exp    { $$ = new NodeLt(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_GT exp    { $$ = new NodeGt(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_LE exp    { $$ = new NodeLe(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_GE exp    { $$ = new NodeGe(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_EQ exp    { $$ = new NodeEq(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_NE exp    { $$ = new NodeNe(exp($1),exp($3));  FixPos($$, @2); }
                   | exp TK_AND exp   { $$ = new NodeAnd(exp($1),exp($3)); FixPos($$, @2); }
                   | exp TK_OR exp    { $$ = new NodeOr(exp($1),exp($3));  FixPos($$, @2); }

call-exp           : TK_IDENT                          { $$ = new NodeNoParenCallExp(*$1); FixPos($$, @1); delete($1); }
                   | TK_IDENT TK_LPAREN exps TK_RPAREN { $$ = new NodeCallExp(*$1, std::move(*$3)); FixPos($$, @1); delete($1); delete($3); }

lit                : TK_NUM   { $$ = new NodeNum(std::move(*$1));  FixPos($$, @1); delete $1; }
                   | TK_CHAR  { $$ = new NodeChar($1); FixPos($$, @1); }
                   | TK_STR
                       {
                           // escape: \x -> x
                           $$ = new NodeStr(std::regex_replace(*$1, std::wregex(L"\\\\(.)"), L"$1"));  FixPos($$, @1);
                           delete $1 ;
                       }
                   | array

array              : TK_LBRACKET exps TK_RBRACKET { $$ = new NodeArray(std::move(*$2)); FixPos($$, @1); delete($2); }

array-access       : primary TK_LBRACKET exp TK_RBRACKET   { $$ = new NodeArrayRef(exp($1), exp($3)); FixPos($$, @2); }
                   | primary TK_LBRACKET range TK_RBRACKET { $$ = new NodeArraySlice(exp($1), std::shared_ptr<NodeRange>($3)); FixPos($$, @2); }

none :
%%
