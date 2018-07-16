%option fast
%option unicode
%option namespace=bstorm
%option lexer=DnhLexer
%option prefix=dnh
%option dotall

%top {
#include <bstorm/file_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/file_loader.hpp>

#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <set>
%}

%class {
public :
    wchar_t GetWChar() const { return v_wchar_; }
    std::wstring GetWString() const { return v_wstr_; }
    std::string GetString() const { return v_str_; }
    std::shared_ptr<std::wstring> GetCurrentFilePath() const
    {
        if (includeStack_.empty()) return std::make_shared<std::wstring>(L"");
        return includeStack_.back();
    }
    SourcePos GetSourcePos() const
    {
        return SourcePos((int)lineno(), (int)columno() + 1, GetCurrentFilePath());
    }
    void SetLoader(const std::shared_ptr<FileLoader>& loader)
    {
        this->loader_ = loader;
    }
    void PushInclude(const std::wstring& path)
    {
        std::shared_ptr<std::wstring> includePath;
        if (!includeStack_.empty())
        {
            includePath = std::make_shared<std::wstring>(ExpandIncludePath(*GetCurrentFilePath(), path));
        } else
        {
            includePath = std::make_shared<std::wstring>(GetCanonicalPath(path));
        }
        if (visitedFilePaths_.count(*includePath) != 0)
        {
            return;
        }
        std::FILE* fp = loader_->OpenFile(*includePath);
        if (fp == nullptr)
        {
            if (includeStack_.empty())
            {
                throw Log(Log::Level::LV_ERROR)
                  .SetMessage("can't open file.")
                  .SetParam(Log::Param(Log::Param::Tag::TEXT, path));
            } else
            {
                throw Log(Log::Level::LV_ERROR)
                  .SetMessage("can't include file.")
                  .SetParam(Log::Param(Log::Param::Tag::TEXT, path))
                  .AddSourcePos(std::make_shared<SourcePos>(GetSourcePos()));
            }
        }
        visitedFilePaths_.insert(*includePath);
        if (includeStack_.empty()) 
        {
            in(fp);
        } else {
            push_matcher(new_matcher(fp));
        }
        includeStack_.push_back(includePath);
    }
    void PopInclude()
    {
        if (!includeStack_.empty())
        {
            loader_->CloseFile(*GetCurrentFilePath(), in());
            if (includeStack_.size() > 1)
            {
                pop_matcher();
            }
            includeStack_.pop_back();
        }
    }
    size_t GetIncludeStackSize() const 
    {
        return includeStack_.size();
    }
    ~DnhLexer()
    {
      while (!includeStack_.empty())
      {
          PopInclude();
      }
    }
private :
    wchar_t v_wchar_;
    std::wstring v_wstr_;
    std::string v_str_;
    std::vector<std::shared_ptr<std::wstring>> includeStack_;
    std::shared_ptr<FileLoader> loader_;
    std::set<std::wstring> visitedFilePaths_;
%}

%{
#include "../bison/dnh.tab.hpp"
using tk = bstorm::DnhParser::token;
%}

white [ \t\r\n\f]+
ident [A-Za-z_][A-Za-z0-9_]*
number ([0-9]+(\.[0-9]+)?)|(0x[0-9a-fA-F]+)
newline \r?\n

%x LINE_COMMENT
%x STRING_LIT

%%
"/*".*?"*/"
"//" { start(LINE_COMMENT); }

<LINE_COMMENT>{
{newline} { start(INITIAL); }
<<EOF>> {
    start(INITIAL);
    if (includeStack_.size() <= 1)
    {
        return tk::TK_EOF;
    }
    PopInclude();
}
.
}

