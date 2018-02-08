#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>

#include <bstorm/script_info.hpp>

namespace bstorm {
  class ScriptExplorer {
  public:
    ScriptExplorer(int left, int top, int width, int height);
    ~ScriptExplorer();
    void draw();
    // ディレクトリーツリー表示用のデータ構造
    struct TreeView {
      TreeView() : isOpen(false) {}
      std::string name;
      std::string uniq;
      bool isOpen;
      std::map<std::string, TreeView> children;
      bool isLeaf() const { return children.empty(); }
    };
    ScriptInfo getSelectedMainScript() const;
    ScriptInfo getSelectedPlayerScript() const;
  private:
    void reload();
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool useTreeView;
    bool showAllPlayerScripts;
    std::map<std::wstring, ScriptInfo> mainScripts;
    std::map<std::wstring, ScriptInfo> playerScripts;
    std::vector<std::wstring> freePlayerScriptPaths;
    TreeView mainScriptTreeView;
    TreeView playerScriptTreeView;
    std::wstring selectedMainScriptPath;
    std::wstring selectedPlayerScriptPath;
    mutable std::mutex mutex;
  };
}