%option unicode
%option namespace=bstorm
%option lexer=UserDefDataLexer
%option prefix=userdefdata
%option dotall

%top{
#include <string>
#include <memory>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/file_loader.hpp>
%}

%init {
 fp = NULL;
%}

%class{
public :
  void setLoader(const std::shared_ptr<FileLoader>& loader) {
    this->loader = loader;
  }
  void setInputSource(const std::wstring& path) {
    filePath = std::make_shared<std::wstring>(canonicalPath(path));
    fp = loader->openFile(*filePath);
    if (fp == NULL) {
      throw Log(Log::Level::LV_ERROR).setMessage("can't open file.")
        .setParam(Log::Param(Log::Param::Tag::TEXT, path));
    }
    in(fp);
  }
  std::shared_ptr<std::wstring> getFilePath() const {
    if (filePath) return filePath;
    return std::make_shared<std::wstring>(L"");
  }

  SourcePos getSourcePos() const {
    return {(int)lineno(), (int)columno() + 1, getFilePath()};
  }
  std::wstring getWString() const {
    return v_wstr;
  }

  ~UserDefDataLexer() {
    if (fp) {
      loader->closeFile(*getFilePath(), fp);
    }
  }
protected :
  std::wstring v_wstr;
  std::shared_ptr<std::wstring> filePath;
  FILE* fp;
  std::shared_ptr<FileLoader> loader;
%}

%{
#include "../bison/user_def_data.tab.hpp"

using tk = bstorm::UserDefDataParser::token;
%}

white [ \t\r\n\f]+
number [0-9]+(\.[0-9]+)?
newline \r?\n

%x LINE_COMMENT
%x STRING_LIT

%%
"/*".*?"*/"
"//" { start(LINE_COMMENT); }

<LINE_COMMENT>{
{newline} { start(INITIAL); }
<<EOF>> { return tk::TK_EOF; }
.
}

{white}
"#UserShotData"    { return tk::TK_USER_SHOT_DATA; }
"#UserItemData"    { return tk::TK_USER_ITEM_DATA; }
"ShotData"         { return tk::TK_ST_SHOT_DATA; }
"ItemData"         { return tk::TK_ST_ITEM_DATA; }
"AnimationData"    { return tk::TK_ST_ANIMATION_DATA; }

"shot_image"       { return tk::TK_P_SHOT_IMAGE; }
"item_image"       { return tk::TK_P_ITEM_IMAGE; }
"id"               { return tk::TK_P_ID; }
"rect"             { return tk::TK_P_RECT; }
"render"           { return tk::TK_P_RENDER; }
"alpha"            { return tk::TK_P_ALPHA; }
"delay_rect"       { return tk::TK_P_DELAY_RECT; }
"delay_color"      { return tk::TK_P_DELAY_COLOR; }
"delay_render"     { return tk::TK_P_DELAY_RENDER; }
"angular_velocity" { return tk::TK_P_ANGULAR_VELOCITY; }
"fixed_angle"      { return tk::TK_P_FIXED_ANGLE; }
"collision"        { return tk::TK_P_COLLISION; }
"animation_data"   { return tk::TK_P_ANIMATION_DATA; }
"type"             { return tk::TK_P_TYPE; }
"out"              { return tk::TK_P_OUT; }

"ALPHA"            { return tk::TK_B_ALPHA; }
"ADD"("_RGB")?     { return tk::TK_B_ADD_RGB; }
"ADD_ARGB"         { return tk::TK_B_ADD_ARGB; }
"MULTIPLY"         { return tk::TK_B_MULTIPLY; }
"SUBTRACT"         { return tk::TK_B_SUBTRACT; }
"INV_DESTRGB"      { return tk::TK_B_INV_DESTRGB; }

"rand"             { return tk::TK_RAND; }
"="                { return tk::TK_EQ; }
"+"                { return tk::TK_PLUS; }
"-"                { return tk::TK_MINUS; }

"("                { return tk::TK_LPAREN; }
")"                { return tk::TK_RPAREN; }
"{"                { return tk::TK_LBRACE; }
"}"                { return tk::TK_RBRACE; }
","                { return tk::TK_COMMA; }
";"                { return tk::TK_SEMI; }

"true"             { return tk::TK_TRUE; }
"false"            { return tk::TK_FALSE; }
{number}           { v_wstr = wstr(); return tk::TK_NUM; }
\"                 { v_wstr.clear(); start(STRING_LIT); }

<STRING_LIT>{
\" { start(INITIAL); return tk::TK_STR; }
[\\][\\] { v_wstr += wstr(); }
\\\" { v_wstr += (wstr())[1]; }
. { v_wstr += wstr(); }
}

. {
  throw Log(Log::Level::LV_ERROR).setMessage("found illegal token.")
    .setParam(Log::Param(Log::Param::Tag::TEXT, str()))
    .addSourcePos(std::make_shared<SourcePos>(getSourcePos()));
}

<<EOF>> {
  return tk::TK_EOF;
}
%%
