#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bstorm/type.hpp>

namespace bstorm {
  struct MqoVec2 {
    float x;
    float y;
  };

  struct MqoVec3 {
    float x;
    float y;
    float z;
  };

  struct MqoColor4 {
    float r;
    float g;
    float b;
    float a;
  };

  // ライト
  struct MqoLight {
    MqoVec3 dir = { 0.408, 0.408, 0.816 };
    MqoColor4 color = MqoColor4{ 1.0f, 1.0f, 1.0f, 1.0f };
  };

  // 一つだけのシーン
  struct MqoScene {
    MqoVec3 pos = MqoVec3{ 0, 0, 1500 };
    MqoVec3 lookat = MqoVec3{ 0, 0, 0 };
    float head = -0.5236;
    float pich = 0.5236;
    float bank = 0;
    int ortho = 0;
    float zoom2 = 5.0000;
    MqoColor4 amb = MqoColor4{ 0.250, 0.250, 0.250, 1.0 };
    float frontclip = 225.00002;
    float backclip = 45000;
    std::vector<MqoLight> dirlights;
  };

  // 材質
  struct MqoMaterial {
    // 材質名
    std::wstring name;
    // シェーダー種類
    int shader = 3;
    // 頂点色の有無
    bool vcol = false;
    // 両面表示かどうか
    bool dbls = false;
    // 色
    MqoColor4 col = MqoColor4{ 1, 1, 1, 1 };
    // 拡散光の明るさ
    float dif = 0.8000;
    // 環境光の明るさ
    float amb = 0.600;
    // 自己発光の明るさ
    float emi = 0.000;
    float spc = 0.000;
    // 光沢の強さ
    float power = 5.00;
    // 鏡面反射
    float reflect = 1.00;
    // 屈折率
    float refract = 1.00;
    // 模様マッピング
    std::wstring tex;
    // 透明マッピング
    std::wstring aplane;
    // 凹凸マッピング
    std::wstring bump;
    // マッピング方式
    int proj_type = 0;
    // 投影位置
    MqoVec3 proj_pos = MqoVec3{ 0, 0, 0 };
    MqoVec3 proj_scale = MqoVec3{ 1, 1, 1 };
    MqoVec3 proj_angle = MqoVec3{ 0, 0, 0 };
  };

  // 1つの面
  struct MqoFace {
    // 頂点インデックス
    std::vector<int> vertexIndices;
    // 材質インデックス
    int materialIndex = 0;
    // 頂点UV
    std::vector<MqoVec2> uvs;
    std::vector<MqoColor4> cols;
    std::vector<float> crss;
  };

  // オブジェクト
  struct MqoObject {
    std::wstring name;
    int uid = -1;
    int depth = 0;
    bool folding = false;
    MqoVec3 scale = MqoVec3{ 1, 1, 1 };
    MqoVec3 rotation = MqoVec3{ 0, 0, 0 };
    MqoVec3 translation = MqoVec3{ 0, 0, 0 };
    int patch = 0;
    int patchtri;
    int segment;
    bool visible = true;
    bool locking = false;
    int shading = 1;
    float facet = 59.5;
    MqoColor4 color = MqoColor4{ 0.898, 0.498, 0.698, 1.0 };
    int color_type = 0;
    int mirror = 0;
    int mirror_axis = 1;
    float mirror_dis = 100;
    int lathe;
    int lathe_axis;
    int lathe_seg;
    std::vector<MqoVec3> vertices;
    std::vector<MqoFace> faces;
  };

  struct Mqo {
    std::wstring path;
    float version = 1.0;
    MqoScene scene;
    std::vector<MqoMaterial> materials;
    std::vector<MqoObject> objects;
  };

}