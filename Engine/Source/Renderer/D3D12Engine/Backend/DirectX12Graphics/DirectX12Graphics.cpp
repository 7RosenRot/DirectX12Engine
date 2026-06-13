#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <filesystem>
#include <stdexcept>
#include <imgui_impl_dx12.h>

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

  // ↓ Allocating Descriptor Heaps ↓
  m_SrvAllocator = std::make_unique<DescriptorAllocator>(
    m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096
  );
  
  m_RtvAllocator = std::make_unique<DescriptorAllocator>(
    m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256
  );
  
  m_DsvAllocator = std::make_unique<DescriptorAllocator>(
    m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256
  );
  // ↑ Allocating Descriptor Heaps ↑
  
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
  m_SwapChain = std::make_unique<SwapChain>();

  m_SwapChain->Initialize(
    m_device.Get(),
    factory4.Get(),
    m_cmdQueue->GetResource(),
    m_hwnd,
    m_WindowWidth, m_WindowHeight,
    *m_RtvAllocator
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

  if (m_SwapChain == nullptr) {
    return;
  }

  m_WindowWidth = WindowWidth;
  m_WindowHeight = WindowHeight;

  m_cmdQueue->Flush();

  if (m_cmdContext != nullptr) {
    m_cmdContext->Reset();
    m_cmdContext->Close();
  }

  m_SwapChain->Resize(m_device.Get(), *m_RtvAllocator, WindowWidth, WindowHeight);
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  if (m_cmdQueue != nullptr) {
    m_cmdQueue->Flush();
  }
  
  m_DepthBuffer.Shutdown(*m_DsvAllocator);
  m_SceneTexture.Shutdown(*m_RtvAllocator, *m_SrvAllocator);
  
  m_SwapChain.reset();
  m_cmdContext.reset();
  m_cmdQueue.reset();

  m_pipelineState.Shutdown();
  m_rootSignature.Shutdown();

  m_SrvAllocator.reset();
  m_RtvAllocator.reset();
  m_DsvAllocator.reset();

#if !defined(NDEBUG)
  if (m_device != nullptr) {
    Microsoft::WRL::ComPtr<ID3D12DebugDevice> debugDevice;
    if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&debugDevice)))) {
      debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
  }
#endif

  if (m_device != nullptr) {
    m_device.Reset();
  }

  if (factory4 != nullptr) {
    factory4.Reset();
  }
}

void D3D12Engine::DirectX12Graphics::LoadPipeline() {
  UINT DXGIFactoryFlags{0};
#if !defined(NDEBUG)
  Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    debugController->EnableDebugLayer();
    DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
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

#if !defined(NDEBUG)
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
}

void D3D12Engine::DirectX12Graphics::ResizeViewport(
  UINT ViewportWidth,
  UINT ViewportHeight
) {
  if (ViewportWidth == 0 || ViewportHeight == 0) {
    return;
  }
  
  m_cmdQueue->Flush();

  m_ViewportWidth = ViewportWidth;
  m_ViewportHeight = ViewportHeight;

  if (m_DsvAllocator != nullptr) {
    m_DepthBuffer.Shutdown(*m_DsvAllocator);
  }

  if (m_RtvAllocator != nullptr && m_SrvAllocator != nullptr) {
    m_SceneTexture.Shutdown(*m_RtvAllocator, *m_SrvAllocator);
  }

  m_SceneTexture.Initialize(
    m_device.Get(), L"SceneRTT",
    m_ViewportWidth, m_ViewportHeight,
    *m_RtvAllocator,
    *m_SrvAllocator
  );

  m_DepthBuffer.Create(
    m_device.Get(), L"MainDepthBuffer",
    m_ViewportWidth, m_ViewportHeight,
    *m_DsvAllocator
  );
}

void D3D12Engine::DirectX12Graphics::BeginFrame() {
  // ↓ Prepare Pipeline ↓
  m_cmdQueue->Flush();
  m_cmdContext->Reset();
  // ↑ Prepare Pipeline ↑

  // ↓ Set Heaps ↓
  ID3D12DescriptorHeap* heaps[] = { m_SrvAllocator->GetHeap() };
  m_cmdContext->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
  // ↑ Set Heaps ↑
  
  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(m_SceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  // ↓ Clean Up ↓
  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_cmdContext->ClearColor(m_SceneTexture, ClearColor);
  m_cmdContext->ClearDepth(m_DepthBuffer);

  m_cmdContext->SetRenderTargets(m_SceneTexture, m_DepthBuffer);
  // ↑ Clean Up ↑

  // ↓ Set up Viewport & ScissorRect ↓
  D3D12_VIEWPORT viewPort{
    0.0F, 0.0F,
    static_cast<float>(m_ViewportWidth), static_cast<float>(m_ViewportHeight),
    0.0F, 1.0F
  };
  
  D3D12_RECT scissorRect{
    0, 0,
    static_cast<long>(m_ViewportWidth), static_cast<long>(m_ViewportHeight)
  };

  m_cmdContext->SetViewports(1, &viewPort);
  m_cmdContext->SetScissorRects(1, &scissorRect);
  // ↑ Set up Viewport & ScissorRect ↑

  m_cmdContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_cmdContext->SetGraphicsRootSignature(m_rootSignature.Get());
  m_cmdContext->SetPipelineState(m_pipelineState.GetPipelineState());
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
  auto& currentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
  
  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  m_cmdContext->Close();

  // ↓ Draw ↓
  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
  
  m_SwapChain->Present();
  
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
  // ↑ Draw ↑
}

void D3D12Engine::DirectX12Graphics::PrepareUIContext() {
  m_cmdContext->TransitionResource(m_SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  auto& currentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
  m_cmdContext->TransitionResource(currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
  
  m_cmdContext->FlushResourceBarriers();

  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_cmdContext->ClearColor(currentBackBuffer, ClearColor);
  m_cmdContext->SetRenderTargets(currentBackBuffer);
}