#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <filesystem>
#include <stdexcept>

#include <Renderer/D3D12Engine/Backend/DirectX12Graphics/DirectX12Graphics.hpp>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(HWND hwnd, UINT WindowWidth, UINT WindowHeight) :
  m_WindowWidth(WindowWidth), m_WindowHeight(WindowHeight), m_hwnd(hwnd)
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {
  OnDestroy();
}

_Use_decl_annotations_
void D3D12Engine::DirectX12Graphics::GetHardwareAdapter(
  _In_ IDXGIFactory1* pFactory1,
  _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1,
  bool requestHighPerformanceAdapter
) {
  *ppAdapter1 = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter1;
  Microsoft::WRL::ComPtr<IDXGIFactory6> Factory6;

  if (SUCCEEDED(pFactory1->QueryInterface(IID_PPV_ARGS(&Factory6)))) {
    const auto GpuPreference = requestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
    
    for (UINT AdapterIndex = 0;
      SUCCEEDED(Factory6->EnumAdapterByGpuPreference(AdapterIndex, GpuPreference, IID_PPV_ARGS(&Adapter1)));
      AdapterIndex += 1)
    {
      DXGI_ADAPTER_DESC1 AdapterDescriptor;
      Adapter1->GetDesc1(&AdapterDescriptor);

      if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }
      if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
        *ppAdapter1 = Adapter1.Detach();
        return;
      }
    }
  }

  for (
    UINT AdapterIndex = 0;
    SUCCEEDED(pFactory1->EnumAdapters1(AdapterIndex, &Adapter1));
    AdapterIndex += 1
  ) {
    DXGI_ADAPTER_DESC1 AdapterDescriptor;
    Adapter1->GetDesc1(&AdapterDescriptor);

    if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }

    if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
      *ppAdapter1 = Adapter1.Detach();
      return;
    }
  }
}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  // ↓ Loading Pipeline ↓
  LoadPipeline();
  // ↑ Loading Pipeline ↑
  
  // ↓ Initializing CommandQueue ↓
  m_cmdQueue = std::make_unique<CommandQueue>(
    m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT
  );
  // ↑ Initializing CommandQueue ↑

  // ↓ Initializing CommandContext ↓
  m_cmdContext = std::make_unique<CommandContext>(
    m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT
  );
  // ↑ Initializing CommandContext ↑

  // ↓ Initializing SwapCahin ↓
  m_display = std::make_unique<SwapChain>();

  m_display->Initialize(
    m_device.Get(),
    factory4.Get(),
    m_cmdQueue->GetResource(),
    m_hwnd,
    m_WindowWidth, m_WindowHeight
  );
  // ↑ Initializing SwapCahin ↑

  // ↓ Loading Assets ↓
  LoadAssets();
  // ↑ Loading Assets ↑
}

void D3D12Engine::DirectX12Graphics::OnResize(UINT WindowWidth, UINT WindowHeight) {
  if (WindowWidth == 0 || WindowHeight == 0) {
    return;
  }
  
  m_WindowWidth = WindowWidth;
  m_WindowHeight = WindowHeight;

  if (m_display == nullptr) {
    return;
  }
  
  m_cmdQueue->Flush();

  m_display->Resize(m_device.Get(), m_depthBuffer, WindowWidth, WindowHeight);
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  if (m_cmdQueue != nullptr) {
    m_cmdQueue->Flush();
  }
  
  m_display.reset();
  m_cmdContext.reset();
  
  m_cmdQueue.reset();

  if (m_device != nullptr) {
    m_device.Reset();
  }
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

  m_depthBuffer.Create(
    m_device.Get(),
    L"MainDepthBuffer",
    m_WindowWidth, m_WindowHeight,
    DXGI_FORMAT_D32_FLOAT
  );
}

