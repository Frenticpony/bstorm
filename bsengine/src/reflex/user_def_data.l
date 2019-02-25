%option unicode
%option namespace=bstorm
%option lexer=UserDefDataLexer
%option prefix=userdefdata
%option dotall

%top{
#include <bstorm/file_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/file_loader.hpp>

#include <string>
#include <memory>
%}

%init {
 fp_ = NULL;
%}

%class{
public :
    void SetLoader(const std::shared_ptr<FileLoader>& loader)
    {
        this->loader_ = loader;
    }
    void SetInputSource(const std::wstring& path)
    {
        filePath_ = std::make_shared<std::wstring>(GetCanonicalPath(path));
        fp_ = loader_->OpenFile(*filePath_);
        if (fp_ == NULL)
        {
            throw Log(LogLevel::LV_ERROR)
              .Msg("Unable to open file. (File not found.)")
              .Param(LogParam(LogParam::Tag::TEXT, path));
        }
        in(fp_);
    }
    std::shared_ptr<std::wstring> getFilePath() const
    {
        if (filePath_) return filePath_;
        return std::make_shared<std::wstring>(L"");
    }
    SourcePos GetSourcePos() const
    {
        return SourcePos((int)lineno(), (int)columno() + 1, getFilePath());
    }
    std::wstring GetWString() const
    {
      return v_wstr_;
    }
    ~UserDefDataLexer()
    {
        if (fp_)
        {
            loader_->CloseFile(*getFilePath(), fp_);
        }
    }
private :
    std::wstring v_wstr_;
    std::shared_ptr<std::wstring> filePath_;
    FILE* fp_;
    std::shared_ptr<FileLoader> loader_;
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
"filter"           { return tk::TK_P_FILTER; }
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

"NONE"             { return tk::TK_F_NONE; }
"LINEAR"           { return tk::TK_F_LINEAR; }

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
{number}           { v_wstr_ = wstr(); return tk::TK_NUM; }
\"                 { v_wstr_.clear(); start(STRING_LIT); }

<STRING_LIT>{
\" { start(INITIAL); return tk::TK_STR; }
[\\][\\] { v_wstr_ += wstr(); }
\\\" { v_wstr_ += (wstr())[1]; }
. { v_wstr_ += wstr(); }
}

. {
    throw Log(LogLevel::LV_ERROR)
      .Msg("Found illegal token.")
      .Param(LogParam(LogParam::Tag::TEXT, str()))
      .AddSourcePos(std::make_shared<SourcePos>(GetSourcePos()));
}

<<EOF>> { return tk::TK_EOF; }
%%
