#include <bstorm/dnh_const.hpp>
#include <bstorm/script_info.hpp>

namespace bstorm {
  int getScriptTypeConstFromName(const std::wstring& name) {
    if (name == SCRIPT_TYPE_PLAYER) return TYPE_SCRIPT_PLAYER;
    if (name == SCRIPT_TYPE_SINGLE) return TYPE_SCRIPT_SINGLE;
    if (name == SCRIPT_TYPE_PLURAL) return TYPE_SCRIPT_PLURAL;
    if (name == SCRIPT_TYPE_STAGE) return TYPE_SCRIPT_STAGE;
    if (name == SCRIPT_TYPE_PACKAGE) return TYPE_SCRIPT_PACKAGE;
    return -1;
  }

  std::wstring getScriptTypeNameFromConst(int c) {
    switch (c) {
      case TYPE_SCRIPT_PLAYER: return SCRIPT_TYPE_PLAYER;
      case TYPE_SCRIPT_SINGLE: return SCRIPT_TYPE_SINGLE;
      case TYPE_SCRIPT_PLURAL: return SCRIPT_TYPE_PLURAL;
      case TYPE_SCRIPT_STAGE: return SCRIPT_TYPE_STAGE;
      case TYPE_SCRIPT_PACKAGE: return SCRIPT_TYPE_PACKAGE;
    }
    return SCRIPT_TYPE_UNKNOWN;
  }
}
