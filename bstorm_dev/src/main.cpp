#include <crtdbg.h>
#include <string>

#include <bstorm/logger.hpp>
#include <bstorm/engine.hpp>
#include <bstorm/util.hpp>
#include <bstorm/input_device.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/config.hpp>
#include <bstorm/th_dnh_def.hpp>

#include <imgui.h>
#include "../../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include <IconsFontAwesome_c.h>
#include "../../glyph_ranges_ja.hpp"
#include "../../version.hpp"
#include "../resource.h"
#include "log_window.hpp"
#include "script_explorer.hpp"
#include "resource_monitor.hpp"
#include "common_data_browser.hpp"
#include "user_def_data_browser.hpp"
#include "camera_browser.hpp"
#include "object_browser.hpp"
#include "game_view.hpp"
#include "play_controller.hpp"

#include "file_logger.hpp"

using namespace bstorm;

static bool isLostFocus = false;

constexpr char* thDnhDefFilePath = "th_dnh_dev.def";
constexpr bool useBinaryFormat = true;
constexpr char* configFilePath = useBinaryFormat ? "config.dat" : "config.json";

extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

constexpr wchar_t* GAME_VIEW_RENDER_TARGET = L"___GAME_VIEW_RENDER_TARGET___";

static LRESULT WINAPI windowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) return true;
  switch (msg) {
    case WM_SYSCOMMAND:
      if ((wp & 0xfff0) == SC_KEYMENU) {
        return 0;
      }
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_SETFOCUS:
      isLostFocus = false;
      return 0;
    case WM_KILLFOCUS:
      isLostFocus = true;
      return 0;
  }
  return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  LONG windowWidth = 1600;
  LONG windowHeight = 900;
  LONG screenWidth = 640;
  LONG screenHeight = 480;
  std::wstring windowTitle = L"bstorm [dev] "  BSTORM_VERSION_W;
  std::wstring packageMainScriptPath;
  std::shared_ptr<Logger> logger;

  auto logWindow = std::make_shared<LogWindow>();
  logger = logWindow;

  HWND hWnd = NULL;
  MSG msg;
  try {
    conf::BstormConfig config = loadBstormConfig(configFilePath, useBinaryFormat, IDR_HTML1);

    {
      // th_dnh.def読み込み
      ThDnhDef def = loadThDnhDef(thDnhDefFilePath);
      packageMainScriptPath = def.packageScriptMain;
      if (!def.windowTitle.empty()) windowTitle = def.windowTitle;
      screenWidth = def.screenWidth;
      screenHeight = def.screenHeight;
    }

    /* create window */
    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = L"bstorm_dev";
    windowClass.hIconSm = NULL;

    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, windowWidth, windowHeight };
    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&windowRect, windowStyle, FALSE);
    hWnd = CreateWindowEx(0, windowClass.lpszClassName, windowTitle.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
    ValidateRect(hWnd, NULL);
    ShowWindow(hWnd, SW_RESTORE);
    UpdateWindow(hWnd);

    auto scriptExplorer = std::make_shared<ScriptExplorer>(windowWidth * 990 / 1280, 19, windowWidth * (1280 - 990) / 1280, windowHeight - 19);
    logWindow->setInitWindowPos(0, windowHeight * 550 / 720, windowWidth * 990 / 1280, windowHeight * 172 / 720);
    auto resourceMonitor = std::make_shared<ResourceMonitor>(0, 19, 300, 530);
    auto commonDataBrowser = std::make_shared<CommonDataBrowser>(300, 19, 300, 530);
    auto userDefDataBrowser = std::make_shared<UserDefDataBrowser>(600, 19, 300, 530);
    auto cameraBrowser = std::make_shared<CameraBrowser>(0, 39, 300, 530);
    auto objectBrowser = std::make_shared<ObjectBrowser>(0, 39, 300, 530);
    auto engine = std::make_shared<Engine>(hWnd, screenWidth, screenHeight, logWindow, std::make_shared<conf::KeyConfig>(config.keyConfig));
    auto playController = std::make_shared<PlayController>(engine);
    auto gameView = std::make_shared<GameView>(windowWidth / 2 - 320, 60, 640, 480, playController);
    engine->setScreenPos(gameView->getViewPosX(), gameView->getViewPosY());
    engine->setGameViewSize(gameView->getViewWidth(), gameView->getViewHeight());

    ImGui_ImplDX9_Init(hWnd, engine->getGraphicDevice());

    {
      // フォント設定
      ImGuiIO& io = ImGui::GetIO();
      io.Fonts->AddFontDefault();
      ImFontConfig fontConfig;
      fontConfig.MergeMode = true;
      io.Fonts->AddFontFromFileTTF("fonts/ja/ipagp.ttf", 12, &fontConfig, glyphRangesJapanese);
      static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
      io.Fonts->AddFontFromFileTTF("fonts/fa/fontawesome-webfont.ttf", 13.0f, &fontConfig, icon_ranges);
    }

    {
      // スタイル設定
      ImGuiStyle& imGuiStyle = ImGui::GetStyle();
      ImGui::StyleColorsDark();
      imGuiStyle.FrameRounding = 3.0f;
    }

    /* message loop */
    while (true) {
      auto d3DDevice = engine->getGraphicDevice();
      if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        BOOL result = GetMessage(&msg, NULL, 0, 0);
        if ((!result) || (!(~result))) {
          break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        continue;
      }
      if (isLostFocus) {
        Sleep(1);
        continue;
      }
      if (SUCCEEDED(d3DDevice->BeginScene())) {
        // NOTE : PlayController, 及びPlayControllerを使っているモジュールは他より先に描画(tickでテクスチャの解放が行われる可能性があるので)
        engine->setBackBufferRenderTarget();
        d3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(114, 144, 154), 1.0f, 0);
        engine->updateFpsCounter();
        ImGui_ImplDX9_NewFrame();
        playController->setScript(scriptExplorer->getSelectedMainScript(), scriptExplorer->getSelectedPlayerScript());
        if (!playController->isPaused()) {
          playController->tick();
        }
        auto gameViewRenderTarget = engine->getRenderTarget(GAME_VIEW_RENDER_TARGET);
        if (!gameViewRenderTarget) {
          gameViewRenderTarget = engine->createRenderTarget(GAME_VIEW_RENDER_TARGET, screenWidth, screenHeight);
        }
        engine->render(GAME_VIEW_RENDER_TARGET);
        gameView->draw(gameViewRenderTarget);
        {
          // main menu bar
          if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Tool")) {
              if (ImGui::MenuItem("Resource", NULL, resourceMonitor->isOpened())) {
                resourceMonitor->setOpen(!resourceMonitor->isOpened());
              }
              if (ImGui::MenuItem("Common Data", NULL, commonDataBrowser->isOpened())) {
                commonDataBrowser->setOpen(!commonDataBrowser->isOpened());
              }
              if (ImGui::MenuItem("User Defined Data", NULL, userDefDataBrowser->isOpened())) {
                userDefDataBrowser->setOpen(!userDefDataBrowser->isOpened());
              }
              if (ImGui::MenuItem("Camera", NULL, cameraBrowser->isOpened())) {
                cameraBrowser->setOpen(!cameraBrowser->isOpened());
              }
              if (ImGui::MenuItem("Object", NULL, objectBrowser->isOpened())) {
                objectBrowser->setOpen(!objectBrowser->isOpened());
              }
              ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
          }
        }
        logWindow->draw();
        scriptExplorer->draw();
        resourceMonitor->draw(engine);
        commonDataBrowser->draw(engine);
        userDefDataBrowser->draw(engine);
        cameraBrowser->draw(engine);
        objectBrowser->draw(engine);
#ifdef _DEBUG
        ImGui::ShowDemoWindow();
        ImGui::ShowMetricsWindow();
        ImGui::ShowStyleEditor();
        ImGui::ShowStyleSelector("style selector");
        ImGui::ShowFontSelector("font selector");
        ImGui::ShowUserGuide();
#endif
        ImGui::Render();
        d3DDevice->EndScene();
        switch (d3DDevice->Present(NULL, NULL, NULL, NULL)) {
          case D3DERR_DEVICELOST:
            if (d3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
              engine->releaseLostableGraphicResource();
              ImGui_ImplDX9_InvalidateDeviceObjects();
              engine->resetGraphicDevice();
              engine->restoreLostableGraphicDevice();
              ImGui_ImplDX9_CreateDeviceObjects();
            } else {
              Sleep(1);
            }
            break;
          case D3DERR_DRIVERINTERNALERROR:
            throw std::runtime_error("graphic device internal error occured.");
            break;
        }
      }
    }
  } catch (const std::exception& e) {
    logger->logError(e.what());
    MessageBoxW(hWnd, toUnicode(e.what()).c_str(), L"Error", MB_OK);
  }
  ImGui_ImplDX9_Shutdown();
  return (int)msg.wParam;
}
