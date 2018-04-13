#include <bstorm/th_dnh_def.hpp>

#include <bstorm/util.hpp>

#include <fstream>
#include <locale>
#include <regex>
#include <sstream>

namespace bstorm
{
ThDnhDef loadThDnhDef(const std::string & path)
{
    ThDnhDef def;
    std::ifstream thDnhDefFile;
    thDnhDefFile.open(path, std::ios::in);
    thDnhDefFile.imbue(std::locale::classic());
    if (thDnhDefFile.good())
    {
        // ÉRÉÅÉìÉgèúãé
        std::string fileContents((std::istreambuf_iterator<char>(thDnhDefFile)), std::istreambuf_iterator<char>());
        thDnhDefFile.close();
        std::istringstream ss(std::regex_replace(fileContents, std::regex(R"((//.*?\r?\n)|(/\*.*?\*/)|(//.*?$))"), "", std::regex_constants::match_not_eol));

        // 1çsÇ∏Ç¬ì«Ç›çûÇﬁ
        std::string line;
        while (std::getline(ss, line))
        {
            if (line.empty()) continue;
            std::smatch match;
            if (regex_match(line, match, std::regex(R"(screen.width\s*=\s*(\d+))")))
            {
                def.screenWidth = std::atoi(match[1].str().c_str());
            } else if (regex_match(line, match, std::regex(R"(screen.height\s*=\s*(\d+))")))
            {
                def.screenHeight = std::atoi(match[1].str().c_str());
            } else if (regex_match(line, match, std::regex(R"(window.title\s*=\s*(.*))")))
            {
                def.windowTitle = fromMultiByte<932>(match[1].str());
            } else if (regex_match(line, match, std::regex(R"(package.script.main\s*=\s*(.*))")))
            {
                def.packageScriptMain = fromMultiByte<932>(match[1].str());
            }
        }
    }
    return def;
}
}