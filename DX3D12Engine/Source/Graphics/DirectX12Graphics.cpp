#include <Include/Graphics/DirectX12Graphics.hpp>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT m_WindowHeight, UINT m_WindowWidth, std::wstring m_WindowName) :
  InterfaceDirectX12(m_WindowHeight, m_WindowWidth, m_WindowName)
  // Добавить соответсвующие поля, которые будут проинициализированы конструктором класса по умолчанию
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  LoadPipeline(); // Подготовливаем среду для работы
  LoadAssets();   // Устанавливаем контракт между нашим кодом и шейдерами
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

void D3D12Engine::DirectX12Graphics::LoadPipeline() {
  UINT DXGIFactoryFlags{0};
#if defined(_DEBUD)
  {
    Microsoft::WRL::ComPtr<ID3DDebug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_AGS(&debugController)))) {
      debugController->EnablrDebugLayer();
      DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif
  Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory4;
  CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&m_Factory4));

  if (useWarpAdapter) {
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_WarpAdapter;
    m_Factory4->EnumWarpAdapter(IID_PPV_ARGS(&m_WarpAdapter));
    D3D12CreateDevice(m_WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));
  }
  else {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_HardwareAdapter;
    getHardwareAdapter(m_Factory4.Get(), &m_HardwareAdapter, true);
    D3D12CreateDevice(m_HardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_HardwareAdapter));
  }

  D3D12_COMMAND_QUEUE_DESC m_CommandQueueDescriptor{};
  m_CommandQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  m_CommandQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  Device->CreateCommandQueue(&m_CommandQueueDescriptor, IID_PPV_ARGS(&Device));

  DXGI_SWAP_CHAIN_DESC1 m_SwapChainDescriptor{};
  m_SwapChainDescriptor.BufferCount = FrameCount;
  m_SwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  m_SwapChainDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  m_SwapChainDescriptor.SampleDesc.Count = 1;
  m_SwapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  m_SwapChainDescriptor.Height = WindowHeight;
  m_SwapChainDescriptor.Width = WindowWidth;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
  m_Factory4->CreateSwapChainForHwnd(CommandQueue.Get(), D3D12Engine::Window::getHwnd(), &m_SwapChainDescriptor, nullptr, nullptr, &m_SwapChain);

  m_Factory4->MakeWindowAssociation(D3D12Engine::Window::getHwnd(), DXGI_MWA_NO_ALT_ENTER);

  m_SwapChain.As(&SwapChain);
  FrameIndex = SwapChain->GetCurrentBackBufferIndex();

  D3D12_DESCRIPTOR_HEAP_DESC m_RenderTargetViewHeapDescriptor{};
  m_RenderTargetViewHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  m_RenderTargetViewHeapDescriptor.NodeMask = 0;
  m_RenderTargetViewHeapDescriptor.NumDescriptors = FrameCount;
  m_RenderTargetViewHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

  Device->CreateDescriptorHeap(&m_RenderTargetViewHeapDescriptor, IID_PPV_ARGS(&RenderTargetViewHeapDescriptor));
  RenderTargetViewDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE m_RenderTargetViewHandle(RenderTargetViewHeapDescriptor->GetCPUDescriptorHandleForHeapStart());
  for (UINT Frame = 0; Frame < FrameCount; Frame += 1) {
    SwapChain->GetBuffer(Frame, IID_PPV_ARGS(&RenderTargets[Frame]));
    Device->CreateRenderTargetView(RenderTargets[Frame].Get(), nullptr, m_RenderTargetViewHandle);
    m_RenderTargetViewHandle.Offset(1, RenderTargetViewDescriptorSize);
  }

  Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
}

void D3D12Engine::DirectX12Graphics::LoadAssets() {}

void D3D12Engine::DirectX12Graphics::FillCommandList() {}

void D3D12Engine::DirectX12Graphics::WaitForPreviousFrame() {}