void D3D12Engine::DirectX12Graphics::LoadAssets() {
  m_rootSignature.Reset(2, 1);
  m_rootSignature[0].InitAsConstants(0, 16, D3D12_SHADER_VISIBILITY_ALL);
  
  CD3DX12_DESCRIPTOR_RANGE srvRange;
  srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
  m_rootSignature[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

  CD3DX12_STATIC_SAMPLER_DESC sampler(
    0,
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP
  );
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  m_rootSignature.InitStaticSampler(0, sampler);
  
  m_rootSignature.Finalize(m_device.Get(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

#if defined(_DEBUG)
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags{0};
#endif

  // ↓ Shader Compilation ↓
  HRESULT hResult{0};

  std::wstring vtxShaderPath = L"Engine/Assets/Shaders/D3D12/VertexShader.hlsl";
  if (!std::filesystem::exists(vtxShaderPath)) { OutputDebugStringW((L"ERROR: File not found - " + vtxShaderPath + L'\n').c_str()); }
  
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
    if (vtxErrorBuffer) {
      OutputDebugStringA(reinterpret_cast<const char*>(vtxErrorBuffer->GetBufferPointer()));
    }

    throw std::runtime_error("Vertex Shader compilation has failed!");
  }

  std::wstring pxlShaderPath = L"Engine/Assets/Shaders/D3D12/PixelShader.hlsl";
  if (!std::filesystem::exists(pxlShaderPath)) { OutputDebugStringW((L"ERROR: File not found - " + pxlShaderPath + L'\n').c_str()); }

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
    if (pxlErrorBuffer) {
      OutputDebugStringA(reinterpret_cast<const char*>(pxlErrorBuffer->GetBufferPointer()));
    }
    
    throw std::runtime_error("Pixel Shader compilation has failed!");
  }
  // ↑ Shader Compilation ↑

  D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  m_pipelineState.SetRootSignature(m_rootSignature);
  m_pipelineState.SetVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize());
  m_pipelineState.SetPixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize());
  m_pipelineState.SetInputLayout(_countof(inputElementDescriptor), inputElementDescriptor);
  m_pipelineState.SetCullMode(D3D12_CULL_MODE_BACK);
  m_pipelineState.SetFrontCounterClockwise(true);
  m_pipelineState.SetDepthTest(true);
  m_pipelineState.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
  m_pipelineState.Finalize(m_device.Get());

  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
 
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
}

void D3D12Engine::DirectX12Graphics::InitUI() {
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  desc.NumDescriptors = 2;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_UiSrvHeap));

  ImGui_ImplDX12_InitInfo initInfo = {};
  initInfo.Device = m_device.Get();
  initInfo.CommandQueue = m_cmdQueue->GetResource();
  initInfo.NumFramesInFlight = GraphicsCore::m_frameCount;
  initInfo.RTVFormat = GraphicsCore::BackBufferFormat;
  initInfo.SrvDescriptorHeap = m_UiSrvHeap.Get();
  initInfo.LegacySingleSrvCpuDescriptor = m_UiSrvHeap->GetCPUDescriptorHandleForHeapStart();
  initInfo.LegacySingleSrvGpuDescriptor = m_UiSrvHeap->GetGPUDescriptorHandleForHeapStart();
  ImGui_ImplDX12_Init(&initInfo);
}

