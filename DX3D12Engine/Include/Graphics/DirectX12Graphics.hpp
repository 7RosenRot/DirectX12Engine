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

    void OnInitialize() override;
    void OnRender() override;
    void OnUpdate() override;
    void OnDestroy() override;
  private:
    struct m_Vertex {
      DirectX::XMFLOAT3 position;
      DirectX::XMFLOAT4 color;
    };

    static const UINT m_frameCount{2};
    UINT m_frameIndex{0};
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameCount];
    
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
    
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    UINT m_rtvDescriptorSize;
    
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;

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

    void LoadPipeline();
    void LoadAssets();
    void FillCommandList();
    void WaitForPreviousFrame();
  };
}
