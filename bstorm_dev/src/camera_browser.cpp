#include <imgui.h>
#include <bstorm/camera2D.hpp>
#include <bstorm/camera3D.hpp>
#include <bstorm/package.hpp>
#include <bstorm/engine.hpp>

#include "util.hpp"
#include "camera_browser.hpp"

namespace bstorm
{
static void drawCamera2DInfo(Camera2D& camera2D)
{
    ImGui::Text("2D camera info");
    ImGui::Separator();
    ImGui::Columns(2);
    ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Separator();
    {
        float focusX = camera2D.GetX();
        InputFloatRow("focus-x", "##camera2DFocusX", &focusX);
        camera2D.SetFocusX(focusX);
    }
    ImGui::Separator();
    {
        float focusY = camera2D.GetY();
        InputFloatRow("focus-y", "##camera2DFocusY", &focusY);
        camera2D.SetFocusY(focusY);
    }
    ImGui::Separator();
    {
        float angleZ = camera2D.GetAngleZ();
        DragAngleRow("angle-z", "##camera2DAngleZ", &angleZ);
        camera2D.SetAngleZ(angleZ);
    }
    ImGui::Separator();
    {
        float ratioX = camera2D.GetRatioX();
        InputFloatRow("ratio-x", "##camera2DRatioX", &ratioX);
        camera2D.SetRatioX(ratioX);
    }
    ImGui::Separator();
    {
        float ratioY = camera2D.GetRatioY();
        InputFloatRow("ratio-y", "##camera2DRatioY", &ratioY);
        camera2D.SetRatioY(ratioY);
    }
    ImGui::Columns(1);
    ImGui::Separator();
}

static void drawCamera3DInfo(Camera3D& camera3D)
{
    ImGui::Text("3D camera info");
    ImGui::Separator();
    ImGui::Columns(2);
    ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Separator();
    {
        float x = camera3D.GetX();
        ViewFloatRow("x", x);
    }
    ImGui::Separator();
    {
        float y = camera3D.GetY();
        ViewFloatRow("y", y);
    }
    ImGui::Separator();
    {
        float z = camera3D.GetZ();
        ViewFloatRow("z", z);
    }
    ImGui::Separator();
    {
        float focusX = camera3D.GetFocusX();
        InputFloatRow("focus-x", "##camera3DFocusX", &focusX);
        camera3D.SetFocusX(focusX);
    }
    ImGui::Separator();
    {
        float focusY = camera3D.GetFocusY();
        InputFloatRow("focus-y", "##camera3DFocusY", &focusY);
        camera3D.SetFocusY(focusY);
    }
    ImGui::Separator();
    {
        float focusZ = camera3D.GetFocusZ();
        InputFloatRow("focus-z", "##camera3DFocusZ", &focusZ);
        camera3D.SetFocusZ(focusZ);
    }
    ImGui::Separator();
    {
        float radius = camera3D.GetRadius();
        InputFloatRow("radius", "##camera3DRadious", &radius);
        camera3D.SetRadius(radius);
    }
    ImGui::Separator();
    {
        float azimuthAngle = camera3D.GetAzimuthAngle();
        DragAngleRow("azimuth-angle", "##camera3DAzimuthAngle", &azimuthAngle);
        camera3D.SetAzimuthAngle(azimuthAngle);
    }
    ImGui::Separator();
    {
        float elevationAngle = camera3D.GetElevationAngle();
        DragAngleRow("elevation-angle", "##camera3DElevationAngle", &elevationAngle);
        camera3D.SetElevationAngle(elevationAngle);
    }
    ImGui::Separator();
    {
        float yaw = camera3D.GetYaw();
        DragAngleRow("yaw", "##camera3DYaw", &yaw);
        camera3D.SetYaw(yaw);
    }
    ImGui::Separator();
    {
        float pitch = camera3D.GetPitch();
        DragAngleRow("pitch", "##camera3DPitch", &pitch);
        camera3D.SetPitch(pitch);
    }
    ImGui::Separator();
    {
        float roll = camera3D.GetRoll();
        DragAngleRow("roll", "##camera3DRoll", &roll);
        camera3D.SetRoll(roll);
    }
    ImGui::Separator();
    {
        float clips[2] = { camera3D.GetNearPerspectiveClip(), camera3D.GetFarPerspectiveClip() };
        InputFloat2Row("clip", "##camera3DClip", clips);
        camera3D.SetPerspectiveClip(clips[0], clips[1]);
    }
    ImGui::Columns(1);
    ImGui::Separator();
}

template <>
void Engine::backDoor<CameraBrowser>()
{
    ImGui::Columns(2, "camera tab");
    ImGui::Separator();
    static int selectedCamera = 0; // 0: 2D, 1: 3D
    if (ImGui::Selectable("Camera2D##Camera2DTab", selectedCamera == 0))
    {
        selectedCamera = 0;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Camera3D##Camera3DTab", selectedCamera == 1))
    {
        selectedCamera = 1;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (selectedCamera == 0)
    {
        drawCamera2DInfo(*package->camera2D);
    } else
    {
        drawCamera3DInfo(*package->camera3D);
    }
}

CameraBrowser::CameraBrowser(int left, int top, int width, int height) :
    iniLeft(left),
    iniTop(top),
    iniWidth(width),
    iniHeight(height),
    openFlag(false)
{
}

CameraBrowser::~CameraBrowser() {}

void CameraBrowser::draw(const std::shared_ptr<Engine>& engine)
{
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Camera", &openFlag, ImGuiWindowFlags_ResizeFromAnySide))
    {
        engine->backDoor<CameraBrowser>();
    }
    ImGui::End();
}
}
