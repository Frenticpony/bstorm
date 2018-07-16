#pragma once

namespace bstorm
{
constexpr int ITEM_DEFAULT_BONUS = -65528;

constexpr int MAX_RENDER_PRIORITY = 100;
constexpr int DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN = 20;
constexpr int DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX = 80;
constexpr int DEFAULT_PLAYER_RENDER_PRIORITY = 30;
constexpr int DEFAULT_ENEMY_RENDER_PRIORITY = 40;
constexpr int DEFAULT_SHOT_RENDER_PRIORITY = 50;
constexpr int DEFAULT_ITEM_RENDER_PRIORITY = 60;
constexpr int DEFAULT_CAMERA_FOCUS_PERMIT_RENDER_PRIORITY = 69;

// prefix
constexpr char* DNH_RUNTIME_PREFIX = "r_"; // ランタイム関数
constexpr char* DNH_RUNTIME_BUILTIN_PREFIX = "rb_"; // ランタイムライブラリに書かれてる組み込み関数
#ifdef _DEBUG
constexpr char* DNH_BUILTIN_FUNC_PREFIX = "b_"; // 組み込み関数
#else
constexpr char* DNH_BUILTIN_FUNC_PREFIX = "d_"; // 組み込み関数
#endif
constexpr char* DNH_VAR_PREFIX = "d_"; // 変数

constexpr wchar_t* FREE_PLAYER_DIR = L"script/player";

constexpr wchar_t* DEFAULT_SYSTEM_PATH = L"script/default_system/Default_System.txt";
constexpr wchar_t* DEFAULT_ITEM_DATA_PATH = L"resource/script/Default_ItemData.txt";
constexpr wchar_t* DEFAULT_PACKAGE_PATH = L"resource/script/Default_Package.txt";
constexpr wchar_t* DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME = L"__DEFAULT_PACKAGE_ARGS__";

constexpr wchar_t* SYSTEM_STG_DIGIT_IMG_PATH = L"resource/img/System_Stg_Digit.png";
constexpr wchar_t* SYSTEM_SINGLE_STAGE_PATH = L"resource/script/System_SingleStage.txt";
constexpr wchar_t* SYSTEM_PLURAL_STAGE_PATH = L"resource/script/System_PluralStage.txt";

constexpr char* DNH_RUNTIME_NAME = "runtime.lua";

constexpr wchar_t* TRANSITION_RENDER_TARGET_NAME = L"__RENDERTARGET_TRANSITION__";
constexpr wchar_t* RESERVED_RENDER_TARGET_PREFIX = L"__RESERVED_RENDER_TARGET__";
constexpr wchar_t* SNAP_SHOT_RENDER_TARGET_NAME = L"__SNAP_SHOT__";
}
