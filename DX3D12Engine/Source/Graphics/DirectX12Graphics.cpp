#include <Include/Graphics/DirectX12Graphics.hpp>
#include <Include/Graphics/GPUResource.hpp>
#include <Include/Graphics/GraphicsContext.hpp>

#include <filesystem>
#include <exception>
#include <stdexcept>

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

// DirectX12Graphics.cpp

void D3D12Engine::DirectX12Graphics::OnRender() {
  GraphicsContext context(m_cmdList.Get());
  
  m_cmdAllocator->Reset();
  m_cmdList->Reset(m_cmdAllocator.Get(), m_pipelineState.Get());
  
  context.SetGraphicsRootSignature(m_rootSignature.Get());
  
  context.SetViewports(1, &m_viewPort);
  context.SetScissorRects(1, &m_scissorRect);
  context.TransitionResource(m_renderTargetsResources[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);
  
  context.FlushResourceBarriers(); 
  
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
    m_frameIndex,
    m_rtvDescriptorSize
  );
  context.GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
  
  const float bgColor[] = {0.1F, 0.1F, 0.1F, 1.F};
  context.GetCommandList()->ClearRenderTargetView(rtvHandle, bgColor, 0, nullptr);
  
  context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context.SetVertexBuffer(0, m_vertexBufferView);
  
  context.SetIndexBuffer(m_indexBufferView);
  context.DrawIndexedInstanced(12, 1, 0, 0, 0);
  context.TransitionResource(m_renderTargetsResources[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT);
  
  context.FlushResourceBarriers(); 
  context.Close();
  
  ID3D12CommandList* ptr_cmdLists[] = { m_cmdList.Get() };
  
  m_cmdQueue->ExecuteCommandLists(_countof(ptr_cmdLists), ptr_cmdLists);
  m_swapChain->Present(1, 0);
  
  WaitForPreviousFrame();
}

void D3D12Engine::DirectX12Graphics::OnUpdate() {}

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
  swapChainDescriptor.Width = m_WindowWidth;
  swapChainDescriptor.Height = m_WindowHeight;
  swapChainDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDescriptor.SampleDesc.Count = 1;
  swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescriptor.BufferCount = m_frameCount;
  swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  factory4->CreateSwapChainForHwnd(
    m_cmdQueue.Get(),
    D3D12Engine::Window::GetHwnd(),
    &swapChainDescriptor,
    nullptr, nullptr,
    &swapChain
  );

  factory4->MakeWindowAssociation(
    D3D12Engine::Window::GetHwnd(),
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
    Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
    m_swapChain->GetBuffer(frame, IID_PPV_ARGS(&backBuffer));
    m_renderTargetsResources[frame].CreateFromSwapChain(backBuffer);

    m_device->CreateRenderTargetView(m_renderTargetsResources[frame].GetResource(), nullptr, rtvHandle);
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

  HRESULT hResult{0};

  std::wstring vtxShaderPath = D3D12Engine::InterfaceDirectX12::GetAssetPath(L"VertexShader.hlsl");
  if (!std::filesystem::exists(vtxShaderPath)) { throw std::runtime_error("Shaders files do not exist!"); }
  
  Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
  Microsoft::WRL::ComPtr<ID3DBlob> vtxErrorBuffer;
  
  hResult = D3DCompileFromFile(
    vtxShaderPath.c_str(),
    nullptr, nullptr,
    "VSMain", "vs_5_0",
    compileFlags, 0,
    &vertexShader, &vtxErrorBuffer
  );

  if (FAILED(hResult)) {
    if (vtxErrorBuffer) { OutputDebugStringA((char*)vtxErrorBuffer->GetBufferPointer()); }

    throw std::runtime_error("Vertex Shader compilation has failed!");
  }

  std::wstring pxlShaderPath = D3D12Engine::InterfaceDirectX12::GetAssetPath(L"PixelShader.hlsl");
  if (!std::filesystem::exists(pxlShaderPath)) { throw std::runtime_error("Shaders files do not exist!"); }

  Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
  Microsoft::WRL::ComPtr<ID3DBlob> pxlErrorBuffer;
  
  hResult = D3DCompileFromFile(
    pxlShaderPath.c_str(),
    nullptr, nullptr,
    "PSMain", "ps_5_0",
    compileFlags, 0,
    &pixelShader, &pxlErrorBuffer
  );

  if (FAILED(hResult)) {
    if (pxlErrorBuffer) { OutputDebugStringA((char*)pxlErrorBuffer->GetBufferPointer()); }
    
    throw std::runtime_error("Pixel Shader compilation has failed!");
  }

  D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor{};
  psoDescriptor.InputLayout = { inputElementDescriptor, _countof(inputElementDescriptor) };
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

  m_Vertex rectangleVertices[] = {
    { { -0.5F,  0.5F, 0.F }, { 1.F, 0.F, 0.F, 1.F } },
    { {  0.5F, -0.5F, 0.F }, { 0.F, 1.F, 0.F, 1.F } },
    { { -0.5F, -0.5F, 0.F }, { 0.F, 0.F, 1.F, 1.F } },
    { {  0.5F,  0.5F, 0.F }, { 0.F, 0.F, 1.F, 1.F } },
    { {  0.F,   1.F,  0.F }, { 1.F, 0.F, 1.F, 1.F } },
    { {  0.F,  -1.F,  0.F }, { 1.F, 0.F, 1.F, 1.F } }
  };

  const size_t vertexBufferSize = sizeof(rectangleVertices);

  m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_vertexBuffer)
  );
  
  UINT8* ptr_vertexDataBegin{nullptr};
  CD3DX12_RANGE readRange{0, 0};
  m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&ptr_vertexDataBegin));
  memcpy(ptr_vertexDataBegin, rectangleVertices, vertexBufferSize);
  m_vertexBuffer->Unmap(0, nullptr);

  /* Extending to index buffer */

  DWORD indexRectangle[] = {
    0, 3, 1,
    0, 1, 2,
    0, 3, 4,
    1, 2, 5
  };

  const size_t indexBufferSize = sizeof(indexRectangle);

  m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&m_indexBuffer)
  );

  m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_indexBufferUploadHeap)
  );

  m_cmdList->Reset(m_cmdAllocator.Get(), m_pipelineState.Get());

  D3D12_SUBRESOURCE_DATA indexData = {};
  indexData.pData = reinterpret_cast<const void*>(indexRectangle);
  indexData.RowPitch = indexBufferSize;
  indexData.SlicePitch = indexBufferSize;
  UpdateSubresources(m_cmdList.Get(), m_indexBuffer.Get(), m_indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);

  CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_indexBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  );

  m_cmdList->ResourceBarrier(1, &transitionBarrier);

  m_cmdList->Close();

  ID3D12CommandList* ptr_cmdLists[] = { m_cmdList.Get() };
  m_cmdQueue->ExecuteCommandLists(_countof(ptr_cmdLists), ptr_cmdLists);

  m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
  m_vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);
  m_vertexBufferView.StrideInBytes = sizeof(m_Vertex);

  m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
  m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
  m_indexBufferView.SizeInBytes = static_cast<UINT>(indexBufferSize);

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

  CD3DX12_RESOURCE_BARRIER transitionBarier{};

  transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_renderTargetsResources[m_frameIndex].GetResource(), 
    D3D12_RESOURCE_STATE_PRESENT, 
    D3D12_RESOURCE_STATE_RENDER_TARGET
  );

  m_cmdList->ResourceBarrier(1, &transitionBarier);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
    m_frameIndex,
    m_rtvDescriptorSize
  );

  m_cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

  const float bgColor[] = {0.1F, 0.1F, 0.1F, 1.F};

  m_cmdList->ClearRenderTargetView(rtvHandle, bgColor, 0, nullptr);
  m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
  m_cmdList->IASetIndexBuffer(&m_indexBufferView);
  
  m_cmdList->DrawIndexedInstanced(12, 1, 0, 0, 0);

  transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(
    m_renderTargetsResources[m_frameIndex].GetResource(), 
    D3D12_RESOURCE_STATE_RENDER_TARGET, 
    D3D12_RESOURCE_STATE_PRESENT
  );

  m_cmdList->ResourceBarrier(1, &transitionBarier);
  
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