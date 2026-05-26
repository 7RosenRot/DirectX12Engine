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

  bool Initialize();
 private:
}