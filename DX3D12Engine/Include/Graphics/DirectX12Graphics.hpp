#pragma once
#include <Include/Window/Window.hpp>
#include <Include/Graphics/InterfaceDirectX12.hpp>

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <exception>
#include <stdexcept>

namespace D3D12Engine {
  class DirectX12Graphics : public InterfaceDirectX12 {
  public:
    DirectX12Graphics(UINT WindowWidth, UINT WindowHeight, std::wstring WindowName);
    virtual ~DirectX12Graphics();

    // Методы, пергруженные из InterfaceDirectX12 - вызываются в Window. Обеспечивает расширяемость - можем добавить Vulkan API, OpenGL API
    // Не позволяем взаимодействовоать с методами, описывающими DirectX 12 API напрямую -> допуск осуществляется через public методы
    void OnInitialize() override;
    void OnRender() override;
    void OnUpdate() override;
    void OnDestroy() override;
  private:
    // DXGI (DirectX Graphics Infrastructure) - отвечает за общие вещи для всех версии DirectX: Окна, мониторы, форматы пикселей.
    // D3D12 - отвчеает только за отрисовку примитивов (треугольники, шейдеры, вычисления)

    struct m_Vertex {
      DirectX::XMFLOAT3 position;
      DirectX::XMFLOAT4 color;
    };

    static const UINT m_frameCount{2};                                    // Количесвто буферов отрисовки
    UINT m_frameIndex{0};                                                 // Индекс текущего буфера
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;                  // Цепочка из буферов
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameCount]; // Массив из буферов
    
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;                        // Логичесвкое представление GPU
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;                // Очередь (список) команд GPU
    
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;     // Таблица RTV-дескрипторов
    UINT m_rtvDescriptorSize;                                             // Размер RTV-дескриптора - получаем из m_device, так как там храниться информация о драйвере
    
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;        // Память для команд

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;
    
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
    HANDLE m_fenceEvent;
    
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    CD3DX12_RESOURCE_BARRIER m_transitionBarier{};
    CD3DX12_VIEWPORT m_viewPort{};
    CD3DX12_RECT m_scissorRect{};

    void loadPipeline();
    void loadAssets();
    void fillCommandList();
    void waitForPreviousFrame();
  };
}
