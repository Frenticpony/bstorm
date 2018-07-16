#include "util.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_item.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <imgui.h>

namespace bstorm
{
const char* GetBlendTypeName(int blendType)
{
    switch (blendType)
    {
        case BLEND_NONE:
            return "NONE";
        case BLEND_ALPHA:
            return "ALPHA";
        case BLEND_ADD_RGB:
            return "ADD";
        case BLEND_MULTIPLY:
            return "MULTIPLY";
        case BLEND_SUBTRACT:
            return "SUBTRACT";
        case BLEND_INV_DESTRGB:
            return "INV_DESTRGB";
        case BLEND_SHADOW:
            return "SHADOW";
        case BLEND_ADD_ARGB:
            return "ADD_ARGB";
        default:
            return "UNKNOWN";
    }
}

const char * GetObjTypeName(uint8_t objType)
{
    switch (objType)
    {
        case  OBJ_PRIMITIVE_2D: return "PRIMITIVE_2D";
        case  OBJ_SPRITE_2D: return "SPRITE_2D";
        case  OBJ_SPRITE_LIST_2D: return "SPRITE_LIST_2D";
        case  OBJ_PRIMITIVE_3D: return "PRIMITIVE_3D";
        case  OBJ_SPRITE_3D: return "SPRITE_3D";
        case  OBJ_TRAJECTORY_3D: return "TRAJECTORY_3D";
        case  OBJ_SHADER: return "SHADER";
        case  OBJ_MESH: return "MESH";
        case  OBJ_TEXT: return "TEXT";
        case  OBJ_SOUND: return "SOUND";
        case  OBJ_FILE_TEXT: return "FILE_TEXT";
        case  OBJ_FILE_BINARY: return "FILE_BINARY";
        case  OBJ_PLAYER: return "PLAYER";
        case  OBJ_SPELL_MANAGE: return "SPELL_MANAGE";
        case  OBJ_SPELL: return "SPELL";
        case  OBJ_ENEMY: return "ENEMY";
        case  OBJ_ENEMY_BOSS: return "ENEMY_BOSS";
        case  OBJ_ENEMY_BOSS_SCENE: return "ENEMY_BOSS_SCENE";
        case  OBJ_SHOT: return "SHOT";
        case  OBJ_LOOSE_LASER: return "LOOSE_LASER";
        case  OBJ_STRAIGHT_LASER: return "STRAIGHT_LASER";
        case  OBJ_CURVE_LASER: return "CURVE_LASER";
        case  OBJ_ITEM: return "ITEM";
    }
    return "UNKNOWN";
}

const char* GetPrimitiveTypeName(int primType)
{
    switch (primType)
    {
        case PRIMITIVE_POINT_LIST: return "POINT_LIST";
        case PRIMITIVE_LINELIST: return "LINELIST";
        case PRIMITIVE_LINESTRIP: return "LINESTRIP";
        case PRIMITIVE_TRIANGLELIST: return "TRIANGLELIST";
        case PRIMITIVE_TRIANGLESTRIP: return "TRIANGLESTRIP";
        case PRIMITIVE_TRIANGLEFAN: return "TRIANGLEFAN";
    }
    return "UNKNOWN";
}

const char * GetItemTypeName(int itemType)
{
    switch (itemType)
    {
        case ITEM_1UP: return "1UP";
        case ITEM_1UP_S: return "1UP_S";
        case ITEM_SPELL: return "SPELL";
        case ITEM_SPELL_S: return "SPELL_S";
        case ITEM_POWER: return "POWER";
        case ITEM_POWER_S: return "POWER_S";
        case ITEM_POINT: return "POINT";
        case ITEM_POINT_S: return "POINT_S";
        case ITEM_DEFAULT_BONUS: return "DEFAULT_BONUS";
    }
    return "UNKNOWN";
}

const char * GetCollisionGroupName(CollisionGroup colGroup)
{
    switch (colGroup)
    {
        case COL_GRP_ENEMY_SHOT: return "ENEMY_SHOT";
        case COL_GRP_PLAYER_ERASE_SHOT: return "PLAYER_ERASE_SHOT";
        case COL_GRP_PLAYER_NON_ERASE_SHOT: return "PLAYER_NON_ERASE_SHOT";
        case COL_GRP_PLAYER: return "PLAYER";
        case COL_GRP_PLAYER_GRAZE: return "PLAYER_GRAZE";
        case COL_GRP_ENEMY_TO_SHOT: return "ENEMY_TO_SHOT";
        case COL_GRP_ENEMY_TO_PLAYER: return "ENEMY_TO_PLAYER";
        case COL_GRP_SPELL: return "SPELL";
        case COL_GRP_PLAYER_TO_ITEM: return "PLAYER_TO_ITEM";
        case COL_GRP_ITEM: return "ITEM";
    }
    return "UNKNOWN";
}

const char * GetMoveModeName(const std::shared_ptr<MoveMode>& mode)
{
    if (std::dynamic_pointer_cast<MoveModeA>(mode)) return "A";
    if (std::dynamic_pointer_cast<MoveModeB>(mode)) return "B";
    if (std::dynamic_pointer_cast<MoveModeAtFrame>(mode)) return "AtFrame";
    if (std::dynamic_pointer_cast<MoveModeAtWeight>(mode)) return "AtWeight";
    if (std::dynamic_pointer_cast<MoveModeItemDown>(mode)) return "ItemDown";
    if (std::dynamic_pointer_cast<MoveModeItemToPlayer>(mode)) return "ItemToPlayer";
    if (std::dynamic_pointer_cast<MoveModeItemDest>(mode)) return "ItemDest";
    if (std::dynamic_pointer_cast<MoveModeHoverItemScoreText>(mode)) return "ItemScoreText";
    return "Unknown";
}

void DrawCroppedImage(const Rect<int>& rect, const std::shared_ptr<Texture>& texture)
{
    float u1 = 1.0f * rect.left / texture->GetWidth();
    float v1 = 1.0f * rect.top / texture->GetHeight();
    float u2 = 1.0f * rect.right / texture->GetWidth();
    float v2 = 1.0f * rect.bottom / texture->GetHeight();
    float rectWidth = std::abs(rect.right - rect.left);
    float rectHeight = std::abs(rect.bottom - rect.top);
    ImGui::Image(texture->GetTexture(), ImVec2(rectWidth, rectHeight), ImVec2(u1, v1), ImVec2(u2, v2));
}

void DrawIntersectionInfo(const std::shared_ptr<Intersection>& isect)
{
    if (!isect) return;
    {
        ImGui::BeginGroup();
        ImGui::BulletText("collision-group : %s", GetCollisionGroupName(isect->GetCollisionGroup()));
        ImGui::BulletText("tree-index      : %d", isect->GetTreeIndex());
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        const Shape& shape = isect->GetShape();
        auto shapeType = shape.GetType();
        ImGui::BulletText("shape    : %s", shapeType == Shape::Type::CIRCLE ? "Circle" : "Line");
        if (shapeType == Shape::Type::CIRCLE)
        {
            float x, y, r;
            shape.GetCircle(x, y, r);
            ImGui::BulletText("center-x : %f", x);
            ImGui::BulletText("center-y : %f", y);
            ImGui::BulletText("radius   : %f", r);
        } else if (shapeType == Shape::Type::RECT)
        {
            float x1, y1, x2, y2, width;
            shape.GetRect(x1, y1, x2, y2, width);
            ImGui::BulletText("begin-x : %f", x1);
            ImGui::BulletText("begin-y : %f", y1);
            ImGui::BulletText("end-x   : %f", x2);
            ImGui::BulletText("end-y   : %f", y2);
            ImGui::BulletText("width   : %f", width);
        }
        ImGui::EndGroup();
    }
}
void InputInt64(const char* label, int64_t* i)
{
    const int64_t step = 1;
    const int64_t fastStep = 5;
    ImGui::InputScalar(label, ImGuiDataType_S64, i, &step, &fastStep);
}
void InputDouble(const char * label, double * f)
{
    char buf[32] = { 0 };
    snprintf(buf, sizeof(buf), "%.15f", *f);
    ImGui::InputText(label, &buf[0], sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
    *f = std::atof(buf);
}
void InputString(const char * label, size_t limitSize, std::string & str)
{
    str.resize(std::max(limitSize, str.size() + 1), '\0');
    ImGui::InputText(label, &str[0], str.size());
    str = std::string(str.data());
}
void ViewIntRow(const char * name, int i)
{
    ImGui::Text(name); ImGui::NextColumn(); ImGui::Text("%d", i); ImGui::NextColumn();
}
void ViewFloatRow(const char * name, float f)
{
    ImGui::Text(name); ImGui::NextColumn(); ImGui::Text("%f", f); ImGui::NextColumn();
}
void ViewBoolRow(const char * name, bool b)
{
    ImGui::Text(name); ImGui::NextColumn(); ImGui::Text(b ? "true" : "false"); ImGui::NextColumn();
}
void ViewTextRow(const char * name, const char * value)
{
    ImGui::Text(name); ImGui::NextColumn(); ImGui::Text(value); ImGui::NextColumn();
}
void InputIntRow(const char * name, const char * id, int * i)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::InputInt(id, i, 1, 5); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void InputInt64Row(const char * name, const char * id, int64_t * i)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); InputInt64(id, i); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void InputFloatRow(const char * name, const char * id, float * f)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::InputFloat(id, f, 1.0f); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void InputFloat2Row(const char * name, const char * id, float * fs)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::InputFloat2(id, fs); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void InputDoubleRow(const char * name, const char * id, double * f)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); InputDouble(id, f); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void InputStringRow(const char * name, const char * id, size_t limit, std::string & str)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); InputString(id, limit, str); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void SliderFloatRow(const char * name, const char * id, float * f, float min, float max)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::SliderFloat(id, f, min, max); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void SliderAngleRow(const char * name, const char * id, float * f)
{
    SliderFloatRow(name, id, f, -180.f, 180.f);
}
void DragFloatRow(const char * name, const char * id, float * f, float speed, float min, float max)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::DragFloat(id, f, speed, min, max); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void DragAngleRow(const char * name, const char * id, float * f)
{
    DragFloatRow(name, id, f, 1, -360, 360);
}
void CheckboxRow(const char * name, const char * id, bool * b)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::Checkbox(id, b); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void ComboRow(const char * name, const char * id, int * i, const char * items)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn(); ImGui::PushItemWidth(-1); ImGui::Combo(id, i, items); ImGui::PopItemWidth(); ImGui::NextColumn();
}
void ColorEdit3Row(const char * name, const char * id, float * color)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn();  ImGui::PushItemWidth(-1); ImGui::ColorEdit3(id, color); ImGui::PopItemWidth();  ImGui::NextColumn();
}
void ColorEdit4Row(const char * name, const char * id, float * color)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text(name); ImGui::NextColumn();  ImGui::PushItemWidth(-1); ImGui::ColorEdit4(id, color, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::PopItemWidth();  ImGui::NextColumn();
}
}