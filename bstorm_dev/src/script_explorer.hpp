#pragma once

#include <bstorm/script_info.hpp>

#include <string>
#include <map>
#include <vector>
#include <array>
#include <mutex>
#include <thread>

namespace bstorm {
  class ScriptExplorer {
  public:
    ScriptExplorer(int left, int top, int width, int height);
    ~ScriptExplorer();
    void draw();
    // ディレクトリーツリー表示用のデータ構造
    struct TreeView {
      TreeView() : isOpen(true) {}
      std::string name;
      std::string uniq;
      bool isOpen;
      std::map<std::string, TreeView> children;
      bool isLeaf() const { return children.empty(); }
    };
    ScriptInfo getSelectedMainScript() const;
    ScriptInfo getSelectedPlayerScript() const;
  private:
    static constexpr int MaxFilterInputSize = 65;
    std::array<char, MaxFilterInputSize> filterInputMain;
    std::array<char, MaxFilterInputSize> filterInputPlayer;
    bool isLoadingNow() const;
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
    std::thread reloadThread; // Don't join.
    mutable std::mutex memberAccessSection;
    mutable std::mutex reloadSection;
  };
}