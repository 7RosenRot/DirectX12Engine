#pragma once

#include <memory>
#include <string>
#include <windows.h>

#include <Application/Window/Window.hpp>
#include <Renderer/IRenderer/IRenderer.hpp>

class Kernel {
 public:
  Kernel() = default;
  ~Kernel() = default;

  bool InitializeApp(
    HINSTANCE hInstance, int cmdShow, unsigned int WindowWidth, unsigned int WindowHeight, const std::wstring& WindowName
  );
  void RunApp();
  void DestroyApp();

  unsigned int GetWindowWidth() const { return m_WindowWidth; }
  unsigned int GetWindowHeight() const { return m_WindowHeight; }
  const std::wstring& GetWindowName() const { return m_WindowName; }

  float GetAspectRatio() const { return static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight); }
  
  static IRenderer* GetRendererInstance() { return m_pRendererInstance; }
 private:
  std::unique_ptr<Window> m_Window;
  std::unique_ptr<IRenderer> m_Renderer;

  unsigned int m_WindowWidth;
  unsigned int m_WindowHeight;
  std::wstring m_WindowName;

  bool m_AppRunning = false;

  static inline IRenderer* m_pRendererInstance = nullptr;
}