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

#include "file_logger.hpp"

using namespace bstorm;

bool g_isLostFocus = false;

static LRESULT WINAPI windowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
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
  LONG windowWidth = 640;
  LONG windowHeight = 480;
  LONG screenWidth = 640;
  LONG screenHeight = 480;
  std::wstring version = L"a1.0.3";
  std::wstring windowTitle = L"Bullet storm [" + version + L"]";
  std::wstring packageMainScriptPath;
  std::shared_ptr<Logger> logger;

  try {
    logger = std::make_shared<FileLogger>(L"th_dnh.log");
  } catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    logger = std::make_shared<DummyLogger>();
  }

  // 設定ファイル読み込み
  {
    std::ifstream thDnhDefFile;
    std::string defFileName = "th_dnh.def";
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

    auto engine = std::make_shared<Engine>(hWnd, screenWidth, screenHeight, logger, masterKeyConfig);

    if (packageMainScriptPath.empty()) {
      throw std::runtime_error("package main script not specified.");
    }
    engine->setPackageMainScript(packageMainScriptPath);
    engine->startPackage();

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
        if (engine->isPackageFinished()) break;
        engine->tickFrame();
        engine->render();
        d3DDevice->EndScene();
        switch (d3DDevice->Present(NULL, NULL, NULL, NULL)) {
          case D3DERR_DEVICELOST:
            if (d3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
              engine->releaseLostableGraphicResource();
              engine->resetGraphicDevice();
              engine->restoreLostableGraphicDevice();
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
  return (int)msg.wParam;
}