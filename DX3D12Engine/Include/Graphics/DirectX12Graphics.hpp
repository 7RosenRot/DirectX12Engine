#pragma once
#include <Include/Window/Window.hpp>
#include <Include/Graphics/InterfaceDirectX12.hpp>

namespace D3D12Engine {
  class DirectX12Graphics : InterfaceDirectX12 {
  public:
    DirectX12Graphics(UINT m_WindowWidth, UINT m_WindowHeight, std::wstring m_WindowName);
    virtual ~DirectX12Graphics();

    // Методы, пергруженные из InterfaceDirectX12 - вызываются в Window. Обеспечивает расширяемость - можем добавить Vulkan API, OpenGL API
    virtual void OnInitialize() override;
    virtual void OnRender() override;
    virtual void OnUpdate() override;
    virtual void OnDestroy() override;
  private:
    void LoadPipeline();
    void LoadAssets();
    void FillCommandList();
    void WaitForPreviousFrame();
  };
}
