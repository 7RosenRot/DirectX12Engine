#include <Include/Graphics/DirectX12Graphics.hpp>

D3D12Engine::DirectX12Graphics::DirectX12Graphics(UINT m_WindowHeight, UINT m_WindowWidth, std::wstring m_WindowName) :
  InterfaceDirectX12(m_WindowHeight, m_WindowWidth, m_WindowName)
  // Добавить соответсвующие перменные, которые будут проинициализированы конструктором класса по умолчанию
{}

D3D12Engine::DirectX12Graphics::~DirectX12Graphics() {}

void D3D12Engine::DirectX12Graphics::OnInitialize() {}

void D3D12Engine::DirectX12Graphics::OnRender() {}

void D3D12Engine::DirectX12Graphics::OnUpdate() {}

void D3D12Engine::DirectX12Graphics::OnDestroy() {}

void D3D12Engine::DirectX12Graphics::LoadPipeline() {}

void D3D12Engine::DirectX12Graphics::LoadAssets() {}

void D3D12Engine::DirectX12Graphics::FillCommandList() {}

void D3D12Engine::DirectX12Graphics::WaitForPreviousFrame() {}