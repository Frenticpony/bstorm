%skeleton "lalr1.cc"
%require "3.0"

%define api.namespace {bstorm}
%define parser_class_name {UserDefDataParser}
%locations
%define api.location.type {bstorm::SourceLoc}
%define parse.error verbose

%lex-param   { bstorm::UserDefDataParseContext* ctx }
%parse-param { bstorm::UserDefDataParseContext* ctx }

%code requires
{
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/source_map.hpp>

#include <string>
#include <memory>

namespace bstorm
{
class UserDefDataLexer;
struct UserDefDataParseContext
{
    UserDefDataParseContext(const std::shared_ptr<UserShotData>& data, UserDefDataLexer* lexer) :
        userShotData(data),
        lexer(lexer) {}
    UserDefDataParseContext(const std::shared_ptr<UserItemData>& data, UserDefDataLexer* lexer) :
        userItemData(data),
        lexer(lexer) {}
    std::shared_ptr<UserShotData> userShotData;
    std::shared_ptr<UserItemData> userItemData;
    std::wstring tmpStr;
    AnimationData animationData;
    ShotData shotData;
    ItemData itemData;
    UserDefDataLexer* lexer;
};

struct Int3 {
  int a; int b; int c;
};
struct Int4 {
  int a; int b; int c; int d;
};
struct Int5 {
  int a; int b; int c; int d; int e;
};
}
}

%code
{ // dnh.tab.cpp after #include dnh.tab.hpp
#include "../reflex/user_def_data_lexer.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/logger.hpp>

#include <algorithm>

using namespace bstorm;

static int yylex(UserDefDataParser::semantic_type* yylval,  UserDefDataParser::location_type* yylloc, UserDefDataParseContext* ctx)
{
    auto lexer = ctx->lexer;
    auto tk = lexer->userdefdatalex();
    switch(tk)
    {
        case UserDefDataParser::token_type::TK_TRUE:
            yylval->boolean = true;
            break;
        case UserDefDataParser::token_type::TK_FALSE:
            yylval->boolean = false;
            break;
        case UserDefDataParser::token_type::TK_NUM:
            yylval->num = std::stof(lexer->GetWString());
            break;
        case UserDefDataParser::token_type::TK_STR:
            ctx->tmpStr = lexer->GetWString();
            break;
    }
    yylloc->begin = lexer->GetSourcePos();
    return tk;
}

void UserDefDataParser::error(const UserDefDataParser::location_type& yylloc, const std::string &msg) {
    throw Log(LogLevel::LV_ERROR)
      .Msg(msg)
      .AddSourcePos(std::make_shared<SourcePos>(yylloc.begin));
}
}

%union
{
    int id;
    int blend;
    int filter;
    bool boolean;
    float num;
    Int3 rgb;
    Int4 rect;
    Int5 anim_clip;
    ShotCollision collision;
}

/* header */
%token TK_USER_SHOT_DATA "#UserShotData"
%token TK_USER_ITEM_DATA "#UserItemData"

/* struct */
%token TK_ST_SHOT_DATA "ShotData"
%token TK_ST_ITEM_DATA "ItemData"
%token TK_ST_ANIMATION_DATA "AnimationData"

/* param */
%token TK_P_SHOT_IMAGE "shot_image"
%token TK_P_ITEM_IMAGE "item_image"
%token TK_P_ID "id"
%token TK_P_RECT "rect"
%token TK_P_RENDER "render"
%token TK_P_FILTER "filter"
%token TK_P_ALPHA "alpha"
%token TK_P_DELAY_RECT "delay_rect"
%token TK_P_DELAY_COLOR "delay_color"
%token TK_P_DELAY_RENDER "delay_render"
%token TK_P_FADE_RECT "fade_rect"
%token TK_P_ANGULAR_VELOCITY "angular_velocity"
%token TK_P_FIXED_ANGLE "fixed_angle"
%token TK_P_COLLISION "collision"
%token TK_P_ANIMATION_DATA "animation_data"
%token TK_P_TYPE "type"
%token TK_P_OUT "out"

/* blend mode */
%token TK_B_ALPHA "ALPHA"
%token TK_B_ADD_RGB "ADD"
%token TK_B_ADD_ARGB "ADD_ARGB"
%token TK_B_MULTIPLY "MULTIPLY"
%token TK_B_SUBTRACT "SUBTRACT"
%token TK_B_INV_DESTRGB "INV_DESTRGB"

