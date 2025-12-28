#pragma once
#include <Include/Window/Window.hpp>
#include <Include/Graphics/InterfaceDirectX12.hpp>

namespace D3D12Engine {
  class DirectX12Graphics : public InterfaceDirectX12 {
  public:
    DirectX12Graphics(UINT m_WindowWidth, UINT m_WindowHeight, std::wstring m_WindowName);
    virtual ~DirectX12Graphics();

    // Методы, пергруженные из InterfaceDirectX12 - вызываются в Window. Обеспечивает расширяемость - можем добавить Vulkan API, OpenGL API
    // Не позволяем взаимодействовоать с методами, описывающими DirectX 12 API напрямую -> допуск осуществляется через public методы
    void OnInitialize() override;
    void OnRender() override;
    void OnUpdate() override;
    void OnDestroy() override;
  private:
    static const UINT FrameCount = 2;
    UINT FrameIndex;
    UINT RenderTargetViewDescriptorSize;

    Microsoft::WRL::ComPtr<ID3D12Device> Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeapDescriptor;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

    void LoadPipeline();
    void LoadAssets();
    void FillCommandList();
    void WaitForPreviousFrame();
  };
}
