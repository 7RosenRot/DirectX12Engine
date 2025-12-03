#include <Include/Graphics/InterfaceDirectX12.hpp>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
    m_WindowHeight(WindowHeight), 
    m_WindowWidth(WindowWidht),
    m_WindowName(WindowName)
{}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}