/* FP FILTER mode */
%token TK_F_NONE "NONE"
%token TK_F_LINEAR "LINEAR"

/* operator */
%token TK_RAND "rand"
%token TK_EQ "="
%token TK_PLUS "+"
%token TK_MINUS "-"

/* separator */
%token TK_LPAREN "("
%token TK_RPAREN ")"
%token TK_LBRACE "{"
%token TK_RBRACE "}"
%token TK_COMMA ","
%token TK_SEMI ";"

/* literal */
%token <boolean> TK_TRUE "true"
%token <boolean> TK_FALSE "false"
%token <num> TK_NUM "<number>"
%token TK_STR "<string>"

%token TK_EOF 0 "end of file"

%right UPLUS UMINUS

%type <id> id
%type <num> num
%type <boolean> bool
%type <blend> blend-type
%type <filter> filter-type
%type <rgb> rgb
%type <rect> rect
%type <collision> collision
%type <anim_clip> animation-clip animation-data-struct-param

%start data

%%
data                         : stmts TK_EOF

stmts                        : none
                             | stmts stmt

stmt                         : shot-data-struct
                             {
                               if (ctx->userShotData)
                               {
                                   ctx->userShotData->dataMap[ctx->shotData.id] = ctx->shotData;
                               }
                             }
                             | item-data-struct
                             {
                                 if (ctx->userItemData)
                                 {
                                     if (ctx->itemData.type == ID_INVALID)
                                     {
                                         ctx->itemData.type = ctx->itemData.id;
                                     }
                                     ctx->userItemData->dataMap[ctx->itemData.id] = ctx->itemData;
                                 }
                             }
                             | TK_P_SHOT_IMAGE  TK_EQ TK_STR { if (ctx->userShotData) { ctx->userShotData->imagePath = ctx->tmpStr; } }
                             | TK_P_DELAY_RECT  TK_EQ rect   { if (ctx->userShotData) { ctx->userShotData->delayRect = Rect<int>($3.a, $3.b, $3.c, $3.d); } }
                             | TK_P_DELAY_COLOR TK_EQ rgb    { if (ctx->userShotData) { ctx->userShotData->delayColor = ColorRGB($3.a, $3.b, $3.c); } }
                             | TK_P_ITEM_IMAGE  TK_EQ TK_STR { if (ctx->userItemData) { ctx->userItemData->imagePath = ctx->tmpStr; } }
                             | TK_USER_SHOT_DATA
                             | TK_USER_ITEM_DATA
                             | TK_SEMI

shot-data-struct               : { ctx->shotData = ShotData(); } TK_ST_SHOT_DATA TK_LBRACE shot-data-struct-params TK_RBRACE
shot-data-struct-params        : none
                               | shot-data-struct-params shot-data-struct-param

shot-data-struct-param         : TK_P_ID                TK_EQ id         { ctx->shotData.id = $3; }
                               | TK_P_RECT              TK_EQ rect       { ctx->shotData.rect = Rect<int>($3.a, $3.b, $3.c, $3.d); }
                               | TK_P_RENDER            TK_EQ blend-type { ctx->shotData.render = $3; }
                               | TK_P_FILTER            TK_EQ filter-type { ctx->shotData.filter = $3; }
							   | TK_P_ALPHA             TK_EQ num        { ctx->shotData.alpha = std::min(std::max((int)$3, 0), 0xff); }
                               | TK_P_DELAY_RECT        TK_EQ rect       { ctx->shotData.delayRect = Rect<int>($3.a, $3.b, $3.c, $3.d); ctx->shotData.useDelayRect = true;}
                               | TK_P_DELAY_COLOR       TK_EQ rgb        { ctx->shotData.delayColor = ColorRGB($3.a, $3.b, $3.c); ctx->shotData.useDelayColor = true; }
                               | TK_P_DELAY_RENDER      TK_EQ blend-type { ctx->shotData.delayRender = $3; }
                               | TK_P_FADE_RECT         TK_EQ rect       { ctx->shotData.fadeRect = Rect<int>($3.a, $3.b, $3.c, $3.d); ctx->shotData.useFadeRect = true;}
                               | TK_P_ANGULAR_VELOCITY  TK_EQ num        { ctx->shotData.angularVelocity = $3; ctx->shotData.useAngularVelocityRand = false; }
                               | TK_P_ANGULAR_VELOCITY  TK_EQ TK_RAND TK_LPAREN num TK_COMMA num TK_RPAREN
                                                                         { ctx->shotData.useAngularVelocityRand = true; ctx->shotData.angularVelocityRandMin = $5; ctx->shotData.angularVelocityRandMax = $7; }
                               | TK_P_FIXED_ANGLE       TK_EQ bool       { ctx->shotData.fixedAngle = $3; }
                               | TK_P_COLLISION         TK_EQ collision  { ctx->shotData.collisions.push_back($3); }
                               | animation-data-struct                   { ctx->shotData.animationData = ctx->animationData; }
                               | TK_SEMI

