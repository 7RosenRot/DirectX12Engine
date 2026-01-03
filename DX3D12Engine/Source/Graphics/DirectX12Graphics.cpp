#include <Include/Graphics/DirectX12Graphics.hpp>
#include <filesystem>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName) :
  InterfaceDirectX12(WindowHeight, WindowWidth, WindowName),
  m_viewPort(0.F, 0.F, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight)),
  m_scissorRect(0, 0, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight))
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  LoadPipeline();
  
  LoadAssets();
}

void D3D12Engine::DirectX12Graphics::OnRender() {
  FillCommandList();

  ID3D12CommandList* ptr_cmdLists[] = {m_cmdList.Get()};
  m_cmdQueue->ExecuteCommandLists(_countof(ptr_cmdLists), ptr_cmdLists);

  m_swapChain->Present(1, 0);

  WaitForPreviousFrame();
}

void D3D12Engine::DirectX12Graphics::OnUpdate() {
  auto angle = GetElapsedSeconds() * 2.F;
  float startPosition = 0.25F * m_Coefficient;
  
  m_Vertex triangleVertices[] = {
    { {                                0.F,        startPosition, 0.5F - startPosition * sinf(angle) }, { 1.F, 0.F, 0.F, 1.F } },
    { {        startPosition * cosf(angle), (-1) * startPosition, 0.5F - startPosition * sinf(angle) }, { 0.F, 1.F, 0.F, 1.F } },
    { { (-1) * startPosition * cosf(angle), (-1) * startPosition, 0.5F - startPosition * sinf(angle) }, { 0.F, 0.F, 1.F, 1.F } }
  };

  UINT8* ptr_vertexDataBegin{nullptr};
  m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&ptr_vertexDataBegin));
  memcpy(ptr_vertexDataBegin, triangleVertices, sizeof(triangleVertices));
  m_vertexBuffer->Unmap(0, nullptr);
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  WaitForPreviousFrame();
  CloseHandle(m_fenceEvent);
}

void D3D12Engine::DirectX12Graphics::LoadPipeline() {
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
    
    GetHardwareAdapter(factory4.Get(), &hardwareAdapter, true);
    D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
  }

  D3D12_COMMAND_QUEUE_DESC cmdQueueDescriptor{};
  cmdQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cmdQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  m_device->CreateCommandQueue(&cmdQueueDescriptor, IID_PPV_ARGS(&m_cmdQueue));

  DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
  swapChainDescriptor.BufferCount = m_frameCount;
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDescriptor.SampleDesc.Count = 1;
  swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDescriptor.Height = m_WindowHeight;
  swapChainDescriptor.Width = m_WindowWidth;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  factory4->CreateSwapChainForHwnd(
    m_cmdQueue.Get(),
    D3D12Engine::Window::getHwnd(),
    &swapChainDescriptor,
    nullptr, nullptr,
    &swapChain
  );

  factory4->MakeWindowAssociation(
    D3D12Engine::Window::getHwnd(),
    DXGI_MWA_NO_ALT_ENTER
  );
  
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
}

