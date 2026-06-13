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

#include <Framework/AssetManager/AssetManager.hpp>
#include <Framework/Scene/Scene.hpp>
#include <Framework/EngineUI/EngineUI.hpp>

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
  void RenderFrame();

  static UINT GetWindowWidth()          { return m_WindowWidth; }
  static UINT GetWindowHeight()         { return m_WindowHeight; }
  static const wchar_t* GetWindowName() { return m_WindowName.c_str(); }

  float GetAspectRatio() const {
    return static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight); 
  }
  
  static D3D12Engine::DirectX12Graphics* GetRendererInstance() { return m_pRendererInstance; }
  static Kernel* GetKernelInstance() { return m_pInstance; }
 private:
  std::unique_ptr<D3D12Engine::DirectX12Graphics> m_pRenderer; 
  std::unique_ptr<AssetManager> m_pAssetManager;
  std::unique_ptr<Scene> m_pScene;
  std::unique_ptr<EngineUI> m_pEngineUI;
  std::unique_ptr<Window> m_pWindow;

  static inline UINT m_WindowWidth = 0;
  static inline UINT m_WindowHeight = 0;
  static inline std::wstring m_WindowName = L"";

  bool m_AppRunning = false;

  static inline D3D12Engine::DirectX12Graphics* m_pRendererInstance = nullptr;
  static inline Kernel* m_pInstance = nullptr;
};