item-data-struct             : { ctx->itemData = ItemData(); } TK_ST_ITEM_DATA TK_LBRACE item-data-struct-params TK_RBRACE
item-data-struct-params      : none
                             | item-data-struct-params item-data-struct-param
item-data-struct-param       : TK_P_ID                TK_EQ id          { ctx->itemData.id = $3; }
                             | TK_P_TYPE              TK_EQ id          { ctx->itemData.type = $3; }
                             | TK_P_RECT              TK_EQ rect        { ctx->itemData.rect = Rect<int>($3.a, $3.b, $3.c, $3. d); }
                             | TK_P_OUT               TK_EQ rect        { ctx->itemData.out = Rect<int>($3.a, $3.b, $3.c, $3. d); }
                             | TK_P_RENDER            TK_EQ blend-type  { ctx->itemData.render = $3; }
                             | TK_P_FILTER            TK_EQ filter-type { ctx->itemData.filter = $3; }
                             | animation-data-struct                    { ctx->itemData.animationData = ctx->animationData; }
                             | TK_SEMI

animation-data-struct          : { ctx->animationData.clear(); } TK_ST_ANIMATION_DATA TK_LBRACE animation-data-struct-params TK_RBRACE

animation-data-struct-params   : none
                               | animation-data-struct-params animation-data-struct-param
                                   { ctx->animationData.emplace_back($2.a, $2.b, $2.c, $2.d, $2.e); }

animation-data-struct-param    : TK_P_ANIMATION_DATA TK_EQ animation-clip opt-semi { $$ = $3; }

animation-clip                 : TK_LPAREN num TK_COMMA num TK_COMMA num TK_COMMA num TK_COMMA num TK_RPAREN
                                   { $$ = {(int)$2, (int)$4, (int)$6, (int)$8, (int)$10}; }

blend-type                     : TK_B_ALPHA       { $$ = BLEND_ALPHA; }
                               | TK_B_ADD_RGB     { $$ = BLEND_ADD_RGB; }
                               | TK_B_ADD_ARGB    { $$ = BLEND_ADD_ARGB; }
                               | TK_B_MULTIPLY    { $$ = BLEND_MULTIPLY; }
                               | TK_B_SUBTRACT    { $$ = BLEND_SUBTRACT; }
                               | TK_B_INV_DESTRGB { $$ = BLEND_INV_DESTRGB; }

filter-type                    : TK_F_NONE       { $$ = FILTER_NONE; }
                               | TK_F_LINEAR     { $$ = FILTER_LINEAR; }

bool                           : TK_TRUE | TK_FALSE

rect                           : TK_LPAREN num TK_COMMA num TK_COMMA num TK_COMMA num TK_RPAREN { $$ = {(int)$2, (int)$4, (int)$6, (int)$8}; }

rgb                            : TK_LPAREN num TK_COMMA num TK_COMMA num TK_RPAREN { $$ = {(int)$2, (int)$4, (int)$6}; }

collision                      : num 
                                   { $$ = { $1, 0, 0 }; }
                               | TK_LPAREN num TK_COMMA num TK_COMMA num TK_RPAREN
                                   { $$ = { $2, $4, $6 }; }

num                            : TK_PLUS num  %prec UPLUS  { $$ = $2; }
                               | TK_MINUS num %prec UMINUS { $$ = -$2; }
                               | TK_NUM

id                             : num { $$ = (int)$1; }

opt-semi                       : none
                               | TK_SEMI

none                           :

%%