void D3D12Engine::DirectX12Graphics::LoadAssets() {
  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescriptor{};
  rootSignatureDescriptor.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  Microsoft::WRL::ComPtr<ID3DBlob> signature;
  Microsoft::WRL::ComPtr<ID3DBlob> error;
  
  D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
  m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

#if defined(_DEBUG)
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags{0};
#endif

  Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
  Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

  std::wstring shadersPath = D3D12Engine::InterfaceDirectX12::GetAssetPath(L"Shaders.hlsl");
  if (!std::filesystem::exists(shadersPath)) OutputDebugStringW((L"ERROR: File not found - " + shadersPath + L'\n').c_str());

  D3DCompileFromFile(
    shadersPath.c_str(), 
    nullptr, nullptr,
    "VSMain", "vs_5_0",
    compileFlags, 0,
    &vertexShader, nullptr
  );

  D3DCompileFromFile(
    shadersPath.c_str(), 
    nullptr, nullptr,
    "PSMain", "ps_5_0",
    compileFlags, 0,
    &pixelShader, nullptr
  );

  D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor{};
  psoDescriptor.InputLayout = {inputElementDescriptor, _countof(inputElementDescriptor)};
  psoDescriptor.pRootSignature = m_rootSignature.Get();
  psoDescriptor.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
  psoDescriptor.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
  psoDescriptor.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDescriptor.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  psoDescriptor.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDescriptor.DepthStencilState.DepthEnable = false;
  psoDescriptor.DepthStencilState.StencilEnable = false;
  psoDescriptor.SampleMask = UINT_MAX;
  psoDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDescriptor.NumRenderTargets = 1;
  psoDescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDescriptor.SampleDesc.Count = 1;

  m_device->CreateGraphicsPipelineState(&psoDescriptor, IID_PPV_ARGS(&m_pipelineState));

  m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_cmdList));

  m_cmdList->Close();

  m_Vertex triangleVertices[] = {
    { {  0.F,    0.25F * m_Coefficient, 0.F }, { 1.F, 0.F, 0.F, 1.F } },
    { {  0.25F, -0.25F * m_Coefficient, 0.F }, { 0.F, 1.F, 0.F, 1.F } },
    { { -0.25F, -0.25F * m_Coefficient, 0.F }, { 0.F, 0.F, 1.F, 1.F } }
  };

  const size_t vertexBufferSize = sizeof(triangleVertices);

  CD3DX12_HEAP_PROPERTIES heapProperties{};
  heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

  CD3DX12_RESOURCE_DESC resourceDesriptor = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

  m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesriptor, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer));

  UINT8* ptr_vertexDataBegin{};
  CD3DX12_RANGE readRange{0, 0};
  m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&ptr_vertexDataBegin));
  memcpy(ptr_vertexDataBegin, triangleVertices, vertexBufferSize);
  m_vertexBuffer->Unmap(0, nullptr);

  m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
  m_vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);
  m_vertexBufferView.StrideInBytes = sizeof(m_Vertex);

  m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
  m_fenceValue = 1;

  m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
  if (m_fenceEvent == nullptr) {
    std::throw_with_nested(HRESULT_FROM_WIN32(GetLastError()));
  }

  WaitForPreviousFrame();
}

void D3D12Engine::DirectX12Graphics::FillCommandList() {
  m_cmdAllocator->Reset();
  m_cmdList->Reset(m_cmdAllocator.Get(), m_pipelineState.Get());

  m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
  m_cmdList->RSSetViewports(1, &m_viewPort);
  m_cmdList->RSSetScissorRects(1, &m_scissorRect);

  m_transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_renderTargets[m_frameIndex].Get(), 
    D3D12_RESOURCE_STATE_PRESENT, 
    D3D12_RESOURCE_STATE_RENDER_TARGET
  );

  m_cmdList->ResourceBarrier(1, &m_transitionBarier);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

  m_cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

  const float bgColor[] = {0.1F, 0.1F, 0.1F, 1.F};

  m_cmdList->ClearRenderTargetView(rtvHandle, bgColor, 0, nullptr);
  m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
  m_cmdList->DrawInstanced(3, 1, 0, 0);

  m_transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_renderTargets[m_frameIndex].Get(), 
    D3D12_RESOURCE_STATE_RENDER_TARGET, 
    D3D12_RESOURCE_STATE_PRESENT
  );

  m_cmdList->ResourceBarrier(1, &m_transitionBarier);
  
  m_cmdList->Close();
}

void D3D12Engine::DirectX12Graphics::WaitForPreviousFrame() {
  const UINT64 fence = m_fenceValue;
  m_cmdQueue->Signal(m_fence.Get(), fence);
  m_fenceValue += 1;

  if (m_fence->GetCompletedValue() < fence) {
    m_fence->SetEventOnCompletion(fence, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }
  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}