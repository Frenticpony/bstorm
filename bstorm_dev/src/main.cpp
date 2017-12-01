#include <crtdbg.h>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>

#include <bstorm/logger.hpp>
#include <bstorm/engine.hpp>
#include <bstorm/util.hpp>
#include <bstorm/input_device.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/render_target.hpp>

#include <imgui.h>
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include <IconsFontAwesome_c.h>
#include "glyph_ranges_ja.hpp"
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

bool g_isLostFocus = false;

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
      g_isLostFocus = false;
      return 0;
    case WM_KILLFOCUS:
      g_isLostFocus = true;
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
  std::wstring version = L"a1.0.4";
  std::wstring windowTitle = L"Bullet storm [" + version + L"]";
  std::wstring packageMainScriptPath;
  std::shared_ptr<Logger> logger;

  auto logWindow = std::make_shared<LogWindow>();
  logger = logWindow;

  // 設定ファイル読み込み
  {
    std::ifstream thDnhDefFile;
    std::string defFileName = "th_dnh_dev.def";
    thDnhDefFile.open(defFileName, std::ios::in);
    if (thDnhDefFile.good()) {
      std::string line;
      while (std::getline(thDnhDefFile, line)) {
        if (line.empty()) continue;
        std::smatch match;
        if (regex_match(line, match, std::regex(R"(window.width\s*=\s*(\d+))"))) {
          windowWidth = std::atoi(match[1].str().c_str());
          logger->logInfo(defFileName + ": window width = " + std::to_string(windowWidth) + ".");
        } else if (regex_match(line, match, std::regex(R"(window.height\s*=\s*(\d+))"))) {
          windowHeight = std::atoi(match[1].str().c_str());
          logger->logInfo(defFileName + ": window.height = " + std::to_string(windowHeight) + ".");
        } else if (regex_match(line, match, std::regex(R"(screen.width\s*=\s*(\d+))"))) {
          screenWidth = std::atoi(match[1].str().c_str());
          logger->logInfo(defFileName + ": screen.width = " + std::to_string(screenWidth) + ".");
        } else if (regex_match(line, match, std::regex(R"(screen.height\s*=\s*(\d+))"))) {
          screenHeight = std::atoi(match[1].str().c_str());
          logger->logInfo(defFileName + ": screen.height = " + std::to_string(screenHeight) + ".");
        } else if (regex_match(line, match, std::regex(R"(window.title\s*=\s*(.*))"))) {
          windowTitle = fromMultiByte<932>(match[1].str());
          logger->logInfo(defFileName + ": window.title = " + toUTF8(windowTitle) + ".");
        } else if (regex_match(line, match, std::regex(R"(package.script.main\s*=\s*(.*))"))) {
          packageMainScriptPath = fromMultiByte<932>(match[1].str());
          logger->logInfo(defFileName + ": package.script.main = " + toUTF8(packageMainScriptPath) + ".");
        }
      }
      thDnhDefFile.close();
    } else {
      logger->logWarn(defFileName + " not found.");
    }
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
  windowClass.lpszClassName = L"bstorm";
  windowClass.hIconSm = NULL;

  RegisterClassEx(&windowClass);

  RECT windowRect = { 0, 0, windowWidth, windowHeight };
  DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&windowRect, windowStyle, FALSE);
  HWND hWnd = CreateWindowEx(0, windowClass.lpszClassName, windowTitle.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
  ValidateRect(hWnd, NULL);
  ShowWindow(hWnd, SW_RESTORE);
  UpdateWindow(hWnd);

  MSG msg;
  std::shared_ptr<KeyConfig> masterKeyConfig = std::make_shared<KeyConfig>();
  try {
    masterKeyConfig->addVirtualKey(VK_LEFT, KEY_LEFT, 0);
    masterKeyConfig->addVirtualKey(VK_RIGHT, KEY_RIGHT, 1);
    masterKeyConfig->addVirtualKey(VK_UP, KEY_UP, 2);
    masterKeyConfig->addVirtualKey(VK_DOWN, KEY_DOWN, 3);
    masterKeyConfig->addVirtualKey(VK_SHOT, KEY_Z, 5);
    masterKeyConfig->addVirtualKey(VK_SPELL, KEY_X, 6);
    masterKeyConfig->addVirtualKey(VK_OK, KEY_Z, 5);
    masterKeyConfig->addVirtualKey(VK_CANCEL, KEY_X, 6);
    masterKeyConfig->addVirtualKey(VK_SLOWMOVE, KEY_LSHIFT, 7);
    masterKeyConfig->addVirtualKey(VK_USER1, KEY_C, 8);
    masterKeyConfig->addVirtualKey(VK_USER2, KEY_V, 9);
    masterKeyConfig->addVirtualKey(VK_PAUSE, KEY_ESCAPE, 10);

    auto scriptExplorer = std::make_shared<ScriptExplorer>(windowWidth * 990 / 1280, 19, windowWidth * (1280 - 990) / 1280, windowHeight - 19);
    logWindow->setInitWindowPos(0, windowHeight * 550 / 720, windowWidth * 990 / 1280, windowHeight * 172 / 720);
    auto resourceMonitor = std::make_shared<ResourceMonitor>(0, 19, 300, 530);
    auto commonDataBrowser = std::make_shared<CommonDataBrowser>(300, 19, 300, 530);
    auto userDefDataBrowser = std::make_shared<UserDefDataBrowser>(600, 19, 300, 530);
    auto cameraBrowser = std::make_shared<CameraBrowser>(0, 39, 300, 530);
    auto objectBrowser = std::make_shared<ObjectBrowser>(0, 39, 300, 530);
    auto engine = std::make_shared<Engine>(hWnd, screenWidth, screenHeight, logWindow, masterKeyConfig);
    auto playController = std::make_shared<PlayController>(engine);
    auto gameView = std::make_shared<GameView>(windowWidth / 2 - 320, 60, 640, 480, playController);
    engine->setScreenPos(gameView->getViewPosX(), gameView->getViewPosY());
    engine->setGameViewSize(gameView->getViewWidth(), gameView->getViewHeight());

    ImGui_ImplDX9_Init(hWnd, engine->getGraphicDevice());
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.MergeMode = true;
    io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("fonts/ja/ipagp.ttf", 12, &config, glyphRangesJapanese);
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF("fonts/fa/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);

    /* message loop */
    engine->resetFpsCounter();
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
      if (g_isLostFocus) {
        Sleep(1);
        continue;
      }
      if (SUCCEEDED(d3DDevice->BeginScene())) {
        engine->updateFpsCounter();
        engine->setBackBufferRenderTarget();
        d3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(114, 144, 154), 1.0f, 0);
        // NOTE : PlayController, 及びPlayControllerを使っているモジュールは他より先に描画(tickでテクスチャの解放が行われる可能性があるので)
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
        static bool showTestWindow = true;
        ImGui::ShowTestWindow(&showTestWindow);
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
