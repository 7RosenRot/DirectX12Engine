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

  // ↓ Initializing Camera ↓
  m_Camera = std::make_unique<Camera>();
  
  m_Camera->GetTransform().SetPosition(0.0F, 15.0F, -25.0F);
  // ↑ Initializing Camera ↑

  // ↓ Loading Assets ↓
  LoadAssets();
  // ↑ Loading Assets ↑
}

void D3D12Engine::DirectX12Graphics::OnRender() {
  // ↓ Prepare Pipeline ↓
  m_cmdQueue->Flush();
  m_cmdContext->Reset();
  // ↑ Prepare Pipeline ↑
  
  // ↓ PipelineState & RootSignature ↓
  m_cmdContext->SetPipelineState(m_pipelineState.GetPipelineState());
  m_cmdContext->SetGraphicsRootSignature(m_rootSignature.Get());
  // ↑ PipelineState & RootSignature ↑

  // ↓ Viewport & ScissorRect ↓
  m_cmdContext->SetViewports(1, &m_viewPort);
  m_cmdContext->SetScissorRects(1, &m_scissorRect);
  // ↑ Viewport & ScissorRect ↑

  // ↓ Clean Up ↓
  const float ClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  auto& currentBackBuffer = m_display->GetCurrentBackBufferIndex();

  m_cmdContext->ClearColor(currentBackBuffer, ClearColor);
  m_cmdContext->ClearDepth(m_depthBuffer);
  m_cmdContext->SetRenderTargets(currentBackBuffer, m_depthBuffer);
  // ↑ Clean Up ↑

  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(m_display->GetCurrentBackBufferIndex(), D3D12_RESOURCE_STATE_RENDER_TARGET);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  m_cmdContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  
  m_GameObjects["MshkFrede"]->Draw(*m_cmdContext);
  m_GameObjects["Plane"]->Draw(*m_cmdContext);

  // ↓ Barrier ↓
  m_cmdContext->TransitionResource(m_display->GetCurrentBackBufferIndex(), D3D12_RESOURCE_STATE_PRESENT);
  m_cmdContext->FlushResourceBarriers();
  // ↑ Barrier ↑

  m_cmdContext->Close();

  // ↓ Draw ↓
  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
  
  m_display->Present();
  // ↑ Draw ↑
  
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
}

void D3D12Engine::DirectX12Graphics::OnResize(UINT WindowWidth, UINT WindowHeight) {
  m_cmdQueue->Flush();

  m_display->Resize(m_device.Get(), m_depthBuffer, WindowWidth, WindowHeight);

  const float AspectRatio = static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);

  float viewportWidth = static_cast<float>(WindowWidth);
  float viewportHeight = viewportWidth / AspectRatio;
  
  if (viewportHeight > WindowHeight) {
    viewportHeight = static_cast<float>(WindowHeight);
    viewportWidth = viewportHeight * AspectRatio;
  }
  
  float offsetX = (WindowWidth - viewportWidth) * 0.5F;
  float offsetY = (WindowHeight - viewportHeight) * 0.5F;
  
  m_viewPort = CD3DX12_VIEWPORT(
    offsetX, offsetY,
    viewportWidth, viewportHeight
  );
  m_scissorRect = CD3DX12_RECT(
    static_cast<long>(offsetX), static_cast<long>(offsetY),
    static_cast<long>(offsetX + viewportWidth), static_cast<long>(offsetY + viewportHeight)
  );

  m_Camera->SetLensProperties(DirectX::XMConvertToRadians(45.0f), AspectRatio, 0.1f, 20000.0f);
}

void D3D12Engine::DirectX12Graphics::OnUpdate() {
  // ↓ Camera Movement ↓
  const float MovementSpeed = 0.25F, MouseSensivity = 0.05F;
  m_Camera->InputProcessing(MovementSpeed, MouseSensivity);

  DirectX::XMMATRIX view = m_Camera->GetTransform().GetMatrixView();
  DirectX::XMMATRIX projection = m_Camera->GetMatrixProjection();
  // ↑ Camera Movement ↑
  
  // ↓ Rotation ↓
  static float angle = 0.0f;
  angle += 0.00f; // ← Set up rotation speed
  
  m_GameObjects["MshkFrede"]->UpdateModelMatrix(view, projection);
  m_GameObjects["Plane"]->UpdateModelMatrix(view, projection);
  // ↑ Rotation ↑
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  if (m_cmdQueue != nullptr) {
    m_cmdQueue->Flush();
  }

  m_GameObjects["MshkFrede"].reset();
  m_GameObjects["Plane"].reset();
  
  m_Camera.reset();
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
  m_rootSignature.Reset(1);
  m_rootSignature[0].InitAsConstants(0, 16, D3D12_SHADER_VISIBILITY_VERTEX);
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

  m_cmdContext->Reset();
    // ↓ Initializing & Downloading Models ↓
    m_GameObjects["MshkFrede"] = std::make_unique<GameObject>("MshkFrede", "Engine/Assets/Models/MshkFrede.obj");
    m_GameObjects["MshkFrede"]->Initialize(m_device.Get(), *m_cmdContext);
    m_GameObjects["MshkFrede"]->GetTransform().SetPosition(0.0F, 0.0F, 0.0F);

    m_GameObjects["Plane"] = std::make_unique<GameObject>("Plane", "Engine/Assets/Models/Plane.obj");
    m_GameObjects["Plane"]->Initialize(m_device.Get(), *m_cmdContext);
    m_GameObjects["Plane"]->GetTransform().SetPosition(0.0F, 0.0F, 0.0F);
    // ↑ Initializing & Downloading Models ↑
  m_cmdContext->Close();

  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
 
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
}