#pragma once

#include <windows.h>

class IRenderer {
 public:
  IRenderer() = default;
  virtual ~IRenderer() = default;
  
  virtual void OnInitialize(HWND hwnd, UINT WindowWidth, UINT WindowHeight) = 0;
  virtual void OnRender() = 0;
  virtual void OnResize(UINT WindowWidth, UINT WindowHeight) = 0;
  virtual void OnUpdate() = 0;
  virtual void OnDestroy() = 0;
};