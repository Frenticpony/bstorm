#pragma once

#include <string>
#include "../../json/single_include/nlohmann/json.hpp"

namespace bstorm {
  namespace conf {
    using nlohmann::json;

    struct KeyMap {
      std::string actionName;
      int64_t vkey;
      int64_t key;
      int64_t pad;
    };

    struct KeyConfig {
      std::vector<struct KeyMap> keyMaps;
    };

    struct Options {
      bool hideMouseCursor;
      bool saveLogFile;
    };

    struct WindowConfig {
      bool fullScreen;
      int64_t windowHeight;
      int64_t windowWidth;
    };

    struct BstormConfig {
      int64_t configVersion;
      std::string generatedBy;
      struct KeyConfig keyConfig;
      struct Options options;
      struct WindowConfig windowConfig;
    };

    inline json get_untyped(const json &j, const char *property) {
      if (j.find(property) != j.end()) {
        return j.at(property).get<json>();
      }
      return json();
    }

    inline void from_json(const json& _j, struct KeyMap& _x) {
      _x.actionName = _j.at("action_name").get<std::string>();
      _x.vkey = _j.at("vkey").get<int64_t>();
      _x.key = _j.at("key").get<int64_t>();
      _x.pad = _j.at("pad").get<int64_t>();
    }

    inline void to_json(json& _j, const struct KeyMap& _x) {
      _j = json::object();
      _j["action_name"] = _x.actionName;
      _j["vkey"] = _x.vkey;
      _j["key"] = _x.key;
      _j["pad"] = _x.pad;
    }

    inline void from_json(const json& _j, struct KeyConfig& _x) {
      _x.keyMaps = _j.at("key_maps").get<std::vector<struct KeyMap>>();
    }

    inline void to_json(json& _j, const struct KeyConfig& _x) {
      _j = json::object();
      _j["key_maps"] = _x.keyMaps;
    }

    inline void from_json(const json& _j, struct Options& _x) {
      _x.hideMouseCursor = _j.at("hide_mouse_cursor").get<bool>();
      _x.saveLogFile = _j.at("save_log_file").get<bool>();
    }

    inline void to_json(json& _j, const struct Options& _x) {
      _j = json::object();
      _j["hide_mouse_cursor"] = _x.hideMouseCursor;
      _j["save_log_file"] = _x.saveLogFile;
    }

    inline void from_json(const json& _j, struct WindowConfig& _x) {
      _x.fullScreen = _j.at("full_screen").get<bool>();
      _x.windowHeight = _j.at("window_height").get<int64_t>();
      _x.windowWidth = _j.at("window_width").get<int64_t>();
    }

    inline void to_json(json& _j, const struct WindowConfig& _x) {
      _j = json::object();
      _j["full_screen"] = _x.fullScreen;
      _j["window_height"] = _x.windowHeight;
      _j["window_width"] = _x.windowWidth;
    }

    inline void from_json(const json& _j, struct BstormConfig& _x) {
      _x.generatedBy = _j.at("generated_by").get<std::string>();
      _x.keyConfig = _j.at("key_config").get<struct KeyConfig>();
      _x.options = _j.at("options").get<struct Options>();
      _x.windowConfig = _j.at("window_config").get<struct WindowConfig>();
    }

    inline void to_json(json& _j, const struct BstormConfig& _x) {
      _j = json::object();
      _j["generated_by"] = _x.generatedBy;
      _j["key_config"] = _x.keyConfig;
      _j["options"] = _x.options;
      _j["window_config"] = _x.windowConfig;
    }
  }
  conf::BstormConfig loadBstormConfig(const std::string& path, bool isBinaryFormat, const std::string& defaultConfigResourceId) noexcept(false);
  bool saveBstormConfig(const std::string& path, bool isBinaryFormat, conf::BstormConfig config);
}
