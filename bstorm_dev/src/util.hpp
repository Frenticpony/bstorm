#pragma once

#include <bstorm/rect.hpp>
#include <bstorm/intersection.hpp>

#include <memory>
#include <string>

namespace bstorm
{
class Texture;
class Intersection;
class MoveMode;
const char* GetBlendTypeName(int blendType);
const char* GetObjTypeName(uint8_t objType);
const char* GetPrimitiveTypeName(int primType);
const char* GetItemTypeName(int itemType);
const char* GetCollisionGroupName(CollisionGroup colGroup);
const char* GetMoveModeName(const std::shared_ptr<MoveMode>& mode);
void DrawCroppedImage(const Rect<int>& rect, const std::shared_ptr<Texture>& texture);
void DrawIntersectionInfo(const std::shared_ptr<Intersection>& isect);
void InputInt64(const char* label, int64_t* i);
void InputDouble(const char* label, double* f);
void InputString(const char* label, size_t limitSize, std::string& str);

void ViewIntRow(const char* name, int i);
void ViewFloatRow(const char* name, float f);
void ViewBoolRow(const char* name, bool b);
void ViewTextRow(const char* name, const char* value);
void InputIntRow(const char* name, const char* id, int* i);
void InputInt64Row(const char* name, const char* id, int64_t* i);
void InputFloatRow(const char* name, const char* id, float* f);
void InputFloat2Row(const char* name, const char* id, float* fs);
void InputDoubleRow(const char* name, const char* id, double* f);
void InputStringRow(const char* name, const char* id, size_t limit, std::string& str);
void SliderFloatRow(const char* name, const char* id, float* f, float min, float max);
void DragFloatRow(const char* name, const char* id, float* f, float speed, float min, float max);
void DragAngleRow(const char* name, const char* id, float* f);
void CheckboxRow(const char* name, const char* id, bool* b);
void ComboRow(const char* name, const char* id, int* i, const char* items);
void ColorEdit3Row(const char* name, const char* id, float* color);
void ColorEdit4Row(const char* name, const char* id, float* color);
}