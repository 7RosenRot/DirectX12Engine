#pragma once

#include <string>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_internal.h>

class IRenderer;
class Scene;

class EngineUI {
 public:
  EngineUI(HWND hwnd, IRenderer* pRenderer, Scene* pScene);
  ~EngineUI();

  void NewFrame();
  void Draw();
  void UpdateLayout();
 private:
  HWND m_hwnd;

  UINT m_ViewportWidth  = 1;
  UINT m_ViewportHeight = 1;
  bool m_SizeChanged = true;

  ImGuiWindowFlags m_WindowFlags = {
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
  };

  IRenderer* m_pRenderer;
  Scene* m_pScene;

  std::string m_SelectedModel = "";

  void DrawDockSpace();
  void DrawViewportUI();
  void DrawBrowserUI();
  void DrawPropertiesUI();
};