#pragma once

#include <memory>
#include <string>
#include <windows.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_internal.h>

#include <Application/Window/Window.hpp>
#include <Application/Input/Input.hpp>

#include <Framework/Scene/Scene.hpp>
#include <Framework/EngineUI/EngineUI.hpp>

#include <Renderer/IRenderer/IRenderer.hpp>
#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>

class Kernel {
 public:
  Kernel() = default;
  ~Kernel() = default;

  void AppInitialize(
    HINSTANCE hInstance,
    int nCmdShow,
    UINT WindowWidth,
    UINT WindowHeight,
    const std::wstring& WindowName
  );
  void AppRun();
  void AppDestroy();

  static UINT GetWindowWidth()  { return m_WindowWidth; }
  static UINT GetWindowHeight() { return m_WindowHeight; }
  static const wchar_t* GetWindowName() { return m_WindowName.c_str(); }

  float GetAspectRatio() const {
    return static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight); 
  }
  
  static IRenderer* GetRendererInstance() { return m_pRendererInstance; }
 private:
  std::unique_ptr<Window>    m_pWindow;
  std::unique_ptr<IRenderer> m_pRenderer;
  std::unique_ptr<Scene>     m_pScene;
  std::unique_ptr<EngineUI>  m_pEngineUI;

  static inline UINT m_WindowWidth = 0;
  static inline UINT m_WindowHeight = 0;
  static inline std::wstring m_WindowName = L"";

  bool m_AppRunning = false;

  static inline IRenderer* m_pRendererInstance = nullptr;
};