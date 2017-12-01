#pragma once

#include <unordered_set>
#include <imgui.h>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
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
#include <bstorm/collision_matrix.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/engine.hpp>

#include "IconsFontAwesome_c.h"
#include "util.hpp"
#include "resource_monitor.hpp"
#include "user_def_data_browser.hpp"
#include "object_browser.hpp"

namespace bstorm {
  static const char blendTypeList[] = "NONE\0ALPHA\0ADD\0ADD_ARGB\0MULTIPLY\0SUBTRACT\0SHADOW\0INV_DESTRGB\0";
  static const char primitiveTypeList[] = "POINT_LIST\0LINELIST\0LINESTRIP\0TRIANGLELIST\0TRIANGLESTRIP\0TRIANGLEFAN\0";
  void drawObjEditArea(const std::shared_ptr<Obj>& obj, std::shared_ptr<ObjectLayerList>& layerList) {
    ImGui::PushID(obj->getID());
    {
      // Obj
      if (ImGui::CollapsingHeader("Obj", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(2);
        ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Separator();
        ViewIntRow("id", obj->getID());
        ImGui::Separator();
        ViewTextRow("type", getObjTypeName(obj->getType()));
        ImGui::Separator();
        ViewBoolRow("dead", obj->isDead());
        ImGui::Separator();
        ViewTextRow("scene", obj->isStgSceneObject() ? "stg" : "package");
        ImGui::Separator();
        const auto& properties = obj->getProperties();
        bool propertiesOpen = ImGui::TreeNode("properties##objProps"); ImGui::NextColumn(); ImGui::Text("(%d)", properties.size()); ImGui::NextColumn();
        if (propertiesOpen) {
          for (const auto& entry : properties) {
            auto name = toUTF8(entry.first);
            auto value = toUTF8(entry.second->toString());
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
      if (auto objRender = std::dynamic_pointer_cast<ObjRender>(obj)) {
        if (ImGui::CollapsingHeader("ObjRender")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            bool visible = objRender->isVisible();
            CheckboxRow("visible", "##visible", &visible);
            objRender->setVisible(visible);
          }
          ImGui::Separator();
          {
            int priority = objRender->getRenderPriority();
            if (layerList) {
              InputIntRow("priority", "##priority", &priority);
              priority = constrain(priority, 0, MAX_RENDER_PRIORITY);
              layerList->setRenderPriority(objRender, priority);
            } else {
              ViewIntRow("priority", priority);
            }
          }
          ImGui::Separator();
          {
            float x = objRender->getX();
            float y = objRender->getY();
            float z = objRender->getZ();
            InputFloatRow("x", "##x", &x);
            ImGui::Separator();
            InputFloatRow("y", "##y", &y);
            ImGui::Separator();
            InputFloatRow("z", "##z", &z);
            objRender->setPosition(x, y, z);
          }
          ImGui::Separator();
          {
            float angleX = objRender->getAngleX();
            float angleY = objRender->getAngleY();
            float angleZ = objRender->getAngleZ();
            SliderAngleRow("angle-x", "##angleX", &angleX);
            ImGui::Separator();
            SliderAngleRow("angle-y", "##angleY", &angleY);
            ImGui::Separator();
            SliderAngleRow("angle-z", "##angleZ", &angleZ);
            objRender->setAngleXYZ(angleX, angleY, angleZ);
          }
          ImGui::Separator();
          {
            float scaleX = objRender->getScaleX();
            float scaleY = objRender->getScaleY();
            float scaleZ = objRender->getScaleZ();
            InputFloatRow("scale-x", "##scaleX", &scaleX);
            ImGui::Separator();
            InputFloatRow("scale-y", "##scaleY", &scaleY);
            ImGui::Separator();
            InputFloatRow("scale-z", "##scaleZ", &scaleZ);
            objRender->setScaleXYZ(scaleX, scaleY, scaleZ);
          }
          ImGui::Separator();
          {
            const auto& rgb = objRender->getColor();
            float color[4] = { rgb.getR() / 255.0f, rgb.getG() / 255.0f, rgb.getB() / 255.0f, objRender->getAlpha() / 255.0f };
            ColorEdit4Row("color", "##color", color);
            objRender->setColor(color[0] * 255, color[1] * 255, color[2] * 255);
            objRender->setAlpha(color[3] * 255);
          }
          ImGui::Separator();
          {
            auto blendType = objRender->getBlendType();
            ComboRow("blend-type", "##blendType", &blendType, blendTypeList);
            objRender->setBlendType(blendType);
          }
          ImGui::Separator();
          {
            bool fog = objRender->isFogEnabled();
            CheckboxRow("fog", "##fog", &fog);
            objRender->setFogEnable(fog);
          }
          ImGui::Separator();
          {
            bool zWrite = objRender->isZWriteEnabled();
            CheckboxRow("z-write", "##zWrite", &zWrite);
            objRender->setZWrite(zWrite);
          }
          ImGui::Separator();
          {
            bool zTest = objRender->isZTestEnabled();
            CheckboxRow("z-test", "##zTest", &zTest);
            objRender->setZTest(zTest);
          }
          ImGui::Separator();
          {
            bool permitCamera = objRender->isPermitCamera();
            CheckboxRow("permit-camera", "##permitCamera", &permitCamera);
            objRender->setPermitCamera(permitCamera);
          }
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }

    {
      // ObjPrim
      if (auto objPrim = std::dynamic_pointer_cast<ObjPrim>(obj)) {
        if (ImGui::CollapsingHeader("ObjPrim")) {
          const auto& texture = objPrim->getTexture();
          const auto& renderTarget = objPrim->getRenderTarget();
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            int primType = objPrim->getPrimitiveType() - 1;
            ComboRow("primitive-type", "##primitiveType", &primType, primitiveTypeList);
            objPrim->setPrimitiveType(primType + 1);
          }
          ImGui::Separator();
          {
            ImGui::AlignFirstTextHeightToWidgets();
            bool verticesOpen = ImGui::TreeNode("vertices##primVertices"); ImGui::NextColumn();
            {
              int vertexCount = objPrim->getVertexCount();
              ImGui::InputInt("count##primVertexCount", &vertexCount, 1, 10);  ImGui::NextColumn();
              objPrim->setVertexCount(std::max(vertexCount, 0), false);
            }
            if (verticesOpen) {
              const auto& vertices = objPrim->getVertices();
              for (int i = 0; i < vertices.size(); i++) {
                ImGui::PushID(i);
                const auto& vertex = vertices[i];
                bool vertexOpen = ImGui::TreeNode(std::to_string(i).c_str()); ImGui::NextColumn(); ImGui::Text(""); ImGui::NextColumn();
                if (vertexOpen) {
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
                    objPrim->setVertexPosition(i, x, y, z);
                  }
                  ImGui::Separator();
                  {
                    float u = vertex.u;
                    float v = vertex.v;
                    SliderFloatRow("u", "##primVertexU", &u, 0.0f, 1.0f);
                    ImGui::Separator();
                    SliderFloatRow("v", "##primVertexV", &v, 0.0f, 1.0f);
                    objPrim->setVertexUV(i, u, v);
                  }
                  ImGui::Separator();
                  {
                    float color[4] = { ((vertex.color >> 16) & 0xff) / 255.0f, ((vertex.color >> 8) & 0xff) / 255.0f, (vertex.color & 0xff) / 255.0f, ((vertex.color >> 24) & 0xff) / 255.0f, };
                    ColorEdit4Row("color", "##primVertexColor", color);
                    objPrim->setVertexColor(i, color[0] * 255, color[1] * 255, color[2] * 255);
                    objPrim->setVertexAlpha(i, color[3] * 255);
                  }
                  ImGui::Separator();
                  ImGui::TreePop();
                }
                ImGui::PopID();
              }
              ImGui::Separator();
              ImGui::AlignFirstTextHeightToWidgets();
              ImGui::Text("use ObjRender::color"); ImGui::NextColumn();
              bool useObjRenderColor = ImGui::Button("apply##primVertexColorAll"); ImGui::NextColumn();
              if (useObjRenderColor) {
                const auto& rgb = objPrim->getColor();
                int alpha = objPrim->getAlpha();
                for (int i = 0; i < vertices.size(); i++) {
                  objPrim->setVertexColor(i, rgb.getR(), rgb.getG(), rgb.getB());
                  objPrim->setVertexAlpha(i, alpha);
                }
              }
              ImGui::Separator();
              ImGui::TreePop();
            }
          }
          ImGui::Separator();
          {
            ViewTextRow("texture", texture ? toUTF8(texture->getPath()).c_str() : "none");
          }
          ImGui::Separator();
          {
            ViewTextRow("render-target", renderTarget ? toUTF8(renderTarget->getName()).c_str() : "none");
          }
          ImGui::Columns(1);
          ImGui::Separator();
          if (texture) {
            if (ImGui::TreeNode("Texture Info##primTexture")) {
              drawTextureInfo(texture, {});
              ImGui::TreePop();
            }
          }
          if (renderTarget) {
            if (ImGui::TreeNode("RenderTarget Info##primRenderTarget")) {
              drawRenderTargetInfo(renderTarget, {});
              ImGui::TreePop();
            }
          }
        }
      }
    }
    {
      // ObjMove
      if (auto objMove = std::dynamic_pointer_cast<ObjMove>(obj)) {
        if (ImGui::CollapsingHeader("ObjMove")) {
          const auto& mode = objMove->getMoveMode();
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewTextRow("mode", getMoveModeName(mode));
          ImGui::Separator();
          {
            float speed = objMove->getSpeed();
            if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode)) {
              InputFloatRow("speed", "##moveSpeed", &speed);
              modeA->setSpeed(speed);
            } else {
              ViewFloatRow("speed", speed);
            }
          }
          ImGui::Separator();
          {
            float angle = objMove->getAngle();
            if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode)) {
              SliderAngleRow("angle", "##moveAngle", &angle);
              modeA->setAngle(angle);
            } else {
              ViewFloatRow("angle", angle);
            }
          }
          if (auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode)) {
            ImGui::Separator();
            {
              float accel = modeA->getAcceleration();
              InputFloatRow("acceleration", "##moveAcceleration", &accel);
              modeA->setAcceleration(accel);
            }
            ImGui::Separator();
            {
              float maxSpeed = modeA->getMaxSpeed();
              InputFloatRow("max-speed", "##moveMaxSpeed", &maxSpeed);
              modeA->setMaxSpeed(maxSpeed);
            }
            ImGui::Separator();
            {
              float angularVelocity = modeA->getAngularVelocity();
              InputFloatRow("angular-velocity", "##moveAngularVelocity", &angularVelocity);
              modeA->setAngularVelocity(angularVelocity);
            }
          }
          if (auto modeB = std::dynamic_pointer_cast<MoveModeB>(mode)) {
            ImGui::Separator();
            {
              float speed[2] = { modeB->getSpeedX(), modeB->getSpeedY() };
              InputFloat2Row("speed-xy", "##moveBSpeedXY", speed);
              modeB->setSpeedX(speed[0]);
              modeB->setSpeedY(speed[1]);
            }
            ImGui::Separator();
            {
              float acceleration[2] = { modeB->getAccelerationX(), modeB->getAccelerationY() };
              InputFloat2Row("acceleration-xy", "##moveBAccelerationXY", acceleration);
              modeB->setAccelerationX(acceleration[0]);
              modeB->setAccelerationY(acceleration[1]);
            }
            ImGui::Separator();
            {
              float maxSpeed[2] = { modeB->getMaxSpeedX(), modeB->getMaxSpeedY() };
              InputFloat2Row("max-speed-xy", "##moveBMaxSpeedXY", maxSpeed);
              modeB->setMaxSpeedX(maxSpeed[0]);
              modeB->setMaxSpeedY(maxSpeed[1]);
            }
          }
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }
    {
      // ObjText
      if (auto objText = std::dynamic_pointer_cast<ObjText>(obj)) {
        if (ImGui::CollapsingHeader("ObjText")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            int linePitch = objText->getLinePitch();
            InputIntRow("line-pitch", "##textLinePitch", &linePitch);
            objText->setLinePitch(linePitch);
          }
          ImGui::Separator();
          {
            int sidePitch = objText->getSidePitch();
            InputIntRow("side-pitch", "##textSidePitch", &sidePitch);
            objText->setSidePitch(sidePitch);
          }
          ImGui::Separator();
          {
            int alignment = objText->getHorizontalAlignment();
            if (alignment == ALIGNMENT_CENTER) alignment -= 2;
            ComboRow("horizontal-alignment", "##textHorizontalAlignment", &alignment, "LEFT\0RIGHT\0CENTER\0");
            if (alignment == 2) alignment = ALIGNMENT_CENTER;
            objText->setHorizontalAlignment(alignment);
          }
          ImGui::Separator();
          {
            bool syntacticAnalysis = objText->isSyntacticAnalysisEnabled();
            CheckboxRow("syntactic-analysis", "##textSyntacticAnalysis", &syntacticAnalysis);
            objText->setSyntacticAnalysis(syntacticAnalysis);
          }
          ImGui::Separator();
          {
            bool autoTransCenter = objText->isAutoTransCenterEnabled();
            CheckboxRow("auto-trans-center", "##textAutoTransCenter", &autoTransCenter);
            objText->setAutoTransCenter(autoTransCenter);
          }
          ImGui::Separator();
          {
            float transCenter[2] = { objText->getTransCenterX(), objText->getTransCenterY() };
            InputFloat2Row("trans-center", "##textTransCeneter", transCenter);
            objText->setTransCenter(transCenter[0], transCenter[1]);
          }
          ImGui::Separator();
          {
            int maxWidth = objText->getMaxWidth();
            InputIntRow("max-width", "##textMaxWidth", &maxWidth);
            objText->setMaxWidth(std::max(maxWidth, 0));
          }
          ImGui::Separator();
          {
            int maxHeight = objText->getMaxHeight();
            InputIntRow("max-height", "##textMaxHeight", &maxHeight);
            objText->setMaxHeight(std::max(maxHeight, 0));
          }
          ImGui::Separator();
          ViewIntRow("total-width", objText->getTotalWidth());
          ImGui::Separator();
          ViewIntRow("total-height", objText->getTotalHeight());
          ImGui::Separator();
          ViewIntRow("text-length", objText->getTextLength());
          ImGui::Separator();
          ViewIntRow("text-length-cu", objText->getTextLengthCU());
          ImGui::Separator();
          ImGui::AlignFirstTextHeightToWidgets();
          bool openFontParams = ImGui::TreeNodeEx("font-params##textFontParams", ImGuiTreeNodeFlags_DefaultOpen); ImGui::NextColumn(); ImGui::PushItemWidth(-1);
          if (objText->isFontParamModified()) {
            if (ImGui::Button(ICON_FA_REFRESH" update font##textUpdateFont")) {
              objText->generateFonts();
            }
          }
          ImGui::PopItemWidth(); ImGui::NextColumn();
          if (openFontParams) {
            ImGui::Separator();
            {
              std::string text = toUTF8(objText->getText());
              InputStringRow("text", "##textText", 1024, text);
              objText->setText(toUnicode(text));
            }
            ImGui::Separator();
            {
              std::string fontName = toUTF8(objText->getFontName());
              InputStringRow("font-name", "##textFontName", 128, fontName);
              objText->setFontName(toUnicode(fontName));
            }
            ImGui::Separator();
            {
              int fontSize = objText->getFontSize();
              InputIntRow("size", "##textFontSize", &fontSize);
              objText->setFontSize(std::max(0, fontSize));
            }
            ImGui::Separator();
            {
              bool bold = objText->isFontBold();
              CheckboxRow("bold", "##textFontBold", &bold);
              objText->setFontBold(bold);
            }
            ImGui::Separator();
            {
              const auto& rgb = objText->getFontColorTop();
              float color[3] = { rgb.getR() / 255.0f, rgb.getG() / 255.0f, rgb.getB() / 255.0f };
              ColorEdit3Row("top-color", "##textFontColorTop", color);
              objText->setFontColorTop(color[0] * 255, color[1] * 255, color[2] * 255);
            }
            ImGui::Separator();
            {
              const auto& rgb = objText->getFontColorBottom();
              float color[3] = { rgb.getR() / 255.0f, rgb.getG() / 255.0f, rgb.getB() / 255.0f };
              ColorEdit3Row("bottom-color", "##textFontColorBottom", color);
              objText->setFontColorBottom(color[0] * 255, color[1] * 255, color[2] * 255);
            }
            ImGui::Separator();
            {
              int fontBorderType = objText->getFontBorderType();
              ComboRow("border-type", "##textFontBorderType", &fontBorderType, "NONE\0FULL\0");
              objText->setFontBorderType(fontBorderType);
            }
            ImGui::Separator();
            {
              int fontBorderWidth = objText->getFontBorderWidth();
              InputIntRow("border-width", "##textFontBorderWidth", &fontBorderWidth);
              objText->setFontBorderWidth(std::max(0, fontBorderWidth));
            }
            ImGui::Separator();
            {
              const auto& rgb = objText->getFontBorderColor();
              float color[3] = { rgb.getR() / 255.0f, rgb.getG() / 255.0f, rgb.getB() / 255.0f };
              ColorEdit3Row("border-color", "##textFontBorderColor", color);
              objText->setFontBorderColor(color[0] * 255, color[1] * 255, color[2] * 255);
            }
            ImGui::TreePop();
          }
          ImGui::Columns(1);
          ImGui::Separator();
          {
            if (ImGui::TreeNode("fonts##textFonts")) {
              int fontId = 0;
              // 重複を弾く
              std::unordered_set<IDirect3DTexture9*> displayed;
              const auto& bodyFonts = objText->getBodyFonts();
              for (const auto& font : bodyFonts) {
                if (font) {
                  auto fontTexture = font->getTexture();
                  if (displayed.count(fontTexture) != 0) continue;
                  displayed.insert(fontTexture);
                  ImGui::PushID(fontId++);
                  if (ImGui::TreeNode(toUTF8(std::wstring{ font->getParams().c }).c_str())) {
                    drawFontInfo(font);
                    ImGui::TreePop();
                  }
                  ImGui::PopID();
                }
              }
              const auto& rubyFonts = objText->getRubyFonts();
              for (const auto& ruby : rubyFonts) {
                for (const auto& font : ruby.text) {
                  if (font) {
                    auto fontTexture = font->getTexture();
                    if (displayed.count(fontTexture) != 0) continue;
                    displayed.insert(fontTexture);
                    ImGui::PushID(fontId++);
                    if (ImGui::TreeNode(toUTF8(std::wstring{ font->getParams().c }).c_str())) {
                      drawFontInfo(font);
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
      if (auto objShot = std::dynamic_pointer_cast<ObjShot>(obj)) {
        if (ImGui::CollapsingHeader("ObjShot")) {
          const auto& shotData = objShot->getShotData();
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewTextRow("ownership", objShot->isPlayerShot() ? "player" : "enemy");
          ImGui::Separator();
          ViewBoolRow("registered", objShot->isRegistered());
          ImGui::Separator();
          {
            double damage = objShot->getDamage();
            InputDoubleRow("damage", "##shotDamage", &damage);
            objShot->setDamage(damage);
          }
          ImGui::Separator();
          {
            int penetration = objShot->getPenetration();
            InputIntRow("penetration", "##shotPenetration", &penetration);
            objShot->setPenetration(penetration);
          }
          ImGui::Separator();
          {
            int delay = objShot->getDelay();
            InputIntRow("delay", "##shotDelay", &delay);
            objShot->setDelay(delay);
          }
          ImGui::Separator();
          {
            int sourceBlendType = objShot->getSourceBlendType();
            ComboRow("source-blend-type", "##shotSourceBlendType", &sourceBlendType, blendTypeList);
            objShot->setSourceBlendType(sourceBlendType);
          }
          ImGui::Separator();
          {
            float angularVelocity = objShot->getAngularVelocity();
            InputFloatRow("angular-velocity", "##shotAngularVelocity", &angularVelocity);
            objShot->setAngularVelocity(angularVelocity);
          }
          ImGui::Separator();
          {
            bool intersectionEnable = objShot->isIntersectionEnabled();
            CheckboxRow("intersection-enable", "##shotIntersectionEnable", &intersectionEnable);
            objShot->setIntersectionEnable(intersectionEnable);
          }
          ImGui::Separator();
          {
            bool spellResist = objShot->isSpellResistEnabled();
            CheckboxRow("spell-resist", "##shotSpellResist", &spellResist);
            objShot->setSpellResist(spellResist);
          }
          ImGui::Separator();
          {
            bool spellFactor = objShot->isSpellFactorEnabled();
            CheckboxRow("spell-factor", "##shotSpellFactor", &spellFactor);
            objShot->setSpellFactor(spellFactor);
          }
          ImGui::Separator();
          {
            bool eraseShot = objShot->isEraseShotEnabled();
            CheckboxRow("erase-shot", "##shotEraseShot", &eraseShot);
            objShot->setEraseShot(eraseShot);
          }
          ImGui::Separator();
          {
            bool itemChange = objShot->isItemChangeEnabled();
            CheckboxRow("item-change", "##shotItemChange", &itemChange);
            objShot->setItemChange(itemChange);
          }
          ImGui::Separator();
          {
            bool autoDelete = objShot->isAutoDeleteEnabled();
            CheckboxRow("auto-delete", "##shotAutoDelete", &autoDelete);
            objShot->setAutoDeleteEnable(autoDelete);
          }
          ImGui::Separator();
          ViewBoolRow("graze-enable", objShot->isGrazeEnabled());
          ImGui::Separator();
          ViewBoolRow("use-temp-intersection", objShot->isTempIntersectionMode());
          ImGui::Separator();
          {
            ViewBoolRow("frame-delete-started", objShot->isFrameDeleteStarted());
            if (objShot->isFrameDeleteStarted()) {
              ImGui::Separator();
              ViewFloatRow("frame-delete-timer", objShot->getDeleteFrameTimer());
            }
          }
          ImGui::Separator();
          {
            ViewBoolRow("fade-delete-started", objShot->isFadeDeleteStarted());
            if (objShot->isFadeDeleteStarted()) {
              ImGui::Separator();
              ViewFloatRow("fade-delete-timer", objShot->getFadeDeleteFrameTimer());
              ImGui::Separator();
              ViewFloatRow("fade-scale", objShot->getFadeScale());
            }
          }
          if (shotData && !shotData->animationData.empty()) {
            ImGui::Separator();
            ViewIntRow("animation-frame-count", objShot->getAnimationFrameCount());
            ImGui::Separator();
            ViewIntRow("animation-index", objShot->getAnimationIndex());
          }
          ImGui::Separator();
          {
            ViewIntRow("add-shot-frame-count", objShot->getFrameCountForAddShot());
          }
          ImGui::Columns(1);
          ImGui::Separator();
          if (shotData) {
            if (ImGui::TreeNode("shot-data##shotShotData")) {
              drawShotDataInfo(shotData);
              ImGui::TreePop();
            }
          }
          const auto& isects = objShot->getIntersections();
          if (ImGui::TreeNode("intersections##shotIntersections")) {
            int isectId = 0;
            for (const auto& isect : isects) {
              ImGui::PushID(isectId++);
              if (ImGui::TreeNode("intersection##shotIntersection")) {
                drawIntersectionInfo(isect);
                ImGui::TreePop();
              }
              ImGui::PopID();
            }
            ImGui::TreePop();
          }
          const auto& tempIsects = objShot->getTempIntersections();
          if (ImGui::TreeNode("temporary intersections##shotTempIntersections")) {
            int isectId = 0;
            for (const auto& isect : tempIsects) {
              ImGui::PushID(isectId++);
              if (ImGui::TreeNode("intersection##shotTempIntersection")) {
                drawIntersectionInfo(isect);
                ImGui::TreePop();
              }
              ImGui::PopID();
            }
            ImGui::TreePop();
          }
          const auto& addedShots = objShot->getAddedShot();
          if (ImGui::TreeNode("added shots##shotAddedShots")) {
            int addedShotId = 0;
            for (const auto& addedShot : addedShots) {
              ImGui::PushID(addedShotId++);
              if (ImGui::TreeNode((std::to_string(addedShot.objId) + "##shotAddedShot").c_str())) {
                ImGui::BulletText("type   : %s", addedShot.type == ObjShot::AddedShot::Type::A1 ? "A1" : "A2");
                ImGui::BulletText("obj-id : %d", addedShot.objId);
                ImGui::BulletText("frame  : %d", addedShot.frame);
                if (addedShot.type == ObjShot::AddedShot::Type::A2) {
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
      if (auto objLaser = std::dynamic_pointer_cast<ObjLaser>(obj)) {
        if (ImGui::CollapsingHeader("ObjLaser")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            float length = objLaser->getLength();
            InputFloatRow("length", "##laserLength", &length);
            objLaser->setLength(length);
          }
          ImGui::Separator();
          {
            float width = objLaser->getRenderWidth();
            InputFloatRow("render-width", "##laserRenderWidth", &width);
            objLaser->setRenderWidth(width);
          }
          ImGui::Separator();
          {
            float width = objLaser->getIntersectionWidth();
            InputFloatRow("intersection-width", "##laserIntersectionWidth", &width);
            objLaser->setIntersectionWidth(std::max(width, 0.0f));
          }
          ImGui::Separator();
          {
            float grazeInvalidFrame = objLaser->getGrazeInvalidFrame();
            InputFloatRow("graze-invalid-frame", "##laserGrazeInvalidFrame", &grazeInvalidFrame);
            objLaser->setGrazeInvalidFrame(std::max(grazeInvalidFrame, 0.0f));
          }
          ImGui::Separator();
          ViewFloatRow("graze-invalid-timer", objLaser->getGrazeInvalidTimer());
          ImGui::Separator();
          {
            float itemDistance = objLaser->getItemDistance();
            InputFloatRow("item-distance", "##laserItemDistance", &itemDistance);
            objLaser->setItemDistance(std::max(itemDistance, 0.0f));
          }
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }
    {
      // ObjLooseLaser
      if (auto objLooseLaser = std::dynamic_pointer_cast<ObjLooseLaser>(obj)) {
        if (ImGui::CollapsingHeader("ObjLooseLaser")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewFloatRow("render-length", objLooseLaser->getRenderLength());
          ImGui::Separator();
          {
            float invalidLengthHead = objLooseLaser->getInvalidLengthHead();
            float invalidLengthTail = objLooseLaser->getInvalidLengthTail();
            bool defaultInvalidLengthEnable = objLooseLaser->isDefaultInvalidLengthEnabled();
            CheckboxRow("default-invalid-length-enable", "##looseLaserDefaultInvalidLengthEnable", &defaultInvalidLengthEnable);
            objLooseLaser->setDefaultInvalidLengthEnable(defaultInvalidLengthEnable);
            if (defaultInvalidLengthEnable) {
              ImGui::Separator();
              ViewFloatRow("invalid-length-head", invalidLengthHead);
              ImGui::Separator();
              ViewFloatRow("invalid-length-tail", invalidLengthTail);
            } else {
              ImGui::Separator();
              InputFloatRow("invalid-length-head", "##looseLaserInvalidLengthHead", &invalidLengthHead);
              ImGui::Separator();
              InputFloatRow("invalid-length-tail", "##looseLaserInvalidLengthTail", &invalidLengthTail);
              objLooseLaser->setInvalidLength(invalidLengthHead, invalidLengthTail);
            }
          }
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }
    {
      // ObjStLaser
      if (auto objStLaser = std::dynamic_pointer_cast<ObjStLaser>(obj)) {
        if (ImGui::CollapsingHeader("ObjStLaser")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            float laserAngle = objStLaser->getLaserAngle();
            SliderAngleRow("laser-angle", "##stLaserLaserAngle", &laserAngle);
            objStLaser->setLaserAngle(laserAngle);
          }
          ImGui::Separator();
          {
            bool source = objStLaser->hasSource();
            CheckboxRow("laser-source", "##stLaserLaserSource", &source);
            objStLaser->setSource(source);
          }
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }
    {
      // ObjCrLaser
      if (auto objCrLaser = std::dynamic_pointer_cast<ObjCrLaser>(obj)) {
        if (ImGui::CollapsingHeader("ObjCrLaser")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          {
            float tipDecrement = objCrLaser->getTipDecrement();
            SliderFloatRow("tip-decrement", "##crLaserTipDecrement", &tipDecrement, 0.0f, 1.0f);
            objCrLaser->setTipDecrement(tipDecrement);
          }
          ImGui::Separator();
          ViewIntRow("laser-node-count", objCrLaser->getLaserNodeCount());
          ImGui::Columns(1);
          ImGui::Separator();
        }
      }
    }
    {
      // ObjEnemy
      if (auto objEnemy = std::dynamic_pointer_cast<ObjEnemy>(obj)) {
        if (ImGui::CollapsingHeader("ObjEnemy")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewBoolRow("registered", objEnemy->isRegistered());
          ImGui::Separator();
          ViewBoolRow("is-boss", objEnemy->isBoss());
          ImGui::Separator();
          {
            double life = objEnemy->getLife();
            InputDoubleRow("life", "##enemyLife", &life);
            objEnemy->setLife(life);
          }
          ImGui::Separator();
          {
            float damageRateShot = (float)objEnemy->getDamageRateShot();
            SliderFloatRow("damage-rate-shot", "##enemyDamageRateShot", &damageRateShot, 0.0f, 100.0f);
            objEnemy->setDamageRateShot((double)damageRateShot);
          }
          ImGui::Separator();
          {
            float damageRateSpell = (float)objEnemy->getDamageRateSpell();
            SliderFloatRow("damage-rate-spell", "##enemyDamageRateSpell", &damageRateSpell, 0.0f, 100.0f);
            objEnemy->setDamageRateSpell((double)damageRateSpell);
          }
          ImGui::Separator();
          ViewIntRow("prev-frame-shot-hit-count", objEnemy->getPrevFrameShotHitCount());
          ImGui::Columns(1);
          ImGui::Separator();
          const auto& tempIsects = objEnemy->getTempIntersections();
          if (ImGui::TreeNode("temporary intersections##enemyTempIntersections")) {
            int isectId = 0;
            for (const auto& isect : tempIsects) {
              ImGui::PushID(isectId++);
              if (ImGui::TreeNode("intersection##enemyTempIntersection")) {
                drawIntersectionInfo(isect);
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
      if (auto objSpell = std::dynamic_pointer_cast<ObjSpell>(obj)) {
        if (ImGui::CollapsingHeader("ObjSpell")) {
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewBoolRow("registered", objSpell->isRegistered());
          ImGui::Separator();
          {
            double damage = objSpell->getDamage();
            InputDoubleRow("damage", "##spellDamage", &damage);
            objSpell->setDamage(damage);
          }
          ImGui::Separator();
          {
            bool eraseShot = objSpell->isEraseShotEnabled();
            CheckboxRow("erase-shot", "##spellEraseShot", &eraseShot);
            objSpell->setEraseShotEnable(eraseShot);
          }
          ImGui::Columns(1);
          ImGui::Separator();
          const auto& tempIsects = objSpell->getTempIntersections();
          if (ImGui::TreeNode("temporary intersections##spellTempIntersections")) {
            int isectId = 0;
            for (const auto& isect : tempIsects) {
              ImGui::PushID(isectId++);
              if (ImGui::TreeNode("intersection##spellTempIntersection")) {
                drawIntersectionInfo(isect);
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
      if (auto objItem = std::dynamic_pointer_cast<ObjItem>(obj)) {
        if (ImGui::CollapsingHeader("ObjItem")) {
          const auto& itemData = objItem->getItemData();
          ImGui::Columns(2);
          ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
          ImGui::Separator();
          ImGui::Separator();
          ViewTextRow("item-type", getItemTypeName(objItem->getItemType()));
          ImGui::Separator();
          {
            int64_t score = objItem->getScore();
            InputInt64Row("score", "##itemScore", &score);
            objItem->setScore(score);
          }
          ImGui::Separator();
          {
            bool renderScore = objItem->isRenderScoreEnabled();
            CheckboxRow("render-score", "##itemRenderScore", &renderScore);
            objItem->setRenderScoreEnable(renderScore);
          }
          ImGui::Separator();
          {
            bool autoCollect = objItem->isAutoCollectEnabled();
            CheckboxRow("auto-collect-enable", "##itemAutoCollectEnable", &autoCollect);
            objItem->setAutoCollectEnable(autoCollect);
          }
          if (itemData && !itemData->animationData.empty()) {
            ImGui::Separator();
            ViewIntRow("animation-frame-count", objItem->getAnimationFrameCount());
            ImGui::Separator();
            ViewIntRow("animation-index", objItem->getAnimationIndex());
          }
          ImGui::Separator();
          {
            ViewBoolRow("auto-collected", objItem->isAutoCollected());
          }
          ImGui::Separator();
          {
            ViewBoolRow("obtained", objItem->isObtained());
          }
          ImGui::Columns(1);
          ImGui::Separator();
          if (itemData) {
            if (ImGui::TreeNode("item-data##itemItemData")) {
              drawItemDataInfo(itemData);
              ImGui::TreePop();
            }
          }
          const auto& isects = objItem->getIntersections();
          if (ImGui::TreeNode("intersections##itemIntersections")) {
            int isectId = 0;
            for (const auto& isect : isects) {
              ImGui::PushID(isectId++);
              if (ImGui::TreeNode("intersection##itemIntersection")) {
                drawIntersectionInfo(isect);
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
  void Engine::backDoor<ObjectBrowser>() {
    const auto& table = gameState->objTable->getAll();
    static int selectedId = 0;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("SideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : table) {
      auto id = entry.first;
      ImGui::PushID(id);
      const auto& obj = entry.second;
      auto type = obj->getType();
      auto nameProp = obj->getValue(L"name");
      std::string name;
      if (nameProp->getType() != DnhValue::Type::NIL) {
        name = toUTF8(nameProp->toString());
      } else {
        name = getObjTypeName(obj->getType());
      }
      std::string label = name;
      if (ImGui::Selectable(label.c_str(), selectedId == id)) {
        selectedId = id;
      }
      ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("InfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Object Info");
    auto it = table.find(selectedId);
    if (it != table.end()) {
      drawObjEditArea(it->second, gameState->objLayerList);
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

  ObjectBrowser::~ObjectBrowser() { }

  void ObjectBrowser::draw(const std::shared_ptr<Engine>& engine) {
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Object", &openFlag)) {
      engine->backDoor<ObjectBrowser>();
    }
    ImGui::End();
  }
}
