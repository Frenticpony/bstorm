#pragma once

namespace bstorm
{
// prefix
constexpr char* DNH_RUNTIME_PREFIX = "r_"; // ランタイム関数
constexpr char* DNH_RUNTIME_BUILTIN_PREFIX = "rb_"; // ランタイムライブラリに書かれてる組み込み関数
#ifdef _DEBUG
constexpr char* DNH_BUILTIN_FUNC_PREFIX = "b_"; // 組み込み関数
#else
constexpr char* DNH_BUILTIN_FUNC_PREFIX = "d_"; // 組み込み関数
#endif
constexpr char* DNH_VAR_PREFIX = "d_"; // 変数
}