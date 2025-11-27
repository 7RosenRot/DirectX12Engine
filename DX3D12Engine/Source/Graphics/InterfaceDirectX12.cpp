#include <Include/Graphics/InterfaceDirectX12.hpp>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
    m_WindowHeight(WindowHeight), 
    m_WindowWidth(WindowWidht),
    m_WindowName(WindowName)
{
    coefficient = static_cast<float>(WindowWidht) / static_cast<float>(WindowHeight);
}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}