{white}
"alternative" { return(tk::TK_ALTERNATIVE); }
"ascent" { return(tk::TK_ASCENT); }
"break" { return (tk::TK_BREAK); }
"case" { return(tk::TK_CASE); }
"descent" { return(tk::TK_DESCENT); }
"else" { return(tk::TK_ELSE); }
"function" { return(tk::TK_FUNCTION); }
"if" { return(tk::TK_IF); }
"in" { return(tk::TK_IN); }
"local" { return(tk::TK_LOCAL); }
"loop" { return(tk::TK_LOOP); }
"real" { return(tk::TK_REAL); }
"var" { return(tk::TK_VAR); }
"return" { return(tk::TK_RETURN); }
"let" { return(tk::TK_LET); }
"others" { return(tk::TK_OTHERS); }
"sub" { return(tk::TK_SUB); }
"task" { return(tk::TK_TASK); }
"times" { return(tk::TK_TIMES); }
"while" { return(tk::TK_WHILE); }
"yield" { return(tk::TK_YIELD); }

"@" { return(tk::TK_ATMARK); }

"+" { return(tk::TK_PLUS); }
"-" { return(tk::TK_MINUS); }
"*" { return(tk::TK_MUL); }
"/" { return(tk::TK_DIV); }
"%" { return(tk::TK_REM); }
"^" { return(tk::TK_POW); }
"~" { return(tk::TK_CAT); }
"++" { return(tk::TK_SUCC); }
"--" { return(tk::TK_PRED); }
".." { return(tk::TK_DOTDOT); }
"=" { return(tk::TK_ASSIGN); }
"+=" { return(tk::TK_ADDASSIGN); }
"-=" { return(tk::TK_SUBASSIGN); }
"*=" { return(tk::TK_MULASSIGN); }
"/=" { return(tk::TK_DIVASSIGN); }
"%=" { return(tk::TK_REMASSIGN); }
"^=" { return(tk::TK_POWASSIGN); }
"~=" { return tk::TK_CATASSIGN; }

"<" { return(tk::TK_LT); }
">" { return(tk::TK_GT); }
"<=" { return(tk::TK_LE); }
">=" { return(tk::TK_GE); }
"==" { return(tk::TK_EQ); }
"!=" { return(tk::TK_NE); }
"&&" { return(tk::TK_AND); }
"||" { return(tk::TK_OR); }
"!" { return(tk::TK_NOT); }

";" { return(tk::TK_SEMI); }
"," { return(tk::TK_COMMA); }

"(|" { return(tk::TK_LABSPAREN); }
"|)" { return(tk::TK_RABSPAREN); }
")" { return(tk::TK_RPAREN); }
"(" { return(tk::TK_LPAREN); }
"[" { return(tk::TK_LBRACKET); }
"]" { return(tk::TK_RBRACKET); }
"{" { return(tk::TK_LBRACE); }
"}" { return(tk::TK_RBRACE); }

{number} { v_str_ = str(); return tk::TK_NUM; }

'\\.' { v_wchar_ = (wstr())[2]; return tk::TK_CHAR; }
'.' { v_wchar_ = (wstr())[1]; return tk::TK_CHAR; }

\"  { v_wstr_.clear(); start(STRING_LIT); }

<STRING_LIT>{
\" { start(INITIAL); return tk::TK_STR; }
[\\][\\] { v_wstr_ += wstr(); }
\\\" { v_wstr_ += (wstr())[1]; }
. { v_wstr_ += wstr(); }
}

{ident} { v_str_ = str(); return tk::TK_IDENT; }

"#"{2,}"東方弾幕風" { return tk::TK_IGNORED_HEADER; }
"#"{2,}{ident} { return tk::TK_IGNORED_HEADER; }
"#東方弾幕風" { v_wstr_ = L"TouhouDanmakufu"; return tk::TK_HEADER; }
"#"{ident} { v_wstr_ = wstr().substr(1); return tk::TK_HEADER; }

.  {
    throw Log(Log::Level::LV_ERROR).SetMessage("found illegal token.")
      .SetParam(Log::Param(Log::Param::Tag::TEXT, str()))
      .AddSourcePos(std::make_shared<SourcePos>(GetSourcePos()));
}

<<EOF>> {
  if (includeStack_.size() <= 1)
  {
      return tk::TK_EOF;
  }
  PopInclude();
}
%%
