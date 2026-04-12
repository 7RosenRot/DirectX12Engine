#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <filesystem>
#include <exception>
#include <stdexcept>

#include <Include/Graphics/Core/DirectX12Graphics.hpp>
#include <Include/Graphics/Core/CommandContext.hpp>
#include <Include/Graphics/Pipeline/Display.hpp>
#include <Include/Graphics/Scene/Model.hpp>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName) :
  InterfaceDirectX12(WindowHeight, WindowWidth, WindowName),
  m_viewPort(0.F, 0.F, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight)),
  m_scissorRect(0, 0, m_WindowWidth, m_WindowHeight)
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {
  LoadPipeline();

  // ↓ Initializing CommandQueue ↓
  m_cmdQueue = std::make_unique<CommandQueue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
  // ↓ Initializing CommandQueue ↓

  // ↓ Initializing CommandContext ↓
  m_cmdContext = std::make_unique<CommandContext>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
  // ↓ Initializing CommandContext ↓

  // ↓ Initializing SwapCahin ↓
  m_display = std::make_unique<Display>();

  m_display->Initialize(
    m_device.Get(),
    factory4.Get(),
    m_cmdQueue->GetResource(),
    D3D12Engine::Window::GetHwnd(),
    m_WindowWidth, m_WindowHeight
  );
  // ↑ Initializing SwapCahin ↑

  // ↓ Initializing Model ↓
  m_Model = std::make_unique<Model>();
  // ↑ Initializing Model ↑

  LoadAssets();
}

void D3D12Engine::DirectX12Graphics::OnRender() {
  m_cmdQueue->Flush();
  m_cmdContext->Reset();
  
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
  m_cmdContext->GetCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_DisplacementMatrix, 0);

  m_Model->Draw(*m_cmdContext);

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

void D3D12Engine::DirectX12Graphics::OnUpdate() {
  // ↓ Rotation ↓
  static float angle = 0.0f;
  angle += 0.01f;

  DirectX::XMMATRIX model = DirectX::XMMatrixRotationY(angle);
  
  DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);  // Мы стоим в -2 по Z
  DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // Смотрим в центр
  DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);    // Верх — это Y
  DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, focus, up);

  float aspectRatio = static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
  DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
    DirectX::XMConvertToRadians(45.0f),
    aspectRatio,
    0.1f,
    100.0f
  );

  m_DisplacementMatrix = DirectX::XMMatrixTranspose(model * view * projection);
  // ↑ Rotation ↑
}

void D3D12Engine::DirectX12Graphics::OnDestroy() {
  if (m_cmdQueue != nullptr) {
    m_cmdQueue->Flush();
  }

  m_Model.reset();
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
    if (vtxErrorBuffer) {
      OutputDebugStringA(reinterpret_cast<const char*>(vtxErrorBuffer->GetBufferPointer()));
    }

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
  m_Model->LoadObj("DX3D12Engine\\Assets\\Model.obj", m_device.Get(), *m_cmdContext);
  m_cmdContext->Close();

  UINT64 fenceValue = m_cmdQueue->ExecuteCommandList(m_cmdContext->GetCommandList());
 
  m_cmdQueue->WaitForPreviousFrame(fenceValue);
}