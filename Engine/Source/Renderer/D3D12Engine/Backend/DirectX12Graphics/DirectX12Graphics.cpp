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

  // ↓ Initializing CommandContext ↓
  m_cmdContextPool = std::make_unique<CommandContextPool>(
    m_device.Get(), m_SwapChain.get(), D3D12_COMMAND_LIST_TYPE_DIRECT
  );
  // ↑ Initializing CommandContext ↑

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

  if (m_cmdQueue != nullptr) {
    m_cmdQueue->Flush();
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
  m_pFrameContext = nullptr;
  m_cmdContextPool.reset();
  m_cmdQueue.reset();

  m_pipelineState.Shutdown();
  m_outlinePipelineState.Shutdown();
  m_stencilWritePipelineState.Shutdown();
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
  
#pragma region Set Up Shaders
  HRESULT hResult = 0;

  D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };
  
#pragma region Vertex & Pixel Shader Compilation
  std::wstring vtxShaderPath = L"Engine/Assets/Shaders/D3D12/VertexShader.hlsl";
  if (!std::filesystem::exists(vtxShaderPath)) {
    OutputDebugStringW((L"ERROR: File not found - " + vtxShaderPath + L'\n').c_str());
  }

  Microsoft::WRL::ComPtr<ID3DBlob> vertexShader, vtxErrorBuffer;

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
  if (!std::filesystem::exists(pxlShaderPath)) {
    OutputDebugStringW((L"ERROR: File not found - " + pxlShaderPath + L'\n').c_str());
  }

  Microsoft::WRL::ComPtr<ID3DBlob> pixelShader, pxlErrorBuffer;

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
#pragma endregion

#pragma region Vertex & Pixel Shader Set Up PSO
  m_pipelineState.SetRootSignature(m_rootSignature);
  
  m_pipelineState.SetVertexShader(
    vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()
  );
  
  m_pipelineState.SetPixelShader(
    pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()
  );
  
  m_pipelineState.SetInputLayout(
    _countof(inputElementDescriptor), inputElementDescriptor
  );
  
  m_pipelineState.SetCullMode(D3D12_CULL_MODE_BACK);
  
  m_pipelineState.SetFrontCounterClockwise(true);
  
  m_pipelineState.SetDepthTest(true);
  
  m_pipelineState.SetStencilTest(
    true,
    D3D12_COMPARISON_FUNC_ALWAYS,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_REPLACE,
    0xFF,
    0xFF
  );
  
  m_pipelineState.SetRenderTargetFormat(
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_D24_UNORM_S8_UINT
  );
  
  m_pipelineState.Finalize(m_device.Get());
#pragma endregion

#pragma region Outline Vertex & Pixel Shader Compilation
  std::wstring outVtxShaderPath = L"Engine/Assets/Shaders/D3D12/OutlineVertexShader.hlsl";
  if (!std::filesystem::exists(outVtxShaderPath)) {
    OutputDebugStringW((L"ERROR: File not found - " + outVtxShaderPath + L'\n').c_str());
  }
  
  Microsoft::WRL::ComPtr<ID3DBlob> outVtxShader, outVtxErrorBuffer;
  
  hResult = D3DCompileFromFile(
    outVtxShaderPath.c_str(),
    nullptr, nullptr,
    "VSMain", "vs_5_0",
    compileFlags, 0,
    &outVtxShader, &outVtxErrorBuffer
  );

  if (FAILED(hResult)) {
    if (outVtxErrorBuffer) {
      OutputDebugStringA(reinterpret_cast<const char*>(outVtxErrorBuffer->GetBufferPointer()));
    }
    
    throw std::runtime_error("Outline Vertex Shader compilation has failed!");
  }

  std::wstring outPxlShaderPath = L"Engine/Assets/Shaders/D3D12/OutlinePixelShader.hlsl";
  if (!std::filesystem::exists(outPxlShaderPath)) {
    OutputDebugStringW((L"ERROR: File not found - " + outPxlShaderPath + L'\n').c_str());
  }

  Microsoft::WRL::ComPtr<ID3DBlob> outPxlShader, outPxlErrorBuffer;
  
  hResult = D3DCompileFromFile(
    outPxlShaderPath.c_str(),
    nullptr, nullptr,
    "PSMain", "ps_5_0",
    compileFlags, 0,
    &outPxlShader, &outPxlErrorBuffer
  );
  
  if (FAILED(hResult)) {
    if (outPxlErrorBuffer) {
      OutputDebugStringA(reinterpret_cast<const char*>(outPxlErrorBuffer->GetBufferPointer()));
    }
    
    throw std::runtime_error("Outline Pixel Shader compilation has failed!");
  }
#pragma endregion

#pragma region Outline Vertex & Pixel Shader Set Up PSO
  m_outlinePipelineState.SetRootSignature(m_rootSignature);
  
  m_outlinePipelineState.SetVertexShader(
    outVtxShader->GetBufferPointer(), outVtxShader->GetBufferSize()
  );

  m_outlinePipelineState.SetPixelShader(
    outPxlShader->GetBufferPointer(), outPxlShader->GetBufferSize()
  );
  
  m_outlinePipelineState.SetInputLayout(
    _countof(inputElementDescriptor), inputElementDescriptor
  );
  
  m_outlinePipelineState.SetCullMode(
    D3D12_CULL_MODE_FRONT
  );
  
  m_outlinePipelineState.SetFrontCounterClockwise(true);
  
  m_outlinePipelineState.SetDepthTest(false);
  
  m_outlinePipelineState.SetStencilTest(
    true,
    D3D12_COMPARISON_FUNC_NOT_EQUAL,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP, 
    0xFF,
    0x00
  );
  
  m_outlinePipelineState.SetRenderTargetFormat(
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_D24_UNORM_S8_UINT
  );
  
  m_outlinePipelineState.Finalize(
    m_device.Get()
  );
