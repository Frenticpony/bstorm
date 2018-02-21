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

#include "../../version.hpp"
#include "../resource.h"

using namespace bstorm;

static bool isLostFocus = false;

constexpr char* thDnhDefFilePath = "th_dnh.def";
constexpr bool useBinaryFormat = true;
constexpr char* configFilePath = useBinaryFormat ? "config.dat" : "config.json";
constexpr wchar_t* logFilePath = L"th_dnh.log";

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
  LONG windowWidth = 640;
  LONG windowHeight = 480;
  LONG screenWidth = 640;
  LONG screenHeight = 480;
  std::wstring windowTitle = L"bstorm "  BSTORM_VERSION_W;
  std::wstring packageMainScriptPath;
  Logger::Init(std::make_shared<DummyLogger>());

  HWND hWnd = NULL;
  MSG msg;
  try {
    conf::BstormConfig config = loadBstormConfig(configFilePath, useBinaryFormat, IDR_HTML1);

    if (config.options.saveLogFile) {
      try {
        Logger::Init(std::make_shared<FileLogger>(logFilePath, nullptr));
      } catch (Log& log) {
        OutputDebugStringA(log.toString().c_str());
      }
    }

    windowWidth = config.windowConfig.windowWidth;
    windowHeight = config.windowConfig.windowHeight;

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
    windowClass.lpszClassName = L"bstorm";
    windowClass.hIconSm = NULL;

    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, windowWidth, windowHeight };
    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&windowRect, windowStyle, FALSE);
    hWnd = CreateWindowEx(0, windowClass.lpszClassName, windowTitle.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
    ValidateRect(hWnd, NULL);
    ShowWindow(hWnd, SW_RESTORE);
    UpdateWindow(hWnd);

    auto engine = std::make_shared<Engine>(hWnd, screenWidth, screenHeight, std::make_shared<conf::KeyConfig>(config.keyConfig));

    if (packageMainScriptPath.empty()) {
      throw Log(Log::Level::LV_ERROR).setMessage("package main script not specified.");
    }

    engine->setPackageMainScript(packageMainScriptPath);
    engine->startPackage();

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
            throw Log(Log::Level::LV_ERROR).setMessage("graphic device internal error occured.");
            break;
        }
      }
    }
  } catch (Log& log) {
    Logger::WriteLog(log);
    MessageBoxW(hWnd, toUnicode(log.toString()).c_str(), L"Engine Error", MB_OK);
  } catch (const std::exception& e) {
    Logger::WriteLog(Log::Level::LV_ERROR, e.what());
    MessageBoxW(hWnd, toUnicode(e.what()).c_str(), L"Unexpected Error", MB_OK);
  }
  Logger::Shutdown();
  return (int)msg.wParam;
}