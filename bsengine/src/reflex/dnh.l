%option fast
%option unicode
%option namespace=bstorm
%option lexer=DnhLexer
%option prefix=dnh
%option dotall

%top{
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <set>

#include <bstorm/util.hpp>
#include <bstorm/static_script_error.hpp>
#include <bstorm/file_loader.hpp>
%}

%class{
public :
  wchar_t getWChar() const { return v_wchar; }
  std::wstring getWString() const { return v_wstr; }
  std::string getString() const { return v_str; }
  std::shared_ptr<std::wstring> getCurrentFilePath() const {
    if (includeStack.empty()) return std::make_shared<std::wstring>(L"");
    return includeStack.back();
  }
  void setLoader(const std::shared_ptr<FileLoader>& loader) {
    this->loader = loader;
  }
  void pushInclude(const std::wstring& path) {
    std::shared_ptr<std::wstring> includePath;
    if (!includeStack.empty()) {
      includePath = std::make_shared<std::wstring>(expandIncludePath(*getCurrentFilePath(), path));
    } else {
      includePath = std::make_shared<std::wstring>(canonicalPath(path));
    }
    if (visitedFilePaths.count(*includePath) != 0) {
      return;
    }
    std::FILE* fp = loader->openFile(*includePath);
    if (fp == NULL) {
      if (includeStack.empty()) {
        throw std::runtime_error("can't open file: " + toUTF8(path));
      } else {
        throw static_script_error(*getCurrentFilePath(), lineno(), columno(), L"can't include file: " + path);
      }
    }
    visitedFilePaths.insert(*includePath);
    if (includeStack.empty()) {
      in(fp);
    } else {
      push_matcher(new_matcher(fp));
    }
    includeStack.push_back(includePath);
  }
  void popInclude() {
    if (!includeStack.empty()) {
      loader->closeFile(*getCurrentFilePath(), in());
      if (includeStack.size() > 1) {
        pop_matcher();
      }
      includeStack.pop_back();
    }
  }
  ~DnhLexer() {
    while (!includeStack.empty()) {
      popInclude();
    }
  }
protected :
  wchar_t v_wchar;
  std::wstring v_wstr;
  std::string v_str;
  std::vector<std::shared_ptr<std::wstring>> includeStack;
  std::shared_ptr<FileLoader> loader;
  std::set<std::wstring> visitedFilePaths;
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
  if (includeStack.size() <= 1) {
    return tk::TK_EOF;
  }
  popInclude();
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

{number} { v_str = str(); return tk::TK_NUM; }

'\\.' { v_wchar = (wstr())[2]; return tk::TK_CHAR; }
'.' { v_wchar = (wstr())[1]; return tk::TK_CHAR; }

\"  { v_wstr.clear(); start(STRING_LIT); }

<STRING_LIT>{
\" { start(INITIAL); return tk::TK_STR; }
[\\][\\] { v_wstr += wstr(); }
\\\" { v_wstr += (wstr())[1]; }
. { v_wstr += wstr(); }
}

{ident} { v_str = str(); return tk::TK_IDENT; }

"#"{2,}"東方弾幕風" { return tk::TK_IGNORED_HEADER; }
"#"{2,}{ident} { return tk::TK_IGNORED_HEADER; }
"#東方弾幕風" { v_wstr = L"TouhouDanmakufu"; return tk::TK_HEADER; }
"#"{ident} { v_wstr = wstr().substr(1); return tk::TK_HEADER; }

. {
  throw static_script_error(*getCurrentFilePath(), lineno(), columno(), L"found illegal token: " + wstr());
}

<<EOF>> {
  if (includeStack.size() <= 1) {
    return tk::TK_EOF;
  }
  popInclude();
}
%%