#pragma endregion

#pragma region Stencil Write PSO Set Up
  m_stencilWritePipelineState.SetRootSignature(m_rootSignature);
  
  m_stencilWritePipelineState.SetVertexShader(
    vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()
  );
  
  m_stencilWritePipelineState.SetInputLayout(
    _countof(inputElementDescriptor), inputElementDescriptor
  );
  
  m_stencilWritePipelineState.SetCullMode(D3D12_CULL_MODE_BACK);
  m_stencilWritePipelineState.SetFrontCounterClockwise(true);
  
  m_stencilWritePipelineState.SetDepthTest(false);
  m_stencilWritePipelineState.SetColorWriteEnable(false);
  
  m_stencilWritePipelineState.SetStencilTest(
    true,
    D3D12_COMPARISON_FUNC_ALWAYS,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_REPLACE,
    0xFF,
    0xFF
  );
  
  m_stencilWritePipelineState.SetRenderTargetFormat(
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_D24_UNORM_S8_UINT
  );
  
  m_stencilWritePipelineState.Finalize(m_device.Get());
#pragma endregion
#pragma endregion
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
    *m_DsvAllocator,
    DXGI_FORMAT_D24_UNORM_S8_UINT
  );
}

void D3D12Engine::DirectX12Graphics::BeginFrame() {
  // ↓ Prepare Pipeline ↓
  UINT64 completedFenceValue = m_cmdQueue->GetCompletedFenceValue();
  m_pFrameContext = m_cmdContextPool->Allocate(completedFenceValue);
  // ↑ Prepare Pipeline ↑

  // ↓ Set Heaps ↓
  ID3D12DescriptorHeap* heaps[] = { m_SrvAllocator->GetHeap() };
  m_pFrameContext->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
  // ↑ Set Heaps ↑
  
  // ↓ Barrier ↓
  m_pFrameContext->TransitionResource(m_SceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
  m_pFrameContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  // ↓ Clean Up ↓
  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_pFrameContext->ClearColor(m_SceneTexture, ClearColor);
  m_pFrameContext->ClearDepth(m_DepthBuffer);

  m_pFrameContext->SetRenderTargets(m_SceneTexture, m_DepthBuffer);
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

  m_pFrameContext->SetViewports(1, &viewPort);
  m_pFrameContext->SetScissorRects(1, &scissorRect);
  // ↑ Set up Viewport & ScissorRect ↑

  m_pFrameContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_pFrameContext->SetGraphicsRootSignature(m_rootSignature.Get());
  m_pFrameContext->SetPipelineState(m_pipelineState.GetPipelineState());
}

void D3D12Engine::DirectX12Graphics::DrawNormal(
  D3D12Engine::Model& rModel,
  D3D12Engine::Texture& rTexture,
  const DirectX::XMMATRIX& rViewProjectionMatrix,
  bool isSelected
) {
  rTexture.Bind(*m_pFrameContext, 1);

  m_pFrameContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &rViewProjectionMatrix, 0);

  if (isSelected) {
    m_pFrameContext->GetCommandList()->OMSetStencilRef(1);
  } else {
    m_pFrameContext->GetCommandList()->OMSetStencilRef(0);
  }

  rModel.DrawModel(*m_pFrameContext);
}

void D3D12Engine::DirectX12Graphics::DrawOutline(
  D3D12Engine::Model& rModel,
  const DirectX::XMMATRIX& rViewProjectionMatrix
) {
  m_pFrameContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &rViewProjectionMatrix, 0);

  m_pFrameContext->GetCommandList()->OMSetStencilRef(1);
  m_pFrameContext->SetPipelineState(m_outlinePipelineState.GetPipelineState());

  rModel.DrawModel(*m_pFrameContext);
  
  // Restore normal pipeline state
  m_pFrameContext->SetPipelineState(m_pipelineState.GetPipelineState());
}

void D3D12Engine::DirectX12Graphics::DrawStencil(
  D3D12Engine::Model& rModel,
  const DirectX::XMMATRIX& rViewProjectionMatrix
) {
  m_pFrameContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &rViewProjectionMatrix, 0);

  m_pFrameContext->GetCommandList()->OMSetStencilRef(1);
  m_pFrameContext->SetPipelineState(m_stencilWritePipelineState.GetPipelineState());

  rModel.DrawModel(*m_pFrameContext);
  
  // Restore normal pipeline state
  m_pFrameContext->SetPipelineState(m_pipelineState.GetPipelineState());
}

void D3D12Engine::DirectX12Graphics::EndFrame() {
  auto& CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
  
  // ↓ Barrier ↓
  m_pFrameContext->TransitionResource(CurrentBackBuffer, D3D12_RESOURCE_STATE_PRESENT);
  m_pFrameContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  m_pFrameContext->Close();

  // ↓ Draw ↓
  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_pFrameContext->GetCommandList());

    // ↓ Free Context ↓
    m_pFrameContext->SetFenceValue(fenceValue);
    m_cmdContextPool->Free(m_pFrameContext);
    m_pFrameContext = nullptr;
    // ↑ Free Context ↑
  
  m_SwapChain->Present(*m_cmdQueue);
  // ↑ Draw ↑
}

void D3D12Engine::DirectX12Graphics::PrepareUIContext() {
  m_pFrameContext->TransitionResource(m_SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  auto& CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
  m_pFrameContext->TransitionResource(CurrentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
  
  m_pFrameContext->FlushResourceBarriers();

  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_pFrameContext->ClearColor(CurrentBackBuffer, ClearColor);
  m_pFrameContext->SetRenderTargets(CurrentBackBuffer);
}