#include <Renderer/D3D12Engine/Backend/RHI/Pipeline/SwapChain/SwapChain.hpp>
#include <Renderer/D3D12Engine/Backend/RHI/Resources/DepthBuffer/DepthBuffer.hpp>

void D3D12Engine::SwapChain::Initialize(
  ID3D12Device* pDevice,
  IDXGIFactory4* pFactory,
  ID3D12CommandQueue* pCmdQueue,
  HWND hwnd,
  UINT WindowWidth,
  UINT WindowHeight,
  DescriptorAllocator& RtvAllocator
) {
  DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
  swapChainDescriptor.Width = WindowWidth;
  swapChainDescriptor.Height = WindowHeight;
  swapChainDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDescriptor.SampleDesc.Count = 1;
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescriptor.BufferCount = GraphicsCore::m_frameCount;
  swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  pFactory->CreateSwapChainForHwnd(
    pCmdQueue, hwnd, &swapChainDescriptor, nullptr, nullptr, &swapChain
  );

  pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
  swapChain.As(&m_swapChain);
  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

  for (UINT frame = 0; frame < GraphicsCore::m_frameCount; frame += 1) {
    Microsoft::WRL::ComPtr<ID3D12Resource> pBackBufferResource;
    m_swapChain->GetBuffer(frame, IID_PPV_ARGS(&pBackBufferResource));

    m_rtvAllocations[frame] = RtvAllocator.Allocate();

    m_renderTargetsResources[frame].CreateFromSwapChain(
      pDevice,
      L"BackBuffer_" + std::to_wstring(frame),
      pBackBufferResource.Detach(),
      m_rtvAllocations[frame].CPU      
    );
  }
}

void D3D12Engine::SwapChain::Present() {
  m_swapChain->Present(1, 0);
  
  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12Engine::SwapChain::Resize(
  ID3D12Device* pDevice,
  DescriptorAllocator& RtvAllocator,
  UINT WindowWidth,
  UINT WindowHeight
) {
  for (UINT frame = 0 ; frame < GraphicsCore::m_frameCount; frame += 1) {
    m_renderTargetsResources[frame].Shutdown();

    RtvAllocator.Free(m_rtvAllocations[frame]);
  }
  
  m_swapChain->ResizeBuffers(
    GraphicsCore::m_frameCount,
    WindowWidth, WindowHeight,
    GraphicsCore::BackBufferFormat,
    0
  );

  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

  for (UINT frame = 0; frame < GraphicsCore::m_frameCount; frame += 1) {
    Microsoft::WRL::ComPtr<ID3D12Resource> pBackBufferResource;
    m_swapChain->GetBuffer(frame, IID_PPV_ARGS(&pBackBufferResource));

    m_rtvAllocations[frame] = RtvAllocator.Allocate();

    m_renderTargetsResources[frame].CreateFromSwapChain(
      pDevice,
      L"BackBuffer_" + std::to_wstring(frame),
      pBackBufferResource.Detach(),
      m_rtvAllocations[frame].CPU
    );
  }
}
