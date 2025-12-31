#pragma once
#include <Include/Window/Window.hpp>
#include <Include/Graphics/InterfaceDirectX12.hpp>

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

    static const UINT m_frameCount{2};                                    // Количесвто буферов отрисовки
    UINT m_frameIndex{0};                                                 // Индекс текущего буфера
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;                  // Цепочка из буферов
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameCount]; // Массив из буферов
    
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;                        // Логичесвкое представление GPU
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;                // Очередь (список) команд GPU
    
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;     // Таблица RTV-дескрипторов
    UINT m_rtvDescriptorSize;                                             // Размер RTV-дескриптора - получаем из m_device, так как там храниться информация о драйвере
    
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;        // Память для команд


    void LoadPipeline();
    void LoadAssets();
    void FillCommandList();
    void WaitForPreviousFrame();
  };
}
