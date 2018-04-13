%skeleton "lalr1.cc"
%require "3.0"

%define api.namespace {bstorm}
%define parser_class_name {MqoParser}
%locations
%define api.location.type {bstorm::SourceLoc}
%define parse.error verbose

%parse-param { bstorm::MqoParseContext* ctx }
%lex-param { bstorm::MqoParseContext* ctx }

%code requires
{ // top mqo.tab.hpp
#include <bstorm/mqo.hpp>
#include <bstorm/source_map.hpp>

#include <string>
#include <memory>

namespace bstorm
{
class MqoLexer;
struct MqoParseContext
{
    MqoParseContext(MqoLexer* lexer) : mqo(std::make_shared<Mqo>()), lexer(lexer) {}
    std::shared_ptr<Mqo> mqo;
    MqoLight light;
    MqoScene scene;
    MqoMaterial material;
    MqoFace face;
    MqoObject object;
    std::wstring tmpStr;
    MqoLexer* lexer;
};
}
}

%code
{ // mqo.tab.cpp after #include mqo.tab.hpp
#include <bstorm/logger.hpp>
#include <bstorm/util.hpp>

#include "../reflex/mqo_lexer.hpp"

using namespace bstorm;

static int yylex(MqoParser::semantic_type *yylval, MqoParser::location_type* yylloc, MqoParseContext* ctx)
{
    auto lexer = ctx->lexer;
    auto tk = lexer->mqolex();
    switch(tk)
    {
        case MqoParser::token_type::TK_NUM:
            yylval->num = lexer->getNumber();
            break;
        case MqoParser::token_type::TK_STR:
            ctx->tmpStr = lexer->getWString();
            break;
    }
    yylloc->begin.filename = lexer->getFilePath();
    yylloc->begin.line = lexer->lineno();
    yylloc->begin.column = lexer->columno() + 1;
    return tk;
}

void MqoParser::error(const MqoParser::location_type& yylloc, const std::string& msg)
{
    throw Log(Log::Level::LV_ERROR)
      .setMessage(msg)
      .addSourcePos(std::make_shared<SourcePos>(yylloc.begin));
}

static MqoColor4 toMqoColor4(uint32_t col)
{
    float r = (col & 0x000000ff) / 255.0f;
    float g = (col & 0x0000ff00) / 255.0f;
    float b = (col & 0x00ff0000) / 255.0f;
    float a = (col & 0xff000000) / 255.0f;
    return MqoColor4{r, g, b, a};
}
}

%union
{
    double num;
    MqoVec2 vec2;
    MqoVec3 vec3;
    MqoColor4 col4;
}

/* keyword */
// _U_ : upper case

// Scene
%token TK_SCENE "Scene"
%token TK_POS "pos"
%token TK_LOOKAT "lookat"
%token TK_HEAD "head"
%token TK_PICH "pich"
%token TK_BANK "bank"
%token TK_ORTHO "ortho"
%token TK_ZOOM2 "zoom2"
%token TK_AMB "amb"
%token TK_FRONTCLIP "frontclip"
%token TK_BACKCLIP "backclip"

// Light
%token TK_DIRLIGHTS "dirlights"
%token TK_LIGHT "light"
%token TK_DIR "dir"

// Material
%token TK_MATERIAL "Material"
%token TK_SHADER "shader"
%token TK_VCOL "vcol"
%token TK_DBLS "dbls"
%token TK_COL "col"
%token TK_DIF "dif"
%token TK_EMI "emi"
%token TK_SPC "spc"
%token TK_POWER "power"
%token TK_REFLECT "reflect"
%token TK_REFRACT "refract"
%token TK_TEX "tex"
%token TK_APLANE "aplane"
%token TK_BUMP "bump"
%token TK_PROJ_TYPE "proj_type"
%token TK_PROJ_POS "proj_pos"
%token TK_PROJ_SCALE "proj_scale"
%token TK_PROJ_ANGLE "proj_angle"

// Vertex
%token TK_VERTEX "vertex"

// Face
%token TK_FACE "face"
%token TK_U_V "V"
%token TK_U_M "M"
%token TK_U_UV "UV"
%token TK_U_COL "COL"
%token TK_U_CRS "CRS"

// Object
%token TK_OBJECT "Object"
%token TK_UID "uid"
%token TK_DEPTH "depth"
%token TK_FOLDING "folding"
%token TK_SCALE "scale"
%token TK_ROTATION "rotation"
%token TK_TRANSLATION "translation"
%token TK_PATCH "patch"
%token TK_PATCHTRI "patchtri"
%token TK_SEGMENT "segment"
%token TK_VISIBLE "visible"
%token TK_LOCKING "locking"
%token TK_SHADING "shading"
%token TK_FACET "facet"
%token TK_COLOR "color"
%token TK_COLOR_TYPE "color_type"
%token TK_MIRROR "mirror"
%token TK_MIRROR_AXIS "mirror_axis"
%token TK_MIRROR_DIS "mirror_dis"
%token TK_LATHE "lathe"
%token TK_LATHE_AXIS "lathe_axis"
%token TK_LATHE_SEG "lathe_seg"

