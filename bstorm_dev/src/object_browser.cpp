#include "object_browser.hpp"

#include "IconsFontAwesome_c.h"
#include "util.hpp"
#include "resource_monitor.hpp"
#include "user_def_data_browser.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/string_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/obj_render.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_text.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/obj_spell.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/package.hpp>
#include <bstorm/package.hpp>

#include <unordered_set>
#include <imgui.h>

namespace bstorm
{
static const char blendTypeList[] = "NONE\0ALPHA\0ADD\0ADD_ARGB\0MULTIPLY\0SUBTRACT\0SHADOW\0INV_DESTRGB\0";
static const char primitiveTypeList[] = "POINT_LIST\0LINELIST\0LINESTRIP\0TRIANGLELIST\0TRIANGLESTRIP\0TRIANGLEFAN\0";
void drawObjEditArea(const std::shared_ptr<Obj>& obj, std::shared_ptr<ObjectLayerList>& layerList)
{
    constexpr ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen;
    ImGui::PushID(obj->GetID());
    {
        // Obj
        if (ImGui::CollapsingHeader("Obj", headerFlags))
        {
            ImGui::Columns(2);
            ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
            ImGui::Separator();
            ImGui::Separator();
            ViewIntRow("id", obj->GetID());
            ImGui::Separator();
            ViewTextRow("type", GetObjTypeName(obj->GetType()));
            ImGui::Separator();
            ViewBoolRow("dead", obj->IsDead());
            ImGui::Separator();
            ViewTextRow("scene", obj->IsStgSceneObject() ? "stg" : "package");
            ImGui::Separator();
            const auto& properties = obj->GetProperties();
            bool propertiesOpen = ImGui::TreeNode("properties##objProps"); ImGui::NextColumn(); ImGui::Text("(%d)", properties.size()); ImGui::NextColumn();
            if (propertiesOpen)
            {
                for (const auto& entry : properties)
                {
                    auto name = ToUTF8(entry.first);
                    auto value = ToUTF8(entry.second->ToString());
                    ImGui::Separator();
                    ImGui::Bullet(); ViewTextRow(name.c_str(), value.c_str());
                }
                ImGui::TreePop();
            }
            ImGui::Columns(1);
            ImGui::Separator();
        }
    }
    {
        // ObjRender
        if (auto objRender = std::dynamic_pointer_cast<ObjRender>(obj))
        {
            if (ImGui::CollapsingHeader("ObjRender", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    bool visible = objRender->IsVisible();
                    CheckboxRow("visible", "##visible", &visible);
                    objRender->SetVisible(visible);
                }
                ImGui::Separator();
                {
                    int priority = objRender->getRenderPriority();
                    if (layerList)
                    {
                        InputIntRow("priority", "##priority", &priority);
                        priority = constrain(priority, 0, MAX_RENDER_PRIORITY);
                        layerList->SetRenderPriority(objRender, priority);
                    } else
                    {
                        ViewIntRow("priority", priority);
                    }
                }
                ImGui::Separator();
                {
                    float x = objRender->GetX();
                    float y = objRender->GetY();
                    float z = objRender->GetZ();
                    InputFloatRow("x", "##x", &x);
                    ImGui::Separator();
                    InputFloatRow("y", "##y", &y);
                    ImGui::Separator();
                    InputFloatRow("z", "##z", &z);
                    objRender->SetPosition(x, y, z);
                }
                ImGui::Separator();
                {
                    float angleX = objRender->GetAngleX();
                    float angleY = objRender->GetAngleY();
                    float angleZ = objRender->GetAngleZ();
                    DragAngleRow("angle-x", "##angleX", &angleX);
                    ImGui::Separator();
                    DragAngleRow("angle-y", "##angleY", &angleY);
                    ImGui::Separator();
                    DragAngleRow("angle-z", "##angleZ", &angleZ);
                    objRender->SetAngleXYZ(angleX, angleY, angleZ);
                }
                ImGui::Separator();
                {
                    float scaleX = objRender->GetScaleX();
                    float scaleY = objRender->GetScaleY();
                    float scaleZ = objRender->GetScaleZ();
                    InputFloatRow("scale-x", "##scaleX", &scaleX);
                    ImGui::Separator();
                    InputFloatRow("scale-y", "##scaleY", &scaleY);
                    ImGui::Separator();
                    InputFloatRow("scale-z", "##scaleZ", &scaleZ);
                    objRender->SetScaleXYZ(scaleX, scaleY, scaleZ);
                }
                ImGui::Separator();
                {
                    const auto& rgb = objRender->GetColor();
                    float color[4] = { rgb.GetR() / 255.0f, rgb.GetG() / 255.0f, rgb.GetB() / 255.0f, objRender->GetAlpha() / 255.0f };
                    ColorEdit4Row("color", "##color", color);
                    objRender->SetColor(color[0] * 255, color[1] * 255, color[2] * 255);
                    objRender->SetAlpha(color[3] * 255);
                }
                ImGui::Separator();
                {
                    auto blendType = objRender->GetBlendType();
                    ComboRow("blend-type", "##blendType", &blendType, blendTypeList);
                    objRender->SetBlendType(blendType);
                }
                ImGui::Separator();
                {
                    bool fog = objRender->IsFogEnabled();
                    CheckboxRow("fog", "##fog", &fog);
                    objRender->SetFogEnable(fog);
                }
                ImGui::Separator();
                {
                    bool zWrite = objRender->IsZWriteEnabled();
                    CheckboxRow("z-write", "##zWrite", &zWrite);
                    objRender->SetZWrite(zWrite);
                }
                ImGui::Separator();
                {
                    bool zTest = objRender->IsZTestEnabled();
                    CheckboxRow("z-test", "##zTest", &zTest);
                    objRender->SetZTest(zTest);
                }
                ImGui::Separator();
                {
                    bool permitCamera = objRender->IsPermitCamera();
                    CheckboxRow("permit-camera", "##permitCamera", &permitCamera);
                    objRender->SetPermitCamera(permitCamera);
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }

    {
        // ObjPrim
        if (auto objPrim = std::dynamic_pointer_cast<ObjPrim>(obj))
        {
            if (ImGui::CollapsingHeader("ObjPrim", headerFlags))
            {
                const auto& texture = objPrim->GetTexture();
                const auto& renderTarget = objPrim->GetRenderTarget();
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    int primType = objPrim->GetPrimitiveType() - 1;
                    ComboRow("primitive-type", "##primitiveType", &primType, primitiveTypeList);
                    objPrim->SetPrimitiveType(primType + 1);
                }
                ImGui::Separator();
                {
                    ImGui::AlignTextToFramePadding();
                    bool verticesOpen = ImGui::TreeNode("vertices##primVertices"); ImGui::NextColumn();
                    {
                        int vertexCount = objPrim->GetVertexCount();
                        ImGui::InputInt("count##primVertexCount", &vertexCount, 1, 10);  ImGui::NextColumn();
                        objPrim->SetVertexCount(std::max(vertexCount, 0), false);
                    }
                    if (verticesOpen)
                    {
                        const auto& vertices = objPrim->GetVertices();
                        for (int i = 0; i < vertices.size(); i++)
                        {
                            ImGui::PushID(i);
                            const auto& vertex = vertices[i];
                            bool vertexOpen = ImGui::TreeNode(std::to_string(i).c_str()); ImGui::NextColumn(); ImGui::Text(""); ImGui::NextColumn();
                            if (vertexOpen)
                            {
                                ImGui::Separator();
                                {
                                    float x = vertex.x;
                                    float y = vertex.y;
                                    float z = vertex.z;
                                    InputFloatRow("x", "##primVertexX", &x);
                                    ImGui::Separator();
                                    InputFloatRow("y", "##primVertexY", &y);
                                    ImGui::Separator();
                                    InputFloatRow("z", "##primVertexZ", &z);
                                    objPrim->SetVertexPosition(i, x, y, z);
                                }
                                ImGui::Separator();
                                {
                                    float u = vertex.u;
                                    float v = vertex.v;
                                    SliderFloatRow("u", "##primVertexU", &u, 0.0f, 1.0f);
                                    ImGui::Separator();
                                    SliderFloatRow("v", "##primVertexV", &v, 0.0f, 1.0f);
                                    objPrim->SetVertexUV(i, u, v);
                                }
                                ImGui::Separator();
                                {
                                    float color[4] = { ((vertex.color >> 16) & 0xff) / 255.0f, ((vertex.color >> 8) & 0xff) / 255.0f, (vertex.color & 0xff) / 255.0f, ((vertex.color >> 24) & 0xff) / 255.0f, };
                                    ColorEdit4Row("color", "##primVertexColor", color);
                                    objPrim->SetVertexColor(i, color[0] * 255, color[1] * 255, color[2] * 255);
                                    objPrim->SetVertexAlpha(i, color[3] * 255);
                                }
                                ImGui::Separator();
                                ImGui::TreePop();
                            }
                            ImGui::PopID();
                        }
                        ImGui::Separator();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("use ObjRender::color"); ImGui::NextColumn();
                        bool useObjRenderColor = ImGui::Button("apply##primVertexColorAll"); ImGui::NextColumn();
                        if (useObjRenderColor)
                        {
                            const auto& rgb = objPrim->GetColor();
                            int alpha = objPrim->GetAlpha();
                            for (int i = 0; i < vertices.size(); i++)
                            {
                                objPrim->SetVertexColor(i, rgb.GetR(), rgb.GetG(), rgb.GetB());
                                objPrim->SetVertexAlpha(i, alpha);
                            }
                        }
                        ImGui::Separator();
                        ImGui::TreePop();
                    }
                }
                ImGui::Separator();
                {
                    ViewTextRow("texture", texture ? ToUTF8(texture->GetPath()).c_str() : "none");
                }
                ImGui::Separator();
                {
                    ViewTextRow("render-target", renderTarget ? ToUTF8(renderTarget->GetName()).c_str() : "none");
                }
                ImGui::Columns(1);
                ImGui::Separator();
                if (texture)
                {
                    if (ImGui::TreeNode("Texture Info##primTexture"))
                    {
                        DrawTextureInfo(texture, {}, nullptr);
                        ImGui::TreePop();
                    }
                }
                if (renderTarget)
                {
                    if (ImGui::TreeNode("RenderTarget Info##primRenderTarget"))
                    {
                        DrawRenderTargetInfo(renderTarget, {});
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
    {
        // ObjMove
        if (auto objMove = std::dynamic_pointer_cast<ObjMove>(obj))
        {
            if (ImGui::CollapsingHeader("ObjMove", headerFlags))
            {
                const auto& mode = objMove->GetMoveMode();
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewTextRow("mode", GetMoveModeName(mode));
                ImGui::Separator();
                {
                    float speed = objMove->GetSpeed();
                    if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode))
                    {
                        InputFloatRow("speed", "##moveSpeed", &speed);
                        modeA->SetSpeed(speed);
                    } else
                    {
                        ViewFloatRow("speed", speed);
                    }
                }
                ImGui::Separator();
                {
                    float angle = objMove->GetAngle();
                    if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode))
                    {
                        DragAngleRow("angle", "##moveAngle", &angle);
                        modeA->SetAngle(angle);
                    } else
                    {
                        ViewFloatRow("angle", angle);
                    }
                }
                if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode))
                {
                    ImGui::Separator();
                    {
                        float accel = modeA->GetAcceleration();
                        InputFloatRow("acceleration", "##moveAcceleration", &accel);
                        modeA->SetAcceleration(accel);
                    }
                    ImGui::Separator();
                    {
                        float maxSpeed = modeA->GetMaxSpeed();
                        InputFloatRow("max-speed", "##moveMaxSpeed", &maxSpeed);
                        modeA->SetMaxSpeed(maxSpeed);
                    }
                    ImGui::Separator();
                    {
                        float angularVelocity = modeA->GetAngularVelocity();
                        InputFloatRow("angular-velocity", "##moveAngularVelocity", &angularVelocity);
                        modeA->SetAngularVelocity(angularVelocity);
                    }
                }
                if (auto modeB = std::dynamic_pointer_cast<MoveModeB>(mode))
                {
                    ImGui::Separator();
                    {
                        float speed[2] = { modeB->GetSpeedX(), modeB->GetSpeedY() };
                        InputFloat2Row("speed-xy", "##moveBSpeedXY", speed);
                        modeB->SetSpeedX(speed[0]);
                        modeB->SetSpeedY(speed[1]);
                    }
                    ImGui::Separator();
                    {
                        float acceleration[2] = { modeB->GetAccelerationX(), modeB->GetAccelerationY() };
                        InputFloat2Row("acceleration-xy", "##moveBAccelerationXY", acceleration);
                        modeB->SetAccelerationX(acceleration[0]);
                        modeB->SetAccelerationY(acceleration[1]);
                    }
                    ImGui::Separator();
                    {
                        float maxSpeed[2] = { modeB->GetMaxSpeedX(), modeB->GetMaxSpeedY() };
                        InputFloat2Row("max-speed-xy", "##moveBMaxSpeedXY", maxSpeed);
                        modeB->SetMaxSpeedX(maxSpeed[0]);
                        modeB->SetMaxSpeedY(maxSpeed[1]);
                    }
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }
    {
        // ObjText
        if (auto objText = std::dynamic_pointer_cast<ObjText>(obj))
        {
            if (ImGui::CollapsingHeader("ObjText", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    int linePitch = objText->GetLinePitch();
                    InputIntRow("line-pitch", "##textLinePitch", &linePitch);
                    objText->SetLinePitch(linePitch);
                }
                ImGui::Separator();
                {
                    int sidePitch = objText->GetSidePitch();
                    InputIntRow("side-pitch", "##textSidePitch", &sidePitch);
                    objText->SetSidePitch(sidePitch);
                }
                ImGui::Separator();
                {
                    int alignment = objText->GetHorizontalAlignment();
                    if (alignment == ALIGNMENT_CENTER) alignment -= 2;
                    ComboRow("horizontal-alignment", "##textHorizontalAlignment", &alignment, "LEFT\0RIGHT\0CENTER\0");
                    if (alignment == 2) alignment = ALIGNMENT_CENTER;
                    objText->SetHorizontalAlignment(alignment);
                }
                ImGui::Separator();
                {
                    bool syntacticAnalysis = objText->IsSyntacticAnalysisEnabled();
                    CheckboxRow("syntactic-analysis", "##textSyntacticAnalysis", &syntacticAnalysis);
                    objText->SetSyntacticAnalysis(syntacticAnalysis);
                }
                ImGui::Separator();
                {
                    bool autoTransCenter = objText->IsAutoTransCenterEnabled();
                    CheckboxRow("auto-trans-center", "##textAutoTransCenter", &autoTransCenter);
                    objText->SetAutoTransCenter(autoTransCenter);
                }
                ImGui::Separator();
                {
                    float transCenter[2] = { objText->GetTransCenterX(), objText->GetTransCenterY() };
                    InputFloat2Row("trans-center", "##textTransCeneter", transCenter);
                    objText->SetTransCenter(transCenter[0], transCenter[1]);
                }
                ImGui::Separator();
                {
                    int maxWidth = objText->GetMaxWidth();
                    InputIntRow("max-width", "##textMaxWidth", &maxWidth);
                    objText->SetMaxWidth(std::max(maxWidth, 0));
                }
                ImGui::Separator();
                {
                    int maxHeight = objText->GetMaxHeight();
                    InputIntRow("max-height", "##textMaxHeight", &maxHeight);
                    objText->SetMaxHeight(std::max(maxHeight, 0));
                }
                ImGui::Separator();
                ViewIntRow("total-width", objText->GetTotalWidth());
                ImGui::Separator();
                ViewIntRow("total-height", objText->GetTotalHeight());
                ImGui::Separator();
                ViewIntRow("text-length", objText->GetTextLength());
                ImGui::Separator();
                ViewIntRow("text-length-cu", objText->GetTextLengthCU());
                ImGui::Separator();
                ImGui::AlignTextToFramePadding();
                bool openFontParams = ImGui::TreeNodeEx("font-params##textFontParams", ImGuiTreeNodeFlags_DefaultOpen); ImGui::NextColumn(); ImGui::PushItemWidth(-1);
                if (objText->IsFontParamModified())
                {
                    if (ImGui::Button(ICON_FA_REFRESH" update font##textUpdateFont"))
                    {
                        objText->GenerateFonts();
                    }
                }
                ImGui::PopItemWidth(); ImGui::NextColumn();
                if (openFontParams)
                {
                    ImGui::Separator();
                    {
                        std::string text = ToUTF8(objText->GetText());
                        InputStringRow("text", "##textText", 1024, text);
                        objText->SetText(ToUnicode(text));
                    }
                    ImGui::Separator();
                    {
                        std::string fontName = ToUTF8(objText->GetFontName());
                        InputStringRow("font-name", "##textFontName", 128, fontName);
                        objText->SetFontName(ToUnicode(fontName));
                    }
                    ImGui::Separator();
                    {
                        int fontSize = objText->GetFontSize();
                        InputIntRow("size", "##textFontSize", &fontSize);
                        objText->SetFontSize(std::max(0, fontSize));
                    }
                    ImGui::Separator();
                    {
                        bool bold = objText->IsFontBold();
                        CheckboxRow("bold", "##textFontBold", &bold);
                        objText->SetFontBold(bold);
                    }
                    ImGui::Separator();
                    {
                        const auto& rgb = objText->GetFontColorTop();
                        float color[3] = { rgb.GetR() / 255.0f, rgb.GetG() / 255.0f, rgb.GetB() / 255.0f };
                        ColorEdit3Row("top-color", "##textFontColorTop", color);
                        objText->SetFontColorTop(color[0] * 255, color[1] * 255, color[2] * 255);
                    }
                    ImGui::Separator();
                    {
                        const auto& rgb = objText->GetFontColorBottom();
                        float color[3] = { rgb.GetR() / 255.0f, rgb.GetG() / 255.0f, rgb.GetB() / 255.0f };
                        ColorEdit3Row("bottom-color", "##textFontColorBottom", color);
                        objText->SetFontColorBottom(color[0] * 255, color[1] * 255, color[2] * 255);
                    }
                    ImGui::Separator();
                    {
                        int fontBorderType = objText->GetFontBorderType();
                        ComboRow("border-type", "##textFontBorderType", &fontBorderType, "NONE\0FULL\0");
                        objText->SetFontBorderType(fontBorderType);
                    }
                    ImGui::Separator();
                    {
                        int fontBorderWidth = objText->GetFontBorderWidth();
                        InputIntRow("border-width", "##textFontBorderWidth", &fontBorderWidth);
                        objText->SetFontBorderWidth(std::max(0, fontBorderWidth));
                    }
                    ImGui::Separator();
                    {
                        const auto& rgb = objText->GetFontBorderColor();
                        float color[3] = { rgb.GetR() / 255.0f, rgb.GetG() / 255.0f, rgb.GetB() / 255.0f };
                        ColorEdit3Row("border-color", "##textFontBorderColor", color);
                        objText->SetFontBorderColor(color[0] * 255, color[1] * 255, color[2] * 255);
                    }
                    ImGui::TreePop();
                }
                ImGui::Columns(1);
                ImGui::Separator();
                {
                    if (ImGui::TreeNode("fonts##textFonts"))
                    {
                        int fontId = 0;
                        // 重複を弾く
                        std::unordered_set<IDirect3DTexture9*> displayed;
                        const auto& bodyFonts = objText->GetBodyFonts();
                        for (const auto& font : bodyFonts)
                        {
                            if (font)
                            {
                                auto fontTexture = font->GetTexture();
                                if (displayed.count(fontTexture) != 0) continue;
                                displayed.insert(fontTexture);
                                ImGui::PushID(fontId++);
                                if (ImGui::TreeNode(ToUTF8(std::wstring{ font->GetParams().c }).c_str()))
                                {
                                    DrawFontInfo(font);
                                    ImGui::TreePop();
                                }
                                ImGui::PopID();
                            }
                        }
                        const auto& rubyFonts = objText->GetRubyFonts();
                        for (const auto& ruby : rubyFonts)
                        {
                            for (const auto& font : ruby.text)
                            {
                                if (font)
                                {
                                    auto fontTexture = font->GetTexture();
                                    if (displayed.count(fontTexture) != 0) continue;
                                    displayed.insert(fontTexture);
                                    ImGui::PushID(fontId++);
                                    if (ImGui::TreeNode(ToUTF8(std::wstring{ font->GetParams().c }).c_str()))
                                    {
                                        DrawFontInfo(font);
                                        ImGui::TreePop();
                                    }
                                    ImGui::PopID();
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
    {
        // ObjShot
        if (auto objShot = std::dynamic_pointer_cast<ObjShot>(obj))
        {
            if (ImGui::CollapsingHeader("ObjShot", headerFlags))
            {
                const auto& shotData = objShot->GetShotData();
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewTextRow("ownership", objShot->IsPlayerShot() ? "player" : "enemy");
                ImGui::Separator();
                ViewBoolRow("registered", objShot->IsRegistered());
                ImGui::Separator();
                {
                    double damage = objShot->GetDamage();
                    InputDoubleRow("damage", "##shotDamage", &damage);
                    objShot->SetDamage(damage);
                }
                ImGui::Separator();
                {
                    int penetration = objShot->GetPenetration();
                    InputIntRow("penetration", "##shotPenetration", &penetration);
                    objShot->SetPenetration(penetration);
                }
                ImGui::Separator();
                {
                    int delay = objShot->GetDelay();
                    InputIntRow("delay", "##shotDelay", &delay);
                    objShot->SetDelay(delay);
                }
                ImGui::Separator();
                {
                    int sourceBlendType = objShot->GetSourceBlendType();
                    ComboRow("source-blend-type", "##shotSourceBlendType", &sourceBlendType, blendTypeList);
                    objShot->SetSourceBlendType(sourceBlendType);
                }
                ImGui::Separator();
                {
                    float angularVelocity = objShot->GetAngularVelocity();
                    InputFloatRow("angular-velocity", "##shotAngularVelocity", &angularVelocity);
                    objShot->SetAngularVelocity(angularVelocity);
                }
                ImGui::Separator();
                {
                    bool intersectionEnable = objShot->IsIntersectionEnabled();
                    CheckboxRow("intersection-enable", "##shotIntersectionEnable", &intersectionEnable);
                    objShot->SetIntersectionEnable(intersectionEnable);
                }
                ImGui::Separator();
                {
                    bool spellResist = objShot->IsSpellResistEnabled();
                    CheckboxRow("spell-resist", "##shotSpellResist", &spellResist);
                    objShot->SetSpellResistEnable(spellResist);
                }
                ImGui::Separator();
                {
                    bool spellFactor = objShot->IsSpellFactorEnabled();
                    CheckboxRow("spell-factor", "##shotSpellFactor", &spellFactor);
                    objShot->SetSpellFactor(spellFactor);
                }
                ImGui::Separator();
                {
                    bool eraseShot = objShot->IsEraseShotEnabled();
                    CheckboxRow("erase-shot", "##shotEraseShot", &eraseShot);
                    objShot->SetEraseShotEnable(eraseShot);
                }
                ImGui::Separator();
                {
                    bool itemChange = objShot->IsItemChangeEnabled();
                    CheckboxRow("item-change", "##shotItemChange", &itemChange);
                    objShot->SetItemChangeEnable(itemChange);
                }
                ImGui::Separator();
                {
                    bool autoDelete = objShot->IsAutoDeleteEnabled();
                    CheckboxRow("auto-delete", "##shotAutoDelete", &autoDelete);
                    objShot->SetAutoDeleteEnable(autoDelete);
                }
                ImGui::Separator();
                ViewBoolRow("graze-enable", objShot->IsGrazeEnabled());
                ImGui::Separator();
                ViewBoolRow("use-temp-intersection", objShot->IsTempIntersectionMode());
                ImGui::Separator();
                {
                    ViewBoolRow("frame-delete-started", objShot->IsFrameDeleteStarted());
                    if (objShot->IsFrameDeleteStarted())
                    {
                        ImGui::Separator();
                        ViewFloatRow("frame-delete-timer", objShot->GetDeleteFrameTimer());
                    }
                }
                ImGui::Separator();
                {
                    ViewBoolRow("fade-delete-started", objShot->IsFadeDeleteStarted());
                    if (objShot->IsFadeDeleteStarted())
                    {
                        ImGui::Separator();
                        ViewFloatRow("fade-delete-timer", objShot->GetFadeDeleteFrameTimer());
                        ImGui::Separator();
                        ViewFloatRow("fade-scale", objShot->GetFadeScale());
                    }
                }
                if (shotData && !shotData->animationData.empty())
                {
                    ImGui::Separator();
                    ViewIntRow("animation-frame-count", objShot->GetAnimationFrameCount());
                    ImGui::Separator();
                    ViewIntRow("animation-index", objShot->GetAnimationIndex());
                }
                ImGui::Separator();
                {
                    ViewIntRow("add-shot-frame-count", objShot->GetFrameCountForAddShot());
                }
                ImGui::Columns(1);
                ImGui::Separator();
                if (shotData)
                {
                    if (ImGui::TreeNode("shot-data##shotShotData"))
                    {
                        drawShotDataInfo(shotData);
                        ImGui::TreePop();
                    }
                }
                const auto& isects = objShot->GetIntersections();
                if (ImGui::TreeNode("intersections##shotIntersections"))
                {
                    int isectId = 0;
                    for (const auto& isect : isects)
                    {
                        ImGui::PushID(isectId++);
                        if (ImGui::TreeNode("intersection##shotIntersection"))
                        {
                            DrawIntersectionInfo(isect);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                const auto& tempIsects = objShot->GetTempIntersections();
                if (ImGui::TreeNode("temporary intersections##shotTempIntersections"))
                {
                    int isectId = 0;
                    for (const auto& isect : tempIsects)
                    {
                        ImGui::PushID(isectId++);
                        if (ImGui::TreeNode("intersection##shotTempIntersection"))
                        {
                            DrawIntersectionInfo(isect);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                const auto& addedShots = objShot->GetAddedShot();
                if (ImGui::TreeNode("added shots##shotAddedShots"))
                {
                    int addedShotId = 0;
                    for (const auto& addedShot : addedShots)
                    {
                        ImGui::PushID(addedShotId++);
                        if (ImGui::TreeNode((std::to_string(addedShot.objId) + "##shotAddedShot").c_str()))
                        {
                            ImGui::BulletText("type   : %s", addedShot.type == ObjShot::AddedShot::Type::A1 ? "A1" : "A2");
                            ImGui::BulletText("obj-id : %d", addedShot.objId);
                            ImGui::BulletText("frame  : %d", addedShot.frame);
                            if (addedShot.type == ObjShot::AddedShot::Type::A2)
                            {
                                ImGui::BulletText("dist   : %f", addedShot.dist);
                                ImGui::BulletText("angle  : %f", addedShot.angle);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }
        }
    }
    {
        // ObjLaser
        if (auto objLaser = std::dynamic_pointer_cast<ObjLaser>(obj))
        {
            if (ImGui::CollapsingHeader("ObjLaser", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    float length = objLaser->GetLength();
                    InputFloatRow("length", "##laserLength", &length);
                    objLaser->SetLength(length);
                }
                ImGui::Separator();
                {
                    float width = objLaser->GetRenderWidth();
                    InputFloatRow("render-width", "##laserRenderWidth", &width);
                    objLaser->SetRenderWidth(width);
                }
                ImGui::Separator();
                {
                    float width = objLaser->GetIntersectionWidth();
                    InputFloatRow("intersection-width", "##laserIntersectionWidth", &width);
                    objLaser->SetIntersectionWidth(std::max(width, 0.0f));
                }
                ImGui::Separator();
                {
                    float grazeInvalidFrame = objLaser->GetGrazeInvalidFrame();
                    InputFloatRow("graze-invalid-frame", "##laserGrazeInvalidFrame", &grazeInvalidFrame);
                    objLaser->SetGrazeInvalidFrame(std::max(grazeInvalidFrame, 0.0f));
                }
                ImGui::Separator();
                ViewFloatRow("graze-invalid-timer", objLaser->GetGrazeInvalidTimer());
                ImGui::Separator();
                {
                    float itemDistance = objLaser->GetItemDistance();
                    InputFloatRow("item-distance", "##laserItemDistance", &itemDistance);
                    objLaser->SetItemDistance(std::max(itemDistance, 0.0f));
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }
    {
        // ObjLooseLaser
        if (auto objLooseLaser = std::dynamic_pointer_cast<ObjLooseLaser>(obj))
        {
            if (ImGui::CollapsingHeader("ObjLooseLaser", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewFloatRow("render-length", objLooseLaser->GetRenderLength());
                ImGui::Separator();
                {
                    float invalidLengthHead = objLooseLaser->GetInvalidLengthHead();
                    float invalidLengthTail = objLooseLaser->GetInvalidLengthTail();
                    bool defaultInvalidLengthEnable = objLooseLaser->IsDefaultInvalidLengthEnabled();
                    CheckboxRow("default-invalid-length-enable", "##looseLaserDefaultInvalidLengthEnable", &defaultInvalidLengthEnable);
                    objLooseLaser->SetDefaultInvalidLengthEnable(defaultInvalidLengthEnable);
                    if (defaultInvalidLengthEnable)
                    {
                        ImGui::Separator();
                        ViewFloatRow("invalid-length-head", invalidLengthHead);
                        ImGui::Separator();
                        ViewFloatRow("invalid-length-tail", invalidLengthTail);
                    } else
                    {
                        ImGui::Separator();
                        InputFloatRow("invalid-length-head", "##looseLaserInvalidLengthHead", &invalidLengthHead);
                        ImGui::Separator();
                        InputFloatRow("invalid-length-tail", "##looseLaserInvalidLengthTail", &invalidLengthTail);
                        objLooseLaser->SetInvalidLength(invalidLengthHead, invalidLengthTail);
                    }
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }
    {
        // ObjStLaser
        if (auto objStLaser = std::dynamic_pointer_cast<ObjStLaser>(obj))
        {
            if (ImGui::CollapsingHeader("ObjStLaser", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    float laserAngle = objStLaser->GetLaserAngle();
                    DragAngleRow("laser-angle", "##stLaserLaserAngle", &laserAngle);
                    objStLaser->SetLaserAngle(laserAngle);
                }
                ImGui::Separator();
                {
                    bool source = objStLaser->HasSource();
                    CheckboxRow("laser-source", "##stLaserLaserSource", &source);
                    objStLaser->SetSource(source);
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }
    {
        // ObjCrLaser
        if (auto objCrLaser = std::dynamic_pointer_cast<ObjCrLaser>(obj))
        {
            if (ImGui::CollapsingHeader("ObjCrLaser", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                {
                    float tipDecrement = objCrLaser->GetTipDecrement();
                    SliderFloatRow("tip-decrement", "##crLaserTipDecrement", &tipDecrement, 0.0f, 1.0f);
                    objCrLaser->SetTipDecrement(tipDecrement);
                }
                ImGui::Separator();
                ViewIntRow("laser-node-count", objCrLaser->GetLaserNodeCount());
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }
    }
    {
        // ObjEnemy
        if (auto objEnemy = std::dynamic_pointer_cast<ObjEnemy>(obj))
        {
            if (ImGui::CollapsingHeader("ObjEnemy", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewBoolRow("registered", objEnemy->IsRegistered());
                ImGui::Separator();
                ViewBoolRow("is-boss", objEnemy->IsBoss());
                ImGui::Separator();
                {
                    double life = objEnemy->GetLife();
                    InputDoubleRow("life", "##enemyLife", &life);
                    objEnemy->SetLife(life);
                }
                ImGui::Separator();
                {
                    float damageRateShot = (float)objEnemy->GetDamageRateShot();
                    SliderFloatRow("damage-rate-shot", "##enemyDamageRateShot", &damageRateShot, 0.0f, 100.0f);
                    objEnemy->SetDamageRateShot((double)damageRateShot);
                }
                ImGui::Separator();
                {
                    float damageRateSpell = (float)objEnemy->GetDamageRateSpell();
                    SliderFloatRow("damage-rate-spell", "##enemyDamageRateSpell", &damageRateSpell, 0.0f, 100.0f);
                    objEnemy->SetDamageRateSpell((double)damageRateSpell);
                }
                ImGui::Separator();
                ViewIntRow("prev-frame-shot-hit-count", objEnemy->GetPrevFrameShotHitCount());
                ImGui::Columns(1);
                ImGui::Separator();
                const auto& tempIsects = objEnemy->GetTempIntersections();
                if (ImGui::TreeNode("temporary intersections##enemyTempIntersections"))
                {
                    int isectId = 0;
                    for (const auto& isect : tempIsects)
                    {
                        ImGui::PushID(isectId++);
                        if (ImGui::TreeNode("intersection##enemyTempIntersection"))
                        {
                            DrawIntersectionInfo(isect);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }
        }
    }
    {
        // ObjSpell
        if (auto objSpell = std::dynamic_pointer_cast<ObjSpell>(obj))
        {
            if (ImGui::CollapsingHeader("ObjSpell", headerFlags))
            {
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewBoolRow("registered", objSpell->IsRegistered());
                ImGui::Separator();
                {
                    double damage = objSpell->GetDamage();
                    InputDoubleRow("damage", "##spellDamage", &damage);
                    objSpell->SetDamage(damage);
                }
                ImGui::Separator();
                {
                    bool eraseShot = objSpell->IsEraseShotEnabled();
                    CheckboxRow("erase-shot", "##spellEraseShot", &eraseShot);
                    objSpell->SetEraseShotEnable(eraseShot);
                }
                ImGui::Columns(1);
                ImGui::Separator();
                const auto& tempIsects = objSpell->GetTempIntersections();
                if (ImGui::TreeNode("temporary intersections##spellTempIntersections"))
                {
                    int isectId = 0;
                    for (const auto& isect : tempIsects)
                    {
                        ImGui::PushID(isectId++);
                        if (ImGui::TreeNode("intersection##spellTempIntersection"))
                        {
                            DrawIntersectionInfo(isect);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }
        }
    }
    {
        // ObjItem
        if (auto objItem = std::dynamic_pointer_cast<ObjItem>(obj))
        {
            if (ImGui::CollapsingHeader("ObjItem", headerFlags))
            {
                const auto& itemData = objItem->GetItemData();
                ImGui::Columns(2);
                ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
                ImGui::Separator();
                ImGui::Separator();
                ViewTextRow("item-type", GetItemTypeName(objItem->GetItemType()));
                ImGui::Separator();
                {
                    int64_t score = objItem->GetScore();
                    InputInt64Row("score", "##itemScore", &score);
                    objItem->SetScore(score);
                }
                ImGui::Separator();
                {
                    bool renderScore = objItem->IsRenderScoreEnabled();
                    CheckboxRow("render-score", "##itemRenderScore", &renderScore);
                    objItem->SetRenderScoreEnable(renderScore);
                }
                ImGui::Separator();
                {
                    bool autoCollect = objItem->IsAutoCollectEnabled();
                    CheckboxRow("auto-collect-enable", "##itemAutoCollectEnable", &autoCollect);
                    objItem->SetAutoCollectEnable(autoCollect);
                }
                if (itemData && !itemData->animationData.empty())
                {
                    ImGui::Separator();
                    ViewIntRow("animation-frame-count", objItem->GetAnimationFrameCount());
                    ImGui::Separator();
                    ViewIntRow("animation-index", objItem->GetAnimationIndex());
                }
                ImGui::Separator();
                {
                    ViewBoolRow("auto-collected", objItem->IsAutoCollected());
                }
                ImGui::Separator();
                {
                    ViewBoolRow("obtained", objItem->IsObtained());
                }
                ImGui::Columns(1);
                ImGui::Separator();
                if (itemData)
                {
                    if (ImGui::TreeNode("item-data##itemItemData"))
                    {
                        drawItemDataInfo(itemData);
                        ImGui::TreePop();
                    }
                }
                const auto& isects = objItem->GetIntersections();
                if (ImGui::TreeNode("intersections##itemIntersections"))
                {
                    int isectId = 0;
                    for (const auto& isect : isects)
                    {
                        ImGui::PushID(isectId++);
                        if (ImGui::TreeNode("intersection##itemIntersection"))
                        {
                            DrawIntersectionInfo(isect);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }
        }
    }
    ImGui::PopID();
}

template <>
void Package::backDoor<ObjectBrowser>()
{
    static int selectedId = 0;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("SideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    const auto& table = objTable_->GetAll();
    for (const auto& entry : table)
    {
        auto id = entry.first;
        ImGui::PushID(id);
        const auto& obj = entry.second;
        const auto& nameProp = obj->GetValue(L"name");
        std::string name;
        if (nameProp->GetType() != DnhValue::Type::NIL)
        {
            name = ToUTF8(nameProp->ToString());
        } else
        {
            name = GetObjTypeName(obj->GetType());
        }
        std::string label = name;
        if (ImGui::Selectable(label.c_str(), selectedId == id))
        {
            selectedId = id;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("InfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Object Info");
    auto it = table.find(selectedId);
    if (it != table.end())
    {
        drawObjEditArea(it->second, objLayerList_);
    }
    ImGui::EndChild();
}

ObjectBrowser::ObjectBrowser(int left, int top, int width, int height) :
    iniLeft(left),
    iniTop(top),
    iniWidth(width),
    iniHeight(height),
    openFlag(false)
{
}

ObjectBrowser::~ObjectBrowser() {}

void ObjectBrowser::draw(const std::shared_ptr<Package>& package)
{
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Object", &openFlag, ImGuiWindowFlags_ResizeFromAnySide))
    {
        if (package) package->backDoor<ObjectBrowser>();
    }
    ImGui::End();
}
}
