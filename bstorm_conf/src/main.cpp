#include <windows.h>
#include <string>
#include <cstdio>
#include <clocale>
#include <d3d9.h>

#include <imgui.h>
#include "../../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include "../../glyph_ranges_ja.hpp"

/* window procedure */
extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI windowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) return true;
  switch (msg) {
    case WM_SYSCOMMAND:
      if ((wp & 0xfff0) == SC_KEYMENU)
        return 0;
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWnd, msg, wp, lp);
}

constexpr DWORD screenWidth = 480;
constexpr DWORD screenHeight = 360;
constexpr wchar_t* windowTitle = L"bstorm config";
constexpr char* jaFontPath = "fonts/ipagp.ttf";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  std::setlocale(LC_ALL, "C");

  /* create window */
  WNDCLASSEX windowClass;
  windowClass.cbSize = sizeof(WNDCLASSEX);
  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = windowProc;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = GetModuleHandle(NULL);
  windowClass.hIcon = LoadIcon(NULL, IDC_ICON);
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.hbrBackground = NULL;
  windowClass.lpszMenuName = NULL;
  windowClass.lpszClassName = L"BSTORM CONFIG";
  windowClass.hIconSm = NULL;

  RegisterClassEx(&windowClass);

  RECT windowRect = { 0, 0, screenWidth, screenHeight };
  DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&windowRect, windowStyle, FALSE);
  HWND hWnd = CreateWindowEx(0, windowClass.lpszClassName, windowTitle, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
  ValidateRect(hWnd, NULL);
  ShowWindow(hWnd, SW_RESTORE);
  UpdateWindow(hWnd);

  /* get direct3D device */
  IDirect3D9* d3D = Direct3DCreate9(D3D_SDK_VERSION);
  D3DPRESENT_PARAMETERS presentParams = {
    screenWidth,
    screenHeight,
    D3DFMT_UNKNOWN,
    1,
    D3DMULTISAMPLE_NONE,
    0,
    D3DSWAPEFFECT_DISCARD,
    hWnd,
    TRUE,
    TRUE,
    D3DFMT_D24S8,
    0,
    D3DPRESENT_RATE_DEFAULT,
    D3DPRESENT_INTERVAL_ONE
  };

  IDirect3DDevice9* d3DDevice = NULL;
  if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &presentParams, &d3DDevice))) {
    if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &presentParams, &d3DDevice))) {
      if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &presentParams, &d3DDevice))) {
        d3D->Release();
        return 1;
      }
    }
  }

  /* init imgui */
  ImGui_ImplDX9_Init(hWnd, d3DDevice);
  ImGuiIO& io = ImGui::GetIO();

  // prevent imgui.ini generation
  io.IniFilename = NULL;

  // imgui font setting
  ImFontConfig config;
  config.MergeMode = true;
  io.Fonts->AddFontDefault();
  if (FILE* fp = fopen(jaFontPath, "rb")) {
    fclose(fp);
    io.Fonts->AddFontFromFileTTF(jaFontPath, 12, &config, glyphRangesJapanese);
  }


  // imgui style
  {
    auto& style = ImGui::GetStyle();
    style.FrameRounding = 3.0f;
    ImGui::StyleColorsDark();
  }

  /* message loop */
  MSG msg;
  while (true) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
      BOOL result = GetMessage(&msg, NULL, 0, 0);
      if ((!result) || (!(~result))) {
        // exit or error
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }

    // draw
    if (SUCCEEDED(d3DDevice->BeginScene())) {
      ImGui_ImplDX9_NewFrame();
      d3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(114, 144, 154), 1.0f, 0);
      ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Once);
      ImGui::SetNextWindowSize(ImVec2((float)screenWidth, (float)screenHeight), ImGuiSetCond_Once);
      ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove;
      ImGui::Begin("main window", NULL, windowFlag);
      ImGui::End();
      ImGui::Render();
      d3DDevice->EndScene();
      switch (d3DDevice->Present(NULL, NULL, NULL, NULL)) {
        case D3DERR_DEVICELOST:
          if (d3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            if (FAILED(d3DDevice->Reset(&presentParams))) {
              PostQuitMessage(0);
            }
            ImGui_ImplDX9_CreateDeviceObjects();
          }
          break;
        case D3DERR_DRIVERINTERNALERROR:
          PostQuitMessage(0);
          break;
      }
    }
  }

  // clean
  ImGui_ImplDX9_Shutdown();
  d3DDevice->Release();
  d3D->Release();
  UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
  return msg.wParam;
}
