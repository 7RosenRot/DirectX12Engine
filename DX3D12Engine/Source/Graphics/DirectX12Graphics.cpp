#include <Include/Graphics/DirectX12Graphics.hpp>
#include <stdexcept>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName) :
  InterfaceDirectX12(WindowHeight, WindowWidth, WindowName)
  // Добавить соответсвующие поля, которые будут проинициализированы конструктором класса по умолчанию
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  // Подготовливаем среду для работы
  if (FAILED(LoadPipeline())) throw std::runtime_error("Pipeline load failed");
  
  // Устанавливаем контракт между нашим кодом и шейдерами
  if (FAILED(LoadAssets())) throw std::runtime_error("Assets load failed");
}

void D3D12Engine::DirectX12Graphics::OnRender() {
  // Выполняем отрисовку
}

void D3D12Engine::DirectX12Graphics::OnUpdate() {
  // Обновляем кадр, сели встречено какое-то изменение
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  // Дожидаемся отрисовки последнего кадра и завершаем конвейер
}

bool D3D12Engine::DirectX12Graphics::LoadPipeline() {
  UINT DXGIFactoryFlags{0};
#if defined(_DEBUG)
  {
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
      DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif
  Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
  CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&factory4));

  if (m_useWarpAdapter) {
    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    
    factory4->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
  }
  else {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    
    getHardwareAdapter(factory4.Get(), &hardwareAdapter, true);
    D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
  }

  D3D12_COMMAND_QUEUE_DESC cmdQueueDescriptor{};
  cmdQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cmdQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  m_device->CreateCommandQueue(&cmdQueueDescriptor, IID_PPV_ARGS(&m_cmdQueue));

  DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
  swapChainDescriptor.BufferCount = m_frameCount;
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDescriptor.SampleDesc.Count = 1;
  swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDescriptor.Height = m_WindowHeight;
  swapChainDescriptor.Width = m_WindowWidth;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  factory4->CreateSwapChainForHwnd(m_cmdQueue.Get(), D3D12Engine::Window::getHwnd(), &swapChainDescriptor, nullptr, nullptr, &swapChain);

  factory4->MakeWindowAssociation(D3D12Engine::Window::getHwnd(), DXGI_MWA_NO_ALT_ENTER);
  
  swapChain.As(&m_swapChain);
  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

  D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeap{};
  rtvDescriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvDescriptorHeap.NodeMask = 0;
  rtvDescriptorHeap.NumDescriptors = m_frameCount;
  rtvDescriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

  m_device->CreateDescriptorHeap(&rtvDescriptorHeap, IID_PPV_ARGS(&m_rtvDescriptorHeap));
  m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT frame = 0; frame < m_frameCount; frame += 1) {
    m_swapChain->GetBuffer(frame, IID_PPV_ARGS(&m_renderTargets[frame]));
    m_device->CreateRenderTargetView(m_renderTargets[frame].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, m_rtvDescriptorSize);
  }

  m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator));

  return true;
}

bool D3D12Engine::DirectX12Graphics::LoadAssets() {
  return true;
}

void D3D12Engine::DirectX12Graphics::FillCommandList() {}

void D3D12Engine::DirectX12Graphics::WaitForPreviousFrame() {}