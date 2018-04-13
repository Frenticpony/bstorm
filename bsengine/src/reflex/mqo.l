%option unicode
%option namespace=bstorm
%option lexer=MqoLexer
%option prefix=mqo
%option dotall

%top{
#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/file_loader.hpp>

#include <string>
#include <vector>
#include <cstdio>
#include <memory>
%}

%class{
public :
    double getNumber() const { return v_num; }
    std::wstring getWString() const { return v_wstr; }
    std::string getString() const { return v_str; }
    void setLoader(const std::shared_ptr<FileLoader>& loader)
    {
        this->loader = loader;
    }
    void setInputSource(const std::wstring& path)
    {
        filePath = std::make_shared<std::wstring>(canonicalPath(path));
        fp = loader->openFile(*filePath);
        if (fp == NULL)
        {
            throw Log(Log::Level::LV_ERROR)
              .setMessage("can't open file")
              .setParam(Log::Param(Log::Param::Tag::TEXT, path));
        }
        in(fp);
    }
    std::shared_ptr<std::wstring> getFilePath() const
    {
        if (filePath) return filePath;
        return std::make_shared<std::wstring>(L"");
    }
    ~MqoLexer()
    {
        if (fp)
        {
            loader->closeFile(*getFilePath(), fp);
        }
    }
protected :
    FILE* fp;
    double v_num;
    std::wstring v_wstr;
    std::string v_str;
    std::shared_ptr<FileLoader> loader;
    std::shared_ptr<std::wstring> filePath;
%}

%{
#include "../bison/mqo.tab.hpp"
using tk = bstorm::MqoParser::token;
%}

// â¸çsÇÕwhiteÇ…ä‹ÇﬂÇ»Ç¢
white [ \t\r\f]+
ident [A-Za-z_][A-Za-z0-9_]*
number ([0-9]+(\.[0-9]+)?)
newline \r?\n

%x STRING_LIT

%%

{white}

"Scene"      { return tk::TK_SCENE; }
"pos"        { return tk::TK_POS; }
"lookat"     { return tk::TK_LOOKAT; }
"head"       { return tk::TK_HEAD; }
"pich"       { return tk::TK_PICH; }
"bank"       { return tk::TK_BANK; }
"ortho"      { return tk::TK_ORTHO; }
"zoom2"      { return tk::TK_ZOOM2; }
"amb"        { return tk::TK_AMB; }
"frontclip"  { return tk::TK_FRONTCLIP; }
"backclip"   { return tk::TK_BACKCLIP; }

"dirlights"  { return tk::TK_DIRLIGHTS; }
"light"      { return tk::TK_LIGHT; }
"dir"        { return tk::TK_DIR; }

"Material"   { return tk::TK_MATERIAL; }
"shader"     { return tk::TK_SHADER; }
"vcol"       { return tk::TK_VCOL; }
"dbls"       { return tk::TK_DBLS; }
"col"        { return tk::TK_COL; }
"dif"        { return tk::TK_DIF; }
"emi"        { return tk::TK_EMI; }
"spc"        { return tk::TK_SPC; }
"power"      { return tk::TK_POWER; }
"reflect"    { return tk::TK_REFLECT; }
"refract"    { return tk::TK_REFRACT; }
"tex"        { return tk::TK_TEX; }
"aplane"     { return tk::TK_APLANE; }
"bump"       { return tk::TK_BUMP; }
"proj_type"  { return tk::TK_PROJ_TYPE; }
"proj_pos"   { return tk::TK_PROJ_POS; }
"proj_scale" { return tk::TK_PROJ_SCALE; }
"proj_angle" { return tk::TK_PROJ_ANGLE; }

"vertex" { return tk::TK_VERTEX; }

"face" { return tk::TK_FACE; }
"V"    { return tk::TK_U_V; }
"M"    { return tk::TK_U_M; }
"UV"   { return tk::TK_U_UV; }
"COL"  { return tk::TK_U_COL; }
"CRS"  { return tk::TK_U_CRS; }

"Object"      { return tk::TK_OBJECT; }
"uid"         { return tk::TK_UID; }
"depth"       { return tk::TK_DEPTH; }
"folding"     { return tk::TK_FOLDING; }
"scale"       { return tk::TK_SCALE; }
"rotation"    { return tk::TK_ROTATION; }
"translation" { return tk::TK_TRANSLATION; }
"patch"       { return tk::TK_PATCH; }
"patchtri"    { return tk::TK_PATCHTRI; }
"segment"     { return tk::TK_SEGMENT; }
"visible"     { return tk::TK_VISIBLE; }
"locking"     { return tk::TK_LOCKING; }
"shading"     { return tk::TK_SHADING; }
"facet"       { return tk::TK_FACET; }
"color"       { return tk::TK_COLOR; }
"color_type"  { return tk::TK_COLOR_TYPE; }
"mirror"      { return tk::TK_MIRROR; }
"mirror_axis" { return tk::TK_MIRROR_AXIS; }
"mirror_dis"  { return tk::TK_MIRROR_DIS; }
"lathe"       { return tk::TK_LATHE; }
"lathe_axis"  { return tk::TK_LATHE_AXIS; }
"lathe_seg"   { return tk::TK_LATHE_SEG; }

"+"  { return tk::TK_PLUS; }
"-"  { return tk::TK_MINUS; }

{newline} { return tk::TK_NEWLINE; }

"(" { return tk::TK_LPAREN; }
")" { return tk::TK_RPAREN; }
"[" { return tk::TK_LBRACKET; }
"]" { return tk::TK_RBRACKET; }
"{" { return tk::TK_LBRACE; }
"}" { return tk::TK_RBRACE; }

{number} { v_num = std::atof(str().c_str()); return tk::TK_NUM; }

\"  { v_wstr.clear(); start(STRING_LIT); }

<STRING_LIT>{
\" { start(INITIAL); return tk::TK_STR; }
\\. { v_wstr += (wstr())[1]; }
. { v_wstr += wstr(); }
}

{ident} { v_str = str(); return tk::TK_IDENT; }

. {}

<<EOF>> { return tk::TK_EOF; }
%%