void D3D12Engine::DirectX12Graphics::BeginUI() {
  m_cmdContext->TransitionResource(m_SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  auto& currentBackBuffer = m_display->GetCurrentBackBufferIndex();
  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑
  
  const float ClearColor[] = {0.1F, 0.1F, 0.1F, 1.0F};
  m_cmdContext->ClearColor(currentBackBuffer, ClearColor);
  m_cmdContext->SetRenderTargets(currentBackBuffer);
  
  UINT handleIncrement = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  D3D12_CPU_DESCRIPTOR_HANDLE destCpuHandle = m_UiSrvHeap->GetCPUDescriptorHandleForHeapStart();
  destCpuHandle.ptr += handleIncrement;
  m_device->CopyDescriptorsSimple(1, destCpuHandle, m_SceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  ID3D12DescriptorHeap* heaps[] = { m_UiSrvHeap.Get() };
  m_cmdContext->GetCommandList()->SetDescriptorHeaps(1, heaps);

  ImGui_ImplDX12_NewFrame();
}

void D3D12Engine::DirectX12Graphics::RenderUI() {
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_cmdContext->GetCommandList());
}

void D3D12Engine::DirectX12Graphics::DestroyUI() {
  ImGui_ImplDX12_Shutdown();
}

void D3D12Engine::DirectX12Graphics::ResizeViewport(UINT ViewportWidth, UINT ViewportHeight) {
  if (
    ViewportWidth == 0 ||
    ViewportHeight == 0
  ) { return; }

  m_ViewportWidth = ViewportWidth;
  m_ViewportHeight = ViewportHeight;

  m_cmdQueue->Flush();
  
  m_SceneTexture.CreateScene(
    m_device.Get(),
    L"SceneColorBuffer",
    ViewportWidth,
    ViewportHeight,
    DXGI_FORMAT_R8G8B8A8_UNORM
  );
  
  m_viewPort = CD3DX12_VIEWPORT(
    0.0F, 0.0F,
    static_cast<float>(ViewportWidth), static_cast<float>(ViewportHeight),
    0.0F, 1.0F
  );
  m_scissorRect = CD3DX12_RECT(
    0, 0,
    static_cast<long>(ViewportWidth), static_cast<long>(ViewportHeight)
  );
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12Engine::DirectX12Graphics::GetSceneTextureSRV() {
  D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_UiSrvHeap->GetGPUDescriptorHandleForHeapStart();
  gpuHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  return gpuHandle;
}

void D3D12Engine::DirectX12Graphics::BeginFrame() {
  // ↓ Prepare Pipeline ↓
  m_cmdQueue->Flush();
  m_cmdContext->Reset();
  // ↑ Prepare Pipeline ↑
  
  // ↓ Set up PipelineState & RootSignature ↓
  m_cmdContext->SetPipelineState(m_pipelineState.GetPipelineState());
  m_cmdContext->SetGraphicsRootSignature(m_rootSignature.Get());
  // ↑ Set up PipelineState & RootSignature ↑

  // ↓ Set up Viewport & ScissorRect ↓
  m_cmdContext->SetViewports(1, &m_viewPort);
  m_cmdContext->SetScissorRects(1, &m_scissorRect);
  // ↑ Set up Viewport & ScissorRect ↑

  // ↓ Clean Up ↓
  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_cmdContext->ClearColor(m_SceneTexture, ClearColor);
  m_cmdContext->ClearDepth(m_depthBuffer);

  m_cmdContext->SetRenderTargets(m_SceneTexture, m_depthBuffer);
  // ↑ Clean Up ↑

  m_cmdContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D12Engine::DirectX12Graphics::DrawFrame(
  D3D12Engine::Model& rModel,
  D3D12Engine::Texture& rTexture,
  const DirectX::XMMATRIX& rViewProjectionMatrix
) {
  rTexture.Bind(*m_cmdContext, 1);

  m_cmdContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &rViewProjectionMatrix, 0);

  rModel.DrawModel(*m_cmdContext);
}

void D3D12Engine::DirectX12Graphics::EndFrame() {
  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(m_display->GetCurrentBackBufferIndex(), D3D12_RESOURCE_STATE_PRESENT);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  m_cmdContext->Close();

  // ↓ Draw ↓
  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
  
  m_display->Present();
  
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
  // ↑ Draw ↑
}

std::shared_ptr<D3D12Engine::Model> 
  D3D12Engine::DirectX12Graphics::LoadModel(const std::string FilePath) {
    auto Model = std::make_shared<D3D12Engine::Model>();

    // ↓ TODO: Name stage ↓
    m_cmdContext->Reset();
    
    Model->LoadModel(FilePath, m_device.Get(), *m_cmdContext);
    
    m_cmdContext->Close();
    // ↑ TODO: Name stage ↑

    // ↓ TODO: Name stage ↓
    UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
    m_cmdQueue->WaitForPreviousFrame(fenceValue);
    // ↑ TODO: Name stage ↑

    return Model;
  }

std::shared_ptr<D3D12Engine::Texture>
  D3D12Engine::DirectX12Graphics::LoadTexture(const std::string FilePath) {
    auto Texture = std::make_shared<D3D12Engine::Texture>();

    // ↓ TODO: Name stage ↓
    m_cmdContext->Reset();
    
    Texture->LoadTexture(FilePath, m_device.Get(), *m_cmdContext);
    
    m_cmdContext->Close();
    // ↑ TODO: Name stage ↑

    // ↓ TODO: Name stage ↓
    UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
    m_cmdQueue->WaitForPreviousFrame(fenceValue);
    // ↑ TODO: Name stage ↑

    return Texture;
  }