/* operator */
%token TK_PLUS "+"
%token TK_MINUS "-"

/* separator */
%token TK_NEWLINE "newline"

/* paren */
%token TK_LPAREN "("
%token TK_RPAREN ")"
%token TK_LBRACKET "["
%token TK_RBRACKET "]"
%token TK_LBRACE "{"
%token TK_RBRACE "}"

/* literal */
%token <num> TK_NUM "<number>"
%token TK_IDENT "<identifier>"
%token TK_STR "<string>"

%token TK_EOF 0 "end of file"

%right UPLUS UMINUS

%type <num> num
%type <vec2> vec2
%type <vec3> vec3
%type <col4> col4

%start mqo

%%
mqo      : header chunks TK_EOF

header   : TK_IDENT TK_IDENT TK_NEWLINE TK_IDENT TK_IDENT TK_IDENT num TK_NEWLINE { ctx->mqo->version = $7; }

chunks   : chunks chunk
         | none

chunk    : scene-chunk
         | material-chunk
         | object-chunk
         | skip-unknown-chunk
         | TK_NEWLINE

skip-unknown-chunk : TK_IDENT skip-values TK_LBRACE skip-values-contain-newline TK_RBRACE
                   | TK_IDENT skip-values TK_NEWLINE
skip-values-contain-newline : skip-values-contain-newline skip-value-contain-newline
                            | none
skip-value-contain-newline : skip-value | TK_NEWLINE
skip-values : skip-values skip-value
            | none
skip-value  : TK_NUM | TK_STR | TK_IDENT

dirlights-chunk : TK_DIRLIGHTS skip-values TK_LBRACE lights TK_RBRACE
lights          : lights light
                | none
light           : { ctx->light = MqoLight(); } TK_LIGHT TK_LBRACE light-props TK_RBRACE { ctx->scene.dirlights.push_back(ctx->light); }
                | TK_NEWLINE
light-props     : light-props light-prop
                | none
light-prop      : TK_DIR vec3 { ctx->light.dir = $2; }
                | TK_COLOR vec3 { ctx->light.color = MqoColor4{$2.x, $2.y, $2.z, 1.0f}; }
                | skip-unknown-chunk
                | TK_NEWLINE

scene-chunk       : { ctx->scene = MqoScene(); } TK_SCENE TK_LBRACE scene-chunk-props TK_RBRACE { ctx->mqo->scene = ctx->scene; }
scene-chunk-props : scene-chunk-props scene-chunk-prop
                  | none
scene-chunk-prop  : TK_POS vec3 { ctx->scene.pos = $2; }
                  | TK_LOOKAT vec3 { ctx->scene.lookat = $2; }
                  | TK_HEAD num { ctx->scene.head = $2; }
                  | TK_PICH num { ctx->scene.pich = $2; }
                  | TK_BANK num { ctx->scene.bank = $2; }
                  | TK_ORTHO num { ctx->scene.ortho = (int)$2; }
                  | TK_ZOOM2 num { ctx->scene.zoom2 = $2; }
                  | TK_AMB vec3 { ctx->scene.amb = MqoColor4{$2.x, $2.y, $2.z, 1.0f}; }
                  | TK_FRONTCLIP num { ctx->scene.frontclip = $2; }
                  | TK_BACKCLIP num { ctx->scene.backclip = $2; }
                  | skip-unknown-chunk
                  | TK_NEWLINE
                  | dirlights-chunk

material-chunk : TK_MATERIAL skip-values TK_LBRACE materials TK_RBRACE
materials      : materials material
               | none
material       : { ctx->material = MqoMaterial(); } TK_STR { ctx->material.name = ctx->tmpStr; } material-props TK_NEWLINE { ctx->mqo->materials.push_back(ctx->material); }
               | TK_NEWLINE
material-props : material-props material-prop
               | none
material-prop  : TK_SHADER TK_LPAREN num TK_RPAREN { ctx->material.shader = (int)$3; }
               | TK_VCOL TK_LPAREN num TK_RPAREN { ctx->material.vcol = $3 == 1; }
               | TK_DBLS TK_LPAREN num TK_RPAREN { ctx->material.dbls = $3 == 1; }
               | TK_COL TK_LPAREN col4 TK_RPAREN { ctx->material.col = $3; }
               | TK_DIF TK_LPAREN num TK_RPAREN { ctx->material.dif = $3; }
               | TK_AMB TK_LPAREN num TK_RPAREN { ctx->material.amb = $3; }
               | TK_EMI TK_LPAREN num TK_RPAREN { ctx->material.emi = $3; }
               | TK_SPC TK_LPAREN num TK_RPAREN { ctx->material.spc = $3; }
               | TK_POWER TK_LPAREN num TK_RPAREN { ctx->material.power = $3; }
               | TK_REFLECT TK_LPAREN num TK_RPAREN { ctx->material.reflect = $3; }
               | TK_REFRACT TK_LPAREN num TK_RPAREN { ctx->material.refract = $3; }
               | TK_TEX TK_LPAREN TK_STR TK_RPAREN { ctx->material.tex = ctx->tmpStr; }
               | TK_APLANE TK_LPAREN TK_STR TK_RPAREN { ctx->material.aplane = ctx->tmpStr; }
               | TK_BUMP TK_LPAREN TK_STR TK_RPAREN { ctx->material.bump = ctx->tmpStr; }
               | TK_PROJ_TYPE TK_LPAREN num TK_RPAREN { ctx->material.proj_type = (int)$3; }
               | TK_PROJ_POS TK_LPAREN vec3 TK_RPAREN { ctx->material.proj_pos = $3; }
               | TK_PROJ_SCALE TK_LPAREN vec3 TK_RPAREN { ctx->material.proj_scale = $3; }
               | TK_PROJ_ANGLE TK_LPAREN vec3 TK_RPAREN { ctx->material.proj_angle = $3; }
               | TK_IDENT TK_LPAREN skip-values TK_RPAREN // skip unknown

vertex-chunk        : TK_VERTEX skip-values TK_LBRACE vertices TK_RBRACE
vertices            : vertices vertex
                    | none
vertex              : vec3 { ctx->object.vertices.push_back($1); }
                    | TK_NEWLINE

face-chunk          : TK_FACE skip-values TK_LBRACE faces TK_RBRACE
faces               : faces face
                    | none
face                : { ctx->face = MqoFace(); } num face-props { ctx->object.faces.push_back(ctx->face); }
                    | TK_NEWLINE
face-props          : face-props face-prop
                    | none
face-prop           : TK_U_V TK_LPAREN face-vertex-indices TK_RPAREN
                    | TK_U_M TK_LPAREN num TK_RPAREN { ctx->face.materialIndex = (int)$3; }
                    | TK_U_UV TK_LPAREN face-uvs TK_RPAREN
                    | TK_U_COL TK_LPAREN face-cols TK_RPAREN
                    | TK_U_CRS TK_LPAREN face-crss TK_RPAREN
                    | TK_IDENT TK_LPAREN skip-values TK_RPAREN // skip unknown
face-vertex-indices : face-vertex-indices face-vertex-index
                    | none
face-vertex-index   : num { ctx->face.vertexIndices.push_back((int)$1); }
face-uvs            : face-uvs face-uv
                    | none
face-uv             : vec2 { ctx->face.uvs.push_back($1); }
face-cols           : face-cols face-col
                    | none
face-col            : num { ctx->face.cols.push_back(toMqoColor4($1)); }
face-crss           : face-crss face-crs
                    | none
face-crs            : num { ctx->face.crss.push_back($1); }

object-chunk        : { ctx->object = MqoObject(); } TK_OBJECT TK_STR { ctx->object.name = ctx->tmpStr; } TK_LBRACE object-chunk-params TK_RBRACE { ctx->mqo->objects.push_back(ctx->object); }
object-chunk-params : object-chunk-params object-chunk-param
                    | none
object-chunk-param  : TK_DEPTH num { ctx->object.depth = (int)$2; }
                    | TK_FOLDING num { ctx->object.folding = $2 == 1; }
                    | TK_SCALE vec3 { ctx->object.scale = $2; }
                    | TK_ROTATION vec3 { ctx->object.rotation = $2; }
                    | TK_TRANSLATION vec3 { ctx->object.translation = $2; }
                    | TK_PATCH num { ctx->object.patch = (int)$2; }
                    | TK_PATCHTRI num { ctx->object.patchtri = (int)$2; }
                    | TK_SEGMENT num { ctx->object.segment = (int)$2; }
                    | TK_VISIBLE num { ctx->object.visible = $2 == 15; }
                    | TK_LOCKING num { ctx->object.locking = $2 == 1; }
                    | TK_SHADING num { ctx->object.shading = (int)$2; }
                    | TK_FACET num { ctx->object.facet = $2; }
                    | TK_COLOR vec3 { ctx->object.color = MqoColor4{$2.x, $2.y, $2.z, 1.0f}; }
                    | TK_COLOR_TYPE num { ctx->object.color_type = (int)$2; }
                    | TK_MIRROR num { ctx->object.mirror = (int)$2; }
                    | TK_MIRROR_AXIS num { ctx->object.mirror_axis = (int)$2; }
                    | TK_MIRROR_DIS num { ctx->object.mirror_dis = $2; }
                    | TK_LATHE num { ctx->object.lathe = (int)$2; }
                    | TK_LATHE_AXIS num { ctx->object.lathe_axis = (int)$2; }
                    | TK_LATHE_SEG num { ctx->object.lathe_seg = (int)$2; }
                    | vertex-chunk
                    | face-chunk
                    | skip-unknown-chunk
                    | TK_NEWLINE

vec2 : num num { $$ = MqoVec2{(float)$1, (float)$2}; } 
vec3 : num num num { $$ = MqoVec3{(float)$1, (float)$2, (float)$3}; } 
col4 : num num num num { $$ = MqoColor4{(float)$1, (float)$2, (float)$3, (float)$4}; } 

num  : TK_PLUS num %prec UPLUS { $$ = $2; }
     | TK_MINUS num %prec UMINUS { $$ = -$2; }
     | TK_NUM

none